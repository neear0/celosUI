#include "../glfw/ui_glfw.h"

// vulkan side of platform::create_texture(): the pixels go through a staging buffer into a device local image,
// uploaded with a one off command buffer and waited on, images are only made at startup. the ImTextureID is the
// descriptor set ImGui's vulkan backend binds for the image

namespace celosia_glfw::textures {
    struct t_texture {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDescriptorSet set = VK_NULL_HANDLE;
    };
    static std::vector<t_texture> list;
    static VkSampler sampler = VK_NULL_HANDLE;
    static VkCommandPool command_pool = VK_NULL_HANDLE;

    static void release(t_texture& t) {
        if (t.set) ImGui_ImplVulkan_RemoveTexture(t.set);
        if (t.view) vkDestroyImageView(vulkan::device, t.view, nullptr);
        if (t.image) vkDestroyImage(vulkan::device, t.image, nullptr);
        if (t.memory) vkFreeMemory(vulkan::device, t.memory, nullptr);
        t = {};
    }

    static bool create_shared() { // the sampler and command pool every texture uses, made with the first one
        if (sampler)
            return true;

        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.maxAnisotropy = 1.0f;
        info.maxLod = VK_LOD_CLAMP_NONE;
        if (vkCreateSampler(vulkan::device, &info, nullptr, &sampler) != VK_SUCCESS)
            return false;

        VkCommandPoolCreateInfo pool = {};
        pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool.queueFamilyIndex = vulkan::queue_family;
        return vkCreateCommandPool(vulkan::device, &pool, nullptr, &command_pool) == VK_SUCCESS;
    }

    static bool allocate(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, VkDeviceMemory* memory) {
        VkMemoryAllocateInfo allocation = {};
        allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = vulkan::memory_type(requirements.memoryTypeBits, properties);
        return allocation.memoryTypeIndex != (uint32_t)-1 && vkAllocateMemory(vulkan::device, &allocation, nullptr, memory) == VK_SUCCESS;
    }

    static bool upload(const unsigned char* rgba, int width, int height, VkImage image) { // copies the pixels into the image and leaves it ready for sampling
        struct t_staging { // freed however this returns
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkCommandBuffer command_buffer = VK_NULL_HANDLE;
            ~t_staging() {
                if (command_buffer) vkFreeCommandBuffers(vulkan::device, command_pool, 1, &command_buffer);
                if (buffer) vkDestroyBuffer(vulkan::device, buffer, nullptr);
                if (memory) vkFreeMemory(vulkan::device, memory, nullptr);
            }
        } staging;
        const VkDeviceSize size = (VkDeviceSize)width * height * 4;

        VkBufferCreateInfo buffer = {};
        buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer.size = size;
        buffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(vulkan::device, &buffer, nullptr, &staging.buffer) != VK_SUCCESS)
            return false;
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(vulkan::device, staging.buffer, &requirements);
        void* mapped = nullptr;
        if (!allocate(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging.memory) ||
            vkBindBufferMemory(vulkan::device, staging.buffer, staging.memory, 0) != VK_SUCCESS ||
            vkMapMemory(vulkan::device, staging.memory, 0, size, 0, &mapped) != VK_SUCCESS)
            return false;
        memcpy(mapped, rgba, (size_t)size);
        vkUnmapMemory(vulkan::device, staging.memory);

        VkCommandBufferAllocateInfo allocation = {};
        allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocation.commandPool = command_pool;
        allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocation.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(vulkan::device, &allocation, &staging.command_buffer) != VK_SUCCESS)
            return false;
        const VkCommandBuffer command_buffer = staging.command_buffer;

        VkCommandBufferBeginInfo begin = {};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };
        vkCmdCopyBufferToImage(command_buffer, staging.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &command_buffer;
        return vkEndCommandBuffer(command_buffer) == VK_SUCCESS &&
            vkQueueSubmit(vulkan::queue, 1, &submit, VK_NULL_HANDLE) == VK_SUCCESS &&
            vkQueueWaitIdle(vulkan::queue) == VK_SUCCESS;
    }

    void destroy() {
        for (t_texture& t : list)
            release(t);
        list.clear();
        if (sampler) { vkDestroySampler(vulkan::device, sampler, nullptr); sampler = VK_NULL_HANDLE; }
        if (command_pool) { vkDestroyCommandPool(vulkan::device, command_pool, nullptr); command_pool = VK_NULL_HANDLE; }
    }
}

namespace celosia::platform {
    ImTextureID create_texture(const unsigned char* rgba, int width, int height) {
        using namespace celosia_glfw;
        if (!textures::create_shared())
            return nullptr;

        textures::t_texture t;
        VkImageCreateInfo image = {};
        image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image.imageType = VK_IMAGE_TYPE_2D;
        image.format = VK_FORMAT_R8G8B8A8_UNORM;
        image.extent = { (uint32_t)width, (uint32_t)height, 1 };
        image.mipLevels = 1;
        image.arrayLayers = 1;
        image.samples = VK_SAMPLE_COUNT_1_BIT;
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkMemoryRequirements requirements;
        if (vkCreateImage(vulkan::device, &image, nullptr, &t.image) != VK_SUCCESS)
            return nullptr;
        vkGetImageMemoryRequirements(vulkan::device, t.image, &requirements);
        if (!textures::allocate(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &t.memory) && !textures::allocate(requirements, 0, &t.memory)) {
            textures::release(t);
            return nullptr;
        }

        VkImageViewCreateInfo view = {};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.image = t.image;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = image.format;
        view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (vkBindImageMemory(vulkan::device, t.image, t.memory, 0) != VK_SUCCESS ||
            !textures::upload(rgba, width, height, t.image) ||
            vkCreateImageView(vulkan::device, &view, nullptr, &t.view) != VK_SUCCESS) {
            textures::release(t);
            return nullptr;
        }

        t.set = ImGui_ImplVulkan_AddTexture(textures::sampler, t.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        textures::list.push_back(t);
        return (ImTextureID)t.set;
    }

    void destroy_texture(ImTextureID texture) {
        using namespace celosia_glfw;
        for (size_t i = 0; i < textures::list.size(); i++)
            if ((ImTextureID)textures::list[i].set == texture) {
                vkDeviceWaitIdle(vulkan::device); // frames in flight may still be drawing it
                textures::release(textures::list[i]);
                textures::list.erase(textures::list.begin() + i);
                return;
            }
    }
}

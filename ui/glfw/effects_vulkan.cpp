#include "../glfw/ui_glfw.h"

#include <cstdint>
#include "shaders/gradient_spirv.h"
#include "shaders/glow_spirv.h"
#include "shaders/blur_spirv.h"
#include "shaders/background_spirv.h"

// vulkan side of celosia::effects: every effect is a pipeline made from ImGui's own (same vertex shader, layout and blending)
// with the effect's fragment shader, which the draw list callback binds. the constants go into the push constant range
// the backend shares with fragment shaders, after the 16 bytes ImGui's vertex shader uses.
// the blur needs what's been drawn so far: copies can't happen inside a render pass, so the pass is ended, the frame
// is copied into the backdrop image, and drawing resumes in a render pass that keeps what's there.

namespace celosia_glfw::effect_renderer {
    static VkPipeline pipelines[(int)celosia::effects::e_type::count] = {};
    static VkRenderPass resume_pass = VK_NULL_HANDLE; // the window's render pass, but loading the frame instead of clearing it
    static VkSampler sampler = VK_NULL_HANDLE;        // the blur reads near the frame's edges, ImGui's sampler repeats
    static VkImage backdrop = VK_NULL_HANDLE;         // copy of the frame the blur reads from
    static VkDeviceMemory backdrop_memory = VK_NULL_HANDLE;
    static VkImageView backdrop_view = VK_NULL_HANDLE;
    static VkDescriptorSet backdrop_set = VK_NULL_HANDLE; // the ImTextureID the blur is drawn with, null when the frame can't be copied

    static uint32_t memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(vulkan::physical_device, &memory);
        for (uint32_t i = 0; i < memory.memoryTypeCount; i++)
            if ((type_bits & (1u << i)) && (memory.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        return (uint32_t)-1;
    }

    static VkImageMemoryBarrier barrier(VkImage image, VkImageLayout from, VkImageLayout to, VkAccessFlags src_access, VkAccessFlags dst_access) {
        VkImageMemoryBarrier b = {};
        b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        b.srcAccessMask = src_access;
        b.dstAccessMask = dst_access;
        b.oldLayout = from;
        b.newLayout = to;
        b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.image = image;
        b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        return b;
    }

    bool create() {
        struct { const uint32_t* code; size_t size; } code[] = { // same order as effects::e_type
            { gradient_spirv, sizeof(gradient_spirv) },
            { glow_spirv, sizeof(glow_spirv) },
            { blur_spirv, sizeof(blur_spirv) },
            { background_spirv, sizeof(background_spirv) },
        };
        static_assert(std::size(code) == (size_t)celosia::effects::e_type::count);
        for (int i = 0; i < (int)celosia::effects::e_type::count; i++) {
            VkShaderModuleCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            info.codeSize = code[i].size;
            info.pCode = code[i].code;
            VkShaderModule module;
            if (vkCreateShaderModule(vulkan::device, &info, nullptr, &module) != VK_SUCCESS)
                return false;
            pipelines[i] = ImGui_ImplVulkan_CreatePipelineWithFragmentShader(module);
            vkDestroyShaderModule(vulkan::device, module, nullptr); // the pipeline doesn't need it anymore
            if (pipelines[i] == VK_NULL_HANDLE)
                return false;
        }

        resume_pass = ImGui_ImplVulkanH_CreateRenderPass(vulkan::device, vulkan::window_data.SurfaceFormat.format, false, true, nullptr);

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

        return create_backdrop();
    }

    bool create_backdrop() {
        const ImGui_ImplVulkanH_Window* wd = &vulkan::window_data;
        VkSurfaceCapabilitiesKHR capabilities;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan::physical_device, wd->Surface, &capabilities) != VK_SUCCESS)
            return false;
        if (!(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) // the swapchain can't be copied, blur falls back to a plain panel
            return true;

        VkImageCreateInfo image = {};
        image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image.imageType = VK_IMAGE_TYPE_2D;
        image.format = wd->SurfaceFormat.format;
        image.extent = { (uint32_t)wd->Width, (uint32_t)wd->Height, 1 };
        image.mipLevels = 1;
        image.arrayLayers = 1;
        image.samples = VK_SAMPLE_COUNT_1_BIT;
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        image.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(vulkan::device, &image, nullptr, &backdrop) != VK_SUCCESS)
            return false;

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(vulkan::device, backdrop, &requirements);
        VkMemoryAllocateInfo allocation = {};
        allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (allocation.memoryTypeIndex == (uint32_t)-1)
            allocation.memoryTypeIndex = memory_type(requirements.memoryTypeBits, 0);
        if (vkAllocateMemory(vulkan::device, &allocation, nullptr, &backdrop_memory) != VK_SUCCESS ||
            vkBindImageMemory(vulkan::device, backdrop, backdrop_memory, 0) != VK_SUCCESS)
            return false;

        VkImageViewCreateInfo view = {};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.image = backdrop;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = image.format;
        view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (vkCreateImageView(vulkan::device, &view, nullptr, &backdrop_view) != VK_SUCCESS)
            return false;

        backdrop_set = ImGui_ImplVulkan_AddTexture(sampler, backdrop_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        return true;
    }

    void destroy_backdrop() { // the device has to be idle
        if (backdrop_set) { ImGui_ImplVulkan_RemoveTexture(backdrop_set); backdrop_set = VK_NULL_HANDLE; }
        if (backdrop_view) { vkDestroyImageView(vulkan::device, backdrop_view, nullptr); backdrop_view = VK_NULL_HANDLE; }
        if (backdrop) { vkDestroyImage(vulkan::device, backdrop, nullptr); backdrop = VK_NULL_HANDLE; }
        if (backdrop_memory) { vkFreeMemory(vulkan::device, backdrop_memory, nullptr); backdrop_memory = VK_NULL_HANDLE; }
    }

    void destroy() { // the device has to be idle
        destroy_backdrop();
        for (VkPipeline& pipeline : pipelines)
            if (pipeline) { vkDestroyPipeline(vulkan::device, pipeline, nullptr); pipeline = VK_NULL_HANDLE; }
        if (sampler) { vkDestroySampler(vulkan::device, sampler, nullptr); sampler = VK_NULL_HANDLE; }
        if (resume_pass) { vkDestroyRenderPass(vulkan::device, resume_pass, nullptr); resume_pass = VK_NULL_HANDLE; }
    }

    static void copy_frame() {
        const ImGui_ImplVulkanH_Window* wd = &vulkan::window_data;
        const VkImage frame = wd->Frames[wd->FrameIndex].Backbuffer;
        const VkExtent2D extent = { (uint32_t)wd->Width, (uint32_t)wd->Height };

        vkCmdEndRenderPass(command_buffer); // leaves the frame in PRESENT_SRC_KHR, the pass's end dependency finishes the drawing before transfers

        VkImageMemoryBarrier before[2] = {
            barrier(frame, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT),
            barrier(backdrop, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT), // earlier blurs may still be reading it, its old contents don't matter
        };
        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 2, before);

        VkImageCopy region = {};
        region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.dstSubresource = region.srcSubresource;
        region.extent = { extent.width, extent.height, 1 };
        vkCmdCopyImage(command_buffer, frame, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, backdrop, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier after[2] = {
            barrier(frame, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT),
            barrier(backdrop, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT),
        };
        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 2, after);

        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = resume_pass;
        info.framebuffer = wd->Frames[wd->FrameIndex].Framebuffer;
        info.renderArea.extent = extent;
        vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE); // bound pipeline, buffers and dynamic state carry over
    }

    static void bind(const celosia::effects::t_command& effect) {
        if (effect.type == celosia::effects::e_type::blur)
            copy_frame();

        constexpr uint32_t offset = offsetof(celosia::effects::t_constants, rect); // the bytes before belong to ImGui's vertex shader
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[(int)effect.type]);
        vkCmdPushConstants(command_buffer, ImGui_ImplVulkan_GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            offset, sizeof(celosia::effects::t_constants) - offset, &effect.constants.rect);
    }
}

namespace celosia::platform {
    void draw_effect(const ImDrawList*, const ImDrawCmd* command) {
        celosia_glfw::effect_renderer::bind(effects::commands[(size_t)(intptr_t)command->UserCallbackData]);
    }

    ImTextureID backdrop_texture() {
        return (ImTextureID)celosia_glfw::effect_renderer::backdrop_set;
    }
}

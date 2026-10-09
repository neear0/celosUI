#include "../glfw/ui_glfw.h"

// celosia::platform on top of a glfw window and the vulkan renderer in this folder

namespace celosia::platform {
    static GLFWwindow* window = nullptr;

    static void error_callback(int error, const char* description) {
        std::cerr << "[glfw] error " << error << ": " << description << std::endl;
    }

    bool create(const char* title, ImVec2 pos, ImVec2 size) {
        if (volkInitialize() != VK_SUCCESS) {
            std::cerr << "[vulkan] no vulkan loader found, install your gpu's vulkan driver" << std::endl;
            return false;
        }

        glfwSetErrorCallback(error_callback);
        glfwInitVulkanLoader(vkGetInstanceProcAddr);
        if (glfwPlatformSupported(GLFW_PLATFORM_X11)) // wayland doesn't let windows move themselves or stay on top, xwayland does
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);     // the ui draws its own titlebar
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);       // topmost
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);       // shown once it's in place
        window = glfwCreateWindow((int)size.x, (int)size.y, title, nullptr, nullptr);
        if (!window)
            return false;
        glfwSetWindowPos(window, (int)pos.x, (int)pos.y);

        if (!celosia_glfw::vulkan::create_device(window))
            return false;

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = celosia_glfw::vulkan::instance;
        init_info.PhysicalDevice = celosia_glfw::vulkan::physical_device;
        init_info.Device = celosia_glfw::vulkan::device;
        init_info.QueueFamily = celosia_glfw::vulkan::queue_family;
        init_info.Queue = celosia_glfw::vulkan::queue;
        init_info.DescriptorPool = celosia_glfw::vulkan::descriptor_pool;
        init_info.MinImageCount = celosia_glfw::vulkan::min_image_count;
        init_info.ImageCount = celosia_glfw::vulkan::window_data.ImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn = celosia_glfw::vulkan::check_result;
        ImGui_ImplVulkan_Init(&init_info, celosia_glfw::vulkan::window_data.RenderPass);

        glfwShowWindow(window);
        return true;
    }

    bool poll() {
        glfwPollEvents();
        if (celosia_glfw::vulkan::swapchain_rebuild)
            celosia_glfw::vulkan::resize_swapchain(window);
        return !glfwWindowShouldClose(window);
    }

    void new_frame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
    }

    void present() {
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) // minimized
            return;
        celosia_glfw::vulkan::render(draw_data);
        celosia_glfw::vulkan::present();
    }

    void destroy() {
        vkDeviceWaitIdle(celosia_glfw::vulkan::device);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        celosia_glfw::vulkan::destroy();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    static int glfw_key(int key) { // celosia::keys (windows virtual key codes) to glfw, -1 when there's no match
        if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z'))
            return key; // glfw uses ascii for these as well
        if (key >= keys::f1 && key < keys::f1 + 12)
            return GLFW_KEY_F1 + (key - keys::f1);

        switch (key) {
        case keys::backspace: return GLFW_KEY_BACKSPACE;
        case keys::tab: return GLFW_KEY_TAB;
        case keys::enter: return GLFW_KEY_ENTER;
        case keys::escape: return GLFW_KEY_ESCAPE;
        case keys::space: return GLFW_KEY_SPACE;
        case keys::left: return GLFW_KEY_LEFT;
        case keys::up: return GLFW_KEY_UP;
        case keys::right: return GLFW_KEY_RIGHT;
        case keys::down: return GLFW_KEY_DOWN;
        case keys::insert: return GLFW_KEY_INSERT;
        case keys::del: return GLFW_KEY_DELETE;
        case keys::home: return GLFW_KEY_HOME;
        case keys::end: return GLFW_KEY_END;
        }
        return -1;
    }

    bool key_held(int key) { // glfw only sees input aimed at its own window
        switch (key) {
        case keys::mouse_left: return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        case keys::mouse_right: return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        case keys::mouse_middle: return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        case keys::shift: return glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        case keys::control: return glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
        case keys::alt: return glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
        }
        const int glfw = glfw_key(key);
        return glfw != -1 && glfwGetKey(window, glfw) == GLFW_PRESS;
    }

    ImVec2 cursor_pos() { // glfw reports the cursor relative to the window
        double x, y;
        int window_x, window_y;
        glfwGetCursorPos(window, &x, &y);
        glfwGetWindowPos(window, &window_x, &window_y);
        return ImVec2((float)(window_x + x), (float)(window_y + y));
    }

    ImRect window_rect() {
        int x, y, w, h;
        glfwGetWindowPos(window, &x, &y);
        glfwGetWindowSize(window, &w, &h);
        return ImRect((float)x, (float)y, (float)(x + w), (float)(y + h));
    }

    ImVec2 screen_size() {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return mode ? ImVec2((float)mode->width, (float)mode->height) : ImVec2(0, 0);
    }

    void move(ImVec2 pos) {
        glfwSetWindowPos(window, (int)pos.x, (int)pos.y);
    }

    void bring_to_top() {
        // GLFW_FLOATING already keeps the window above others, raising it every frame would steal focus
    }
}

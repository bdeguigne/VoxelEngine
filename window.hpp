#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vxe
{
    class Window
    {
    public:
        Window(int width, int height, std::string name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(_window); };
        VkExtent2D getExtent() { return {static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)}; }
        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        bool isKeyPressed(int key);

    private:
        void
        _initWindow();

        const int _width;
        const int _height;
        std::string _windowName;
        GLFWwindow *_window;
    };
} // namespace vxe
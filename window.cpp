#include "window.hpp"
#include <stdexcept>

namespace vxe {
    Window::Window(int width, int height, std::string name) : _width{width}, _height{height}, _windowName{name} {
        _initWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, _window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    bool Window::isKeyPressed(int key) {
        return (glfwGetKey(_window, key) == GLFW_PRESS);
    }

    void Window::_initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _window = glfwCreateWindow(_width, _height, _windowName.c_str(), nullptr, nullptr);
    }
}

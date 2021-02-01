#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

#include <memory>
#include <vector>

namespace vxe
{
    class Engine
    {
    public:
        static const int WIDTH = 800;
        static const int HEIGHT = 600;

        Engine();
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        void run();

    private:
        void _createPipelineLayout();
        void _createPipeline();
        void _createCommandBuffers();
        void _drawFrame();

        Window _window{WIDTH, HEIGHT, "Hello Vulkan!"};
        Device _device{_window};
        SwapChain _swapChain{_device, _window.getExtent()};
        std::unique_ptr<Pipeline> _pipeline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;
    };
} // namespace vxe
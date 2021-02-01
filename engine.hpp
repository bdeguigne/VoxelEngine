#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "mesh.hpp"

#include "lib/vk_mem_alloc.h"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace vxe
{
    struct MeshPushConstants
    {
        glm::vec4 data;
        glm::mat4 render_matrix;
    };

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
        void _createAllocator();
        void _createPipelineLayout();
        void _createPipeline();
        void _createCommandBuffers();
        void _drawFrame();
        void _loadMeshes();
        void _uploadMesh(Mesh &mesh);

        Window _window{WIDTH, HEIGHT, "Hello Vulkan!"};
        Device _device{_window};
        SwapChain _swapChain{_device, _window.getExtent()};
        std::unique_ptr<Pipeline> _pipeline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;

        VmaAllocator _allocator;
        Mesh _triangleMesh;

        int _frameNumber = 0;

        bool _switchPipeline = false;
    };
} // namespace vxe
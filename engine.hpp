#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "mesh.hpp"
#include "camera.hpp"

#include "lib/vk_mem_alloc.h"
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace vxe
{
    struct Material
    {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    struct RenderObject
    {
        Mesh *mesh;

        Material *material;

        glm::mat4 transformMatrix;
    };

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

        // Scene management
        Material* createMaterial(VkPipeline pipeline, VkPipelineLayout layout,const std::string& name);
        Material* getMaterial(const std::string& name);
        Mesh* getMesh(const std::string& name); 

        std::vector<RenderObject> renderables;
        std::unordered_map<std::string, Material> materials;
        std::unordered_map<std::string, Mesh> meshes;
        void drawObjects(VkCommandBuffer cmd,RenderObject* first, int count);

    private:
        void _createAllocator();
        void _createPipelineLayout();
        void _createPipeline();
        void _createCommandBuffers();
        void _drawFrame();
        void _loadMeshes();
        void _uploadMesh(Mesh &mesh);
        void initScene();

        void _updateTime();

        Window _window{WIDTH, HEIGHT, "Hello Vulkan!"};
        Device _device{_window};
        SwapChain _swapChain{_device, _window.getExtent()};
        std::unique_ptr<Pipeline> _pipeline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;

        Camera _camera{};
        double _mouseX = 0;
        double _mouseY = 0;

        VmaAllocator _allocator;
        Mesh _triangleMesh;
        Mesh _room;

        int _frameNumber = 0;

        // Delta Time
        std::chrono::steady_clock::time_point _lastTime = std::chrono::high_resolution_clock::now();
        float _dt = 0.f;
    };
} // namespace vxe
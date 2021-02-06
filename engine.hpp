#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "camera.hpp"
#include "model.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace vxe
{

    // struct Material
    // {
    //     VkPipeline pipeline;
    //     VkPipelineLayout pipelineLayout;
    // };

    // struct RenderObject
    // {
    //     Mesh *mesh;

    //     Material *material;

    //     glm::mat4 transformMatrix;
    // };

    // struct MeshPushConstants
    // {
    //     glm::vec4 data;
    //     glm::mat4 render_matrix;
    // };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
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
        // Material *createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name);
        // Material *getMaterial(const std::string &name);
        // Mesh *getMesh(const std::string &name);

        // std::vector<RenderObject> renderables;
        // std::unordered_map<std::string, Material> materials;
        // std::unordered_map<std::string, Mesh> meshes;
        // void drawObjects(VkCommandBuffer cmd, RenderObject *first, int count);

    private:
        //Ubo management
        void _createDescriptorSetLayout();
        void _createUniformBuffers();
        void _createDescriptorPool();
        void _createDescriptorSets();
        void _updateUniformBuffer(uint32_t currentImage);

        void _createPipelineLayout();
        void _createPipeline();
        void _createCommandBuffers();
        void _drawFrame();
        void _loadModels();
        void initScene();
        std::vector<Model::Vertex> _loadFromObj(const char *filename);

        void _updateTime();

        Window _window{WIDTH, HEIGHT, "Hello Vulkan!"};
        Device _device{_window};
        SwapChain _swapChain{_device, _window.getExtent()};
        std::unique_ptr<Pipeline> _pipeline;
        VkPipelineLayout _pipelineLayout;
        std::vector<VkCommandBuffer> _commandBuffers;

        //ubo
        VkDescriptorSetLayout _descriptorSetLayout;
        VkBuffer _indexBuffer;
        VkDeviceMemory _indexBufferMemory;
        std::vector<VkBuffer> _uniformBuffers;
        std::vector<VkDeviceMemory> _uniformBuffersMemory;
        VkDescriptorPool _descriptorPool;
        std::vector<VkDescriptorSet> _descriptorSets;

        Camera _camera{};
        double _mouseX = 0;
        double _mouseY = 0;

        std::unique_ptr<Model> _model;

        int _frameNumber = 0;

        // Delta Time
        std::chrono::steady_clock::time_point _lastTime = std::chrono::high_resolution_clock::now();
        float _dt = 0.f;
    };
} // namespace vxe
#include "engine.hpp"

#define VMA_IMPLEMENTATION
#include "lib/vk_mem_alloc.h"

#include <stdexcept>
#include <array>
#include <iostream>
#include <glm/gtx/transform.hpp>

namespace vxe
{
    Engine::Engine()
    {
        _createAllocator();
        _createPipelineLayout();
        _createPipeline();
        _loadMeshes();
    }

    Engine::~Engine()
    {
        vkDestroyPipelineLayout(_device.device(), _pipelineLayout, nullptr);
        vmaDestroyBuffer(_allocator, _triangleMesh.vertexBuffer._buffer, _triangleMesh.vertexBuffer._allocation);
        vmaDestroyAllocator(_allocator);
    }

    void Engine::run()
    {
        while (!_window.shouldClose())
        {
            glfwPollEvents();

            if (_window.isKeyPressed(GLFW_KEY_SPACE))
            {
                _switchPipeline = !_switchPipeline;
                std::cout << _switchPipeline << std::endl;
            }

            _drawFrame();
        }

        vkDeviceWaitIdle(_device.device());
    }

    void Engine::_createAllocator()
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = _device.getPhysicalDevice();
        allocatorInfo.device = _device.device();
        allocatorInfo.instance = _device.getInstance();
        vmaCreateAllocator(&allocatorInfo, &_allocator);
    }

    void Engine::_createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        //setup push constants
        VkPushConstantRange push_constant;
        //this push constant range starts at the beginning
        push_constant.offset = 0;
        //this push constant range takes up the size of a MeshPushConstants struct
        push_constant.size = sizeof(MeshPushConstants);
        //this push constant range is accessible only in the vertex shader
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if (vkCreatePipelineLayout(_device.device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void Engine::_createPipeline()
    {
        auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(_swapChain.width(), _swapChain.height());
        pipelineConfig.renderPass = _swapChain.getRenderPass();
        pipelineConfig.pipelineLayout = _pipelineLayout;

        _pipeline = std::make_unique<Pipeline>(
            _device,
            "shaders/tri_mesh.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void Engine::_createCommandBuffers()
    {

        _commandBuffers.resize(_swapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = _device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

        if (vkAllocateCommandBuffers(_device.device(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers");
        }

        for (int i = 0; i < _commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = _swapChain.getRenderPass();
            renderPassInfo.framebuffer = _swapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = _swapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            _pipeline->bind(_commandBuffers[i]);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, &_triangleMesh.vertexBuffer._buffer, &offset);

            //make a model view matrix for rendering the object
            //camera position
            glm::vec3 camPos = {0.f, 0.f, -1.f};

            glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
            //camera projection
            glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
            projection[1][1] *= -1;
            //model rotation
            glm::mat4 model = glm::rotate(glm::mat4{1.0f}, glm::radians(_frameNumber * 0.4f), glm::vec3(0, 1, 0));

            //calculate final mesh matrix
            glm::mat4 mesh_matrix = projection * view * model;

            MeshPushConstants constants;
            constants.render_matrix = mesh_matrix;

            //upload the matrix to the GPU via pushconstants
            vkCmdPushConstants(_commandBuffers[i], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

            vkCmdDraw(_commandBuffers[i], _triangleMesh.vertices.size(), 1, 0, 0);

            vkCmdEndRenderPass(_commandBuffers[i]);

            if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer");
            }
        }
    }

    void Engine::_drawFrame()
    {
        uint32_t imageIndex;
        auto result = _swapChain.acquireNextImage(&imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image");
        }

        _createCommandBuffers();

        result = _swapChain.submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image");
        }

        _frameNumber++;
    }

    void Engine::_loadMeshes()
    {
        _triangleMesh.vertices.resize(3);

        //vertex positions
        _triangleMesh.vertices[0].position = {0.5f, 0.5f, 0.0f};
        _triangleMesh.vertices[1].position = {-0.5f, 0.5f, 0.0f};
        _triangleMesh.vertices[2].position = {0.f, -0.5f, 0.0f};

        //vertex colors, all green
        _triangleMesh.vertices[0].color = {1.f, 0.f, 0.0f}; //pure green
        _triangleMesh.vertices[1].color = {0.f, 1.f, 0.0f}; //pure green
        _triangleMesh.vertices[2].color = {0.f, 1.f, 0.0f}; //pure green

        _uploadMesh(_triangleMesh);
    }

    void Engine::_uploadMesh(Mesh &mesh)
    {
        //allocate vertex buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        //this is the total size, in bytes, of the buffer we are allocating
        bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
        //this buffer is going to be used as a Vertex Buffer
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        //let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        //allocate the buffer
        if (vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
                            &mesh.vertexBuffer._buffer,
                            &mesh.vertexBuffer._allocation,
                            nullptr))
            ;

        void *data;
        vmaMapMemory(_allocator, mesh.vertexBuffer._allocation, &data);
        memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
        vmaUnmapMemory(_allocator, mesh.vertexBuffer._allocation);
    }
} // namespace vxe
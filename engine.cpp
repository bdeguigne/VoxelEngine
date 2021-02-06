#include "engine.hpp"

#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define TINYOBJLOADER_IMPLEMENTATION
#include "lib/tiny_obj_loader.h"

namespace vxe
{
    Engine::Engine()
    {
        // _createAllocator();
        _loadModels();
        _createDescriptorSetLayout();
        _createPipelineLayout();
        _createPipeline();
        _createUniformBuffers();
        _createDescriptorPool();
        _createDescriptorSets();
        // initScene();
        _createCommandBuffers();
    }

    Engine::~Engine()
    {
        for (size_t i = 0; i < _swapChain.imageCount(); i++)
        {
            vkDestroyBuffer(_device.device(), _uniformBuffers[i], nullptr);
            vkFreeMemory(_device.device(), _uniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(_device.device(), _descriptorPool, nullptr);

        vkDestroyPipelineLayout(_device.device(), _pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(_device.device(), _descriptorSetLayout, nullptr);
    }

    void Engine::run()
    {
        while (!_window.shouldClose())
        {
            _updateTime();
            glfwPollEvents();
            _window.getMousePos(&_mouseX, &_mouseY);
            _camera.setMousePos(_dt, _mouseX, _mouseY);

            if (_window.isKeyPressed(GLFW_KEY_ESCAPE))
            {
                _window.close();
            }
            if (_window.isKeyPressed(GLFW_KEY_W))
            {
                _camera.move(10.f, FORWARD, _dt);
            }
            if (_window.isKeyPressed(GLFW_KEY_A))
            {
                _camera.move(10.f, LEFT, _dt);
            }
            if (_window.isKeyPressed(GLFW_KEY_S))
            {
                _camera.move(10.f, BACKWARD, _dt);
            }
            if (_window.isKeyPressed(GLFW_KEY_D))
            {
                _camera.move(10.f, RIGHT, _dt);
            }
            _drawFrame();
        }

        vkDeviceWaitIdle(_device.device());
    }

    // Material *Engine::createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name)
    // {
    //     Material mat;
    //     mat.pipeline = pipeline;
    //     mat.pipelineLayout = layout;
    //     materials[name] = mat;
    //     return &materials[name];
    // }

    // Material *Engine::getMaterial(const std::string &name)
    // {
    //     //search for the object, and return nullpointer if not found
    //     auto it = materials.find(name);
    //     if (it == materials.end())
    //     {
    //         return nullptr;
    //     }
    //     else
    //     {
    //         return &(*it).second;
    //     }
    // }

    // Mesh *Engine::getMesh(const std::string &name)
    // {
    //     auto it = meshes.find(name);
    //     if (it == meshes.end())
    //     {
    //         return nullptr;
    //     }
    //     else
    //     {
    //         return &(*it).second;
    //     }
    // }

    // void Engine::drawObjects(VkCommandBuffer cmd, RenderObject *first, int count)
    // {
    //     //make a model view matrix for rendering the object
    //     //camera view
    //     glm::vec3 camPos = {0.f, -6.f, -10.f};

    //     glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
    //     // glm::mat4 view = _camera.getViewMatrix();

    //     //camera projection
    //     glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    //     projection[1][1] *= -1;

    //     Mesh *lastMesh = nullptr;
    //     Material *lastMaterial = nullptr;
    //     for (int i = 0; i < count; i++)
    //     {
    //         RenderObject &object = first[i];

    //         _pipeline->bind(cmd);

    //         //only bind the pipeline if it doesnt match with the already bound one
    //         // if (object.material != lastMaterial)
    //         // {
    //         //     // std::cout << "NEW MATERIAL" << std::endl;
    //         //     vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
    //         //     lastMaterial = object.material;
    //         // }

    //         glm::mat4 model = object.transformMatrix;
    //         //final render matrix, that we are calculating on the cpu
    //         glm::mat4 mesh_matrix = projection * view * model;

    //         MeshPushConstants constants;
    //         constants.render_matrix = mesh_matrix;

    //         //upload the mesh to the GPU via pushconstants
    //         vkCmdPushConstants(cmd, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

    //         //only bind the mesh if it's a different one from last bind
    //         //bind the mesh vertex buffer with offset 0
    //         VkDeviceSize offset = 0;
    //         vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer, &offset);
    //         lastMesh = object.mesh;
    //         //we can now draw
    //         vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
    //     }
    // }

    // void Engine::_createAllocator()
    // {
    //     VmaAllocatorCreateInfo allocatorInfo = {};
    //     allocatorInfo.physicalDevice = _device.getPhysicalDevice();
    //     allocatorInfo.device = _device.device();
    //     allocatorInfo.instance = _device.getInstance();
    //     vmaCreateAllocator(&allocatorInfo, &_allocator);
    // }

    void Engine::_createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(_device.device(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void Engine::_createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        _uniformBuffers.resize(_swapChain.imageCount());
        _uniformBuffersMemory.resize(_swapChain.imageCount());

        for (size_t i = 0; i < _swapChain.imageCount(); i++)
        {
            _device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i], _uniformBuffersMemory[i]);
        }
    }

    void Engine::_createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(_swapChain.imageCount());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(_swapChain.imageCount());

        if (vkCreateDescriptorPool(_device.device(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void Engine::_createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(_swapChain.imageCount(), _descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_swapChain.imageCount());
        allocInfo.pSetLayouts = layouts.data();

        _descriptorSets.resize(_swapChain.imageCount());
        if (vkAllocateDescriptorSets(_device.device(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < _swapChain.imageCount(); i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = _uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;

            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;

            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;       // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(_device.device(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void Engine::_updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};

        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        //camera view
        glm::vec3 camPos = {0.f, -6.f, -10.f};

        ubo.view = glm::translate(glm::mat4(1.f), camPos);

        //camera projection
        ubo.proj = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
        ubo.proj[1][1] *= -1;

        void *data;
        vkMapMemory(_device.device(), _uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(_device.device(), _uniformBuffersMemory[currentImage]);
    }

    void Engine::_createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

        // //setup push constants
        // VkPushConstantRange push_constant;
        // //this push constant range starts at the beginning
        // push_constant.offset = 0;
        // //this push constant range takes up the size of a MeshPushConstants struct
        // push_constant.size = sizeof(MeshPushConstants);
        // //this push constant range is accessible only in the vertex shader
        // push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        // pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        // pipelineLayoutInfo.pushConstantRangeCount = 1;

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
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);

        // createMaterial(_pipeline->getPipeline(), _pipelineLayout, "defaultmesh");
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
            _model->bind(_commandBuffers[i]);

            vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);

            _model->draw(_commandBuffers[i]);

            // VkBuffer vertexBuffers[] = {_vertexBuffer};
            // VkDeviceSize offsets[] = {0};
            // vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);

            // vkCmdDraw(_commandBuffers[i], static_cast<uint32_t>(_triangleMesh.vertices.size()), 1, 0, 0);

            // drawObjects(_commandBuffers[i], renderables.data(), renderables.size());

            vkCmdEndRenderPass(_commandBuffers[i]);

            if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer");
            }

            // VkDeviceSize offset = 0;
            // vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, &_room.vertexBuffer._buffer, &offset);

            // //make a model view matrix for rendering the object
            // //camera position
            // glm::vec3 camPos = {0.f, -6.f, -10.f};

            // glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
            // //camera projection
            // glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
            // projection[1][1] *= -1;

            // //model rotation
            // glm::mat4 model = glm::rotate(glm::mat4{1.0f}, glm::radians(_frameNumber * 0.4f), glm::vec3(0, 1, 0));

            // //calculate final mesh matrix
            // glm::mat4 mesh_matrix = projection * view * model;

            // MeshPushConstants constants;
            // constants.render_matrix = mesh_matrix;

            // //upload the matrix to the GPU via pushconstants
            // vkCmdPushConstants(_commandBuffers[i], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

            // vkCmdDraw(_commandBuffers[i], _room.vertices.size(), 1, 0, 0);
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

        _updateUniformBuffer(imageIndex);

        result = _swapChain.submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image");
        }

        _frameNumber++;
    }

    void Engine::_loadModels()
    {

        // std::vector<Model::Vertex> roomVertices = _loadFromObj("./assets/viking_room.obj");

        // _createVertexBuffer(_triangleMesh);
        // // _createVertexBuffer(_room);

        // // meshes["room"] = _room;
        // meshes["triangle"] = _triangleMesh;

        // std::vector<Model::Vertex> vertices{
        //     {{0.0f, -0.5f}},
        //     {{0.5f, 0.5f}},
        //     {{-0.5f, 0.5f}}};

        std::vector<Model::Vertex> vertices(3);

        vertices[0].position = {0.0f, -0.5f, 0.f};
        vertices[1].position = {0.5f, 0.5f, 0.f};
        vertices[2].position = {-0.5f, 0.5f, 0.f};

        vertices[0].color = {1.0f, 0.0f, 0.f};
        vertices[1].color = {0.0f, 1.0f, 0.f};
        vertices[2].color = {0.0f, 0.0f, 1.f};

        _model = std::make_unique<Model>(_device, vertices);
    }

    void Engine::initScene()
    {
        // // RenderObject room;
        // // room.mesh = getMesh("room");
        // // // _createVertexBuffer(*room.mesh);
        // // // room.material = getMaterial("defaultmesh");
        // // room.transformMatrix = glm::mat4{1.0f};

        // // renderables.push_back(room);

        // for (int x = -20; x <= 20; x++)
        // {
        //     for (int y = -20; y <= 20; y++)
        //     {
        //         RenderObject tri;
        //         tri.mesh = getMesh("triangle");
        //         _createVertexBuffer(*tri.mesh);
        //         // tri.material = getMaterial("defaultmesh");
        //         glm::mat4 translation = glm::translate(glm::mat4{1.0}, glm::vec3(x, 0, y));
        //         glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(0.2, 0.2, 0.2));
        //         tri.transformMatrix = translation * scale;

        //         renderables.push_back(tri);
        //     }
        // }
    }

    std::vector<Model::Vertex> Engine::_loadFromObj(const char *filename)
    {
        std::vector<Model::Vertex> vertices;

        //attrib will contain the vertex arrays of the file
        tinyobj::attrib_t attrib;
        //shapes contains the info for each separate object in the file
        std::vector<tinyobj::shape_t> shapes;
        //materials contains the information about the material of each shape, but we won't use it.
        std::vector<tinyobj::material_t> materials;

        //error and warning output from the load function
        std::string warn;
        std::string err;

        //load the OBJ file
        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
        //make sure to output the warnings to the console, in case there are issues with the file
        if (!warn.empty())
        {
            std::cout << "WARN: " << warn << std::endl;
        }
        //if we have any error, print it to the console, and break the mesh loading.
        //This happens if the file can't be found or is malformed
        if (!err.empty())
        {
            std::cerr << err << std::endl;
            throw std::runtime_error("failed to load Object");
        }

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++)
        {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {

                //hardcode loading to triangles
                int fv = 3;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++)
                {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    //vertex position
                    tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                    //vertex normal
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

                    //copy it into our vertex
                    Model::Vertex new_vert;
                    new_vert.position.x = vx;
                    new_vert.position.y = vy;
                    new_vert.position.z = vz;

                    new_vert.normal.x = nx;
                    new_vert.normal.y = ny;
                    new_vert.normal.z = nz;

                    //we are setting the vertex color as the vertex normal. This is just for display purposes
                    new_vert.color = new_vert.normal;

                    vertices.push_back(new_vert);
                }
                index_offset += fv;
            }
        }

        return vertices;
    }

    void Engine::_updateTime()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        _dt = std::chrono::duration<float, std::chrono::seconds::period>((currentTime - _lastTime)).count();
        _lastTime = currentTime;
    }
} // namespace vxe
// #include "engine.hpp"

// #define VMA_IMPLEMENTATION
// #include "lib/vk_mem_alloc.h"

// #include <stdexcept>
// #include <array>
// #include <iostream>

// #include <glm/gtx/transform.hpp>

// namespace vxe
// {
//     Engine::Engine()
//     {
//         _createAllocator();
//         _createPipelineLayout();
//         _createPipeline();
//         _loadMeshes();
//         initScene();
//     }

//     Engine::~Engine()
//     {
//         vkDestroyPipelineLayout(_device.device(), _pipelineLayout, nullptr);
//         vmaDestroyBuffer(_allocator, _triangleMesh.vertexBuffer._buffer, _triangleMesh.vertexBuffer._allocation);
//         vmaDestroyBuffer(_allocator, _room.vertexBuffer._buffer, _room.vertexBuffer._allocation);
//         vmaDestroyAllocator(_allocator);
//     }

//     void Engine::run()
//     {
//         while (!_window.shouldClose())
//         {
//             _updateTime();
//             glfwPollEvents();
//             _window.getMousePos(&_mouseX, &_mouseY);
//             _camera.setMousePos(_dt, _mouseX, _mouseY);

//             if (_window.isKeyPressed(GLFW_KEY_ESCAPE)) {
//                 _window.close();
//             }
//             if (_window.isKeyPressed(GLFW_KEY_W))
//             {
//                 _camera.move(10.f, FORWARD, _dt);
//             }
//             if (_window.isKeyPressed(GLFW_KEY_A))
//             {
//                 _camera.move(10.f, LEFT, _dt);
//             }
//             if (_window.isKeyPressed(GLFW_KEY_S))
//             {
//                 _camera.move(10.f, BACKWARD, _dt);
//             }
//             if (_window.isKeyPressed(GLFW_KEY_D))
//             {
//                 _camera.move(10.f, RIGHT, _dt);
//             }
//             _drawFrame();
//         }

//         vkDeviceWaitIdle(_device.device());
//     }

//     Material *Engine::createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name)
//     {
//         Material mat;
//         mat.pipeline = pipeline;
//         mat.pipelineLayout = layout;
//         materials[name] = mat;
//         return &materials[name];
//     }

//     Material *Engine::getMaterial(const std::string &name)
//     {
//         //search for the object, and return nullpointer if not found
//         auto it = materials.find(name);
//         if (it == materials.end())
//         {
//             return nullptr;
//         }
//         else
//         {
//             return &(*it).second;
//         }
//     }

//     Mesh *Engine::getMesh(const std::string &name)
//     {
//         auto it = meshes.find(name);
//         if (it == meshes.end())
//         {
//             return nullptr;
//         }
//         else
//         {
//             return &(*it).second;
//         }
//     }

//     void Engine::drawObjects(VkCommandBuffer cmd, RenderObject *first, int count)
//     {
//         //make a model view matrix for rendering the object
//         //camera view
//         // glm::vec3 camPos = {0.f, -6.f, -10.f};

//         // glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
//         glm::mat4 view = _camera.getViewMatrix();
//         //camera projection
//         glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
//         projection[1][1] *= -1;

//         Mesh *lastMesh = nullptr;
//         Material *lastMaterial = nullptr;
//         for (int i = 0; i < count; i++)
//         {
//             RenderObject &object = first[i];

//             //only bind the pipeline if it doesnt match with the already bound one
//             if (object.material != lastMaterial)
//             {
//                 // std::cout << "NEW MATERIAL" << std::endl;
//                 vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
//                 lastMaterial = object.material;
//             }

//             glm::mat4 model = object.transformMatrix;
//             //final render matrix, that we are calculating on the cpu
//             glm::mat4 mesh_matrix = projection * view * model;

//             MeshPushConstants constants;
//             constants.render_matrix = mesh_matrix;

//             //upload the mesh to the GPU via pushconstants
//             vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

//             //only bind the mesh if it's a different one from last bind
//             if (object.mesh != lastMesh)
//             {
//                 //bind the mesh vertex buffer with offset 0
//                 VkDeviceSize offset = 0;
//                 vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer._buffer, &offset);
//                 lastMesh = object.mesh;
//             }
//             //we can now draw
//             vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
//         }
//     }

//     void Engine::_createAllocator()
//     {
//         VmaAllocatorCreateInfo allocatorInfo = {};
//         allocatorInfo.physicalDevice = _device.getPhysicalDevice();
//         allocatorInfo.device = _device.device();
//         allocatorInfo.instance = _device.getInstance();
//         vmaCreateAllocator(&allocatorInfo, &_allocator);
//     }

//     void Engine::_createPipelineLayout()
//     {
//         VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//         pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//         pipelineLayoutInfo.setLayoutCount = 0;
//         pipelineLayoutInfo.pSetLayouts = nullptr;
//         pipelineLayoutInfo.pushConstantRangeCount = 0;
//         pipelineLayoutInfo.pPushConstantRanges = nullptr;

//         //setup push constants
//         VkPushConstantRange push_constant;
//         //this push constant range starts at the beginning
//         push_constant.offset = 0;
//         //this push constant range takes up the size of a MeshPushConstants struct
//         push_constant.size = sizeof(MeshPushConstants);
//         //this push constant range is accessible only in the vertex shader
//         push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

//         pipelineLayoutInfo.pPushConstantRanges = &push_constant;
//         pipelineLayoutInfo.pushConstantRangeCount = 1;

//         if (vkCreatePipelineLayout(_device.device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) !=
//             VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create pipeline layout!");
//         }
//     }

//     void Engine::_createPipeline()
//     {
//         auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(_swapChain.width(), _swapChain.height());
//         pipelineConfig.renderPass = _swapChain.getRenderPass();
//         pipelineConfig.pipelineLayout = _pipelineLayout;

//         _pipeline = std::make_unique<Pipeline>(
//             _device,
//             "shaders/tri_mesh.vert.spv",
//             "shaders/simple_shader.frag.spv",
//             pipelineConfig);

//         createMaterial(_pipeline->getPipeline(), _pipelineLayout, "defaultmesh");
//     }

//     void Engine::_createCommandBuffers()
//     {

//         _commandBuffers.resize(_swapChain.imageCount());

//         VkCommandBufferAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         allocInfo.commandPool = _device.getCommandPool();
//         allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

//         if (vkAllocateCommandBuffers(_device.device(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to allocate command buffers");
//         }

//         for (int i = 0; i < _commandBuffers.size(); i++)
//         {
//             VkCommandBufferBeginInfo beginInfo{};
//             beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

//             if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("failed to begin recording command buffer!");
//             }

//             VkRenderPassBeginInfo renderPassInfo{};
//             renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//             renderPassInfo.renderPass = _swapChain.getRenderPass();
//             renderPassInfo.framebuffer = _swapChain.getFrameBuffer(i);

//             renderPassInfo.renderArea.offset = {0, 0};
//             renderPassInfo.renderArea.extent = _swapChain.getSwapChainExtent();

//             std::array<VkClearValue, 2> clearValues{};
//             clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
//             clearValues[1].depthStencil = {1.0f, 0};
//             renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
//             renderPassInfo.pClearValues = clearValues.data();

//             vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//             drawObjects(_commandBuffers[i], renderables.data(), renderables.size());

//             // _pipeline->bind(_commandBuffers[i]);

//             // VkDeviceSize offset = 0;
//             // vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, &_room.vertexBuffer._buffer, &offset);

//             // //make a model view matrix for rendering the object
//             // //camera position
//             // glm::vec3 camPos = {0.f, -6.f, -10.f};

//             // glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
//             // //camera projection
//             // glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
//             // projection[1][1] *= -1;

//             // //model rotation
//             // glm::mat4 model = glm::rotate(glm::mat4{1.0f}, glm::radians(_frameNumber * 0.4f), glm::vec3(0, 1, 0));

//             // //calculate final mesh matrix
//             // glm::mat4 mesh_matrix = projection * view * model;

//             // MeshPushConstants constants;
//             // constants.render_matrix = mesh_matrix;

//             // //upload the matrix to the GPU via pushconstants
//             // vkCmdPushConstants(_commandBuffers[i], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

//             // vkCmdDraw(_commandBuffers[i], _room.vertices.size(), 1, 0, 0);

//             vkCmdEndRenderPass(_commandBuffers[i]);

//             if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("failed to record command buffer");
//             }
//         }
//     }

//     void Engine::_drawFrame()
//     {
//         uint32_t imageIndex;
//         auto result = _swapChain.acquireNextImage(&imageIndex);

//         if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//         {
//             throw std::runtime_error("failed to acquire swap chain image");
//         }

//         _createCommandBuffers();

//         result = _swapChain.submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
//         if (result != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to present swap chain image");
//         }

//         _frameNumber++;
//     }

//     void Engine::_loadMeshes()
//     {
//         _triangleMesh.vertices.resize(3);

//         //vertex positions
//         _triangleMesh.vertices[0].position = {0.5f, 0.5f, 0.0f};
//         _triangleMesh.vertices[1].position = {-0.5f, 0.5f, 0.0f};
//         _triangleMesh.vertices[2].position = {0.f, -0.5f, 0.0f};

//         //vertex colors, all green
//         _triangleMesh.vertices[0].color = {1.f, 0.f, 0.0f}; //pure green
//         _triangleMesh.vertices[1].color = {0.f, 1.f, 0.0f}; //pure green
//         _triangleMesh.vertices[2].color = {0.f, 1.f, 0.0f}; //pure green

//         _room.loadFromObj("./assets/viking_room.obj");

//         _uploadMesh(_triangleMesh);
//         _uploadMesh(_room);

//         meshes["room"] = _room;
//         meshes["triangle"] = _triangleMesh;
//     }

//     void Engine::_uploadMesh(Mesh &mesh)
//     {
//         //allocate vertex buffer
//         VkBufferCreateInfo bufferInfo = {};
//         bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         //this is the total size, in bytes, of the buffer we are allocating
//         bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
//         //this buffer is going to be used as a Vertex Buffer
//         bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

//         //let the VMA library know that this data should be writeable by CPU, but also readable by GPU
//         VmaAllocationCreateInfo vmaallocInfo = {};
//         vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

//         //allocate the buffer
//         if (vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
//                             &mesh.vertexBuffer._buffer,
//                             &mesh.vertexBuffer._allocation,
//                             nullptr))
//             ;

//         void *data;
//         vmaMapMemory(_allocator, mesh.vertexBuffer._allocation, &data);
//         memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
//         vmaUnmapMemory(_allocator, mesh.vertexBuffer._allocation);
//     }

//     void Engine::initScene()
//     {
//         RenderObject room;
//         room.mesh = getMesh("room");
//         room.material = getMaterial("defaultmesh");
//         room.transformMatrix = glm::mat4{1.0f};

//         renderables.push_back(room);

//         for (int x = -20; x <= 20; x++)
//         {
//             for (int y = -20; y <= 20; y++)
//             {

//                 RenderObject tri;
//                 tri.mesh = getMesh("triangle");
//                 tri.material = getMaterial("defaultmesh");
//                 glm::mat4 translation = glm::translate(glm::mat4{1.0}, glm::vec3(x, 0, y));
//                 glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(0.2, 0.2, 0.2));
//                 tri.transformMatrix = translation * scale;

//                 renderables.push_back(tri);
//             }
//         }
//     }

//     void Engine::_updateTime()
//     {
//         auto currentTime = std::chrono::high_resolution_clock::now();
//         _dt = std::chrono::duration<float, std::chrono::seconds::period>((currentTime - _lastTime)).count();
//         _lastTime = currentTime;
//     }
// } // namespace vxe
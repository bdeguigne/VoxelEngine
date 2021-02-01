#include "engine.hpp"

#include <stdexcept>
#include <array>

namespace vxe
{
    Engine::Engine()
    {
        _createPipelineLayout();
        _createPipeline();
        _createCommandBuffers();
    }

    Engine::~Engine()
    {
        vkDestroyPipelineLayout(_device.device(), _pipelineLayout, nullptr);
    }

    void Engine::run()
    {
        while (!_window.shouldClose())
        {
            glfwPollEvents();
            _drawFrame();
        }

        vkDeviceWaitIdle(_device.device());
    }

    void Engine::_createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
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
            vkCmdDraw(_commandBuffers[i], 3, 1, 0, 0);

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

        result = _swapChain.submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image");
        }
    }
} // namespace vxe
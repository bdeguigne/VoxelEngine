#pragma once

#include "device.hpp"
#include "mesh.hpp"

#include <string>
#include <vector>

namespace vxe
{
    struct PipelineConfigInfo
    {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline
    {
    public:
        Pipeline(Device &device, const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        void operator=(const Pipeline &) = delete;

        VkPipeline getPipeline() { return _graphicsPipeline; }

        void bind(VkCommandBuffer commandBuffer);

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    private:
        static std::vector<char> _readFile(const std::string &filePath);

        void _createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

        void _createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        Device &_device;
        VkPipeline _graphicsPipeline;
        VkShaderModule _vertShaderModule;
        VkShaderModule _fragShaderModule;
    };
} // namespace vxe
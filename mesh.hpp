#pragma once

#include "types.hpp"
#include <vector>
#include <glm/vec3.hpp>

struct VertexInputDescription
{
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VertexInputDescription getVertexDescription();
};



namespace vxe
{
    class Mesh
    {
    public:
        std::vector<Vertex> vertices;
        AllocatedBuffer vertexBuffer;
    };
} // namespace vxe

// #pragma once

// #include "types.hpp"

// #include <glm/vec3.hpp>

// #include <vector>

// struct VertexInputDescription
// {
//     std::vector<VkVertexInputBindingDescription> bindings;
//     std::vector<VkVertexInputAttributeDescription> attributes;

//     VkPipelineVertexInputStateCreateFlags flags = 0;
// };

// struct Vertex
// {
//     glm::vec3 position;
//     glm::vec3 normal;
//     glm::vec3 color;

//     static VertexInputDescription getVertexDescription();
// };

// namespace vxe
// {
//     class Mesh
//     {
//     public:
//         bool loadFromObj(const char *filename);

//         std::vector<Vertex> vertices;
//         VkBuffer vertexBuffer;
//         VkDeviceMemory vertexBufferMemory;
//     };
// } // namespace vxe

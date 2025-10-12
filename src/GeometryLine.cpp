#include "GeometryLine.h"
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

Line::Line(float scale, int segment_count) {
    buildLine(scale, segment_count);

    // Set up binding description
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    bindingDescriptions.push_back(bindingDescription);

    // Set up attribute descriptions
    // Position
    VkVertexInputAttributeDescription iaDesc{};
    iaDesc.binding = 0;
    iaDesc.location = 0;
    iaDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
    iaDesc.offset = offsetof(Vertex, pos);
    attributeDescriptions.push_back(iaDesc);
    // Color
    iaDesc.binding = 0;
    iaDesc.location = 1;
    iaDesc.format = VK_FORMAT_R32_SFLOAT;
    iaDesc.offset = offsetof(Vertex, hue);
    attributeDescriptions.push_back(iaDesc);
}

void Line::buildLine(float scale, int segment_count) {
    // 1. Setup and Calculation

    // 'scale' now defines the length of a single segment.
    const float segment_length = scale;

    // For N segments, we need N + 1 vertices (fencepost problem).
    const int vertex_count = segment_count + 1;

    // Clear existing data
    vertices.clear();
    indices.clear();

    // The line starts at (0, 0, 0) and extends along the positive X-axis.
    const float start_x = 0.0f;
    const float start_y = 0.0f; // Line is in the XY plane (y=0)

    // 2. Generate Vertices (Points along the X-axis)

    for (int i = 0; i < vertex_count; ++i) {
        Vertex vert;

        // Calculate position: step 'i' times by the segment_length
        float x = start_x + (float)i * segment_length;

        // The line is flat on the XY plane
        vert.pos = glm::vec3(x, start_y, 0.0f);

        // Calculate a normalized value for color/hue (u coordinate, 0.0 to 1.0)
        // This allows the line to be colored or textured along its length.
        float u = (float)i / (float)segment_count;
        vert.hue = u;

        vertices.push_back(vert);
    }

    // 3. Generate Indices (Sequential connection for LINE_STRIP)

    // To render a continuous line strip using an index buffer, we simply 
    // index every vertex sequentially. This results in the topology:
    // Line 1: (Index 0) -> (Index 1)
    // Line 2: (Index 1) -> (Index 2)
    // ...
    // Line N: (Index N-1) -> (Index N)
    for (uint32_t i = 0; i < vertex_count; ++i) {
        indices.push_back(i);
    }

    // If you were using vkCmdDraw without an index buffer, you would skip 
    // step 3 and just rely on the vertex buffer order. However, generating 
    // indices is good practice for consistent draw call pipelines.
}

// -- Interface Implementations --

const void* Line::getVertexData() const {
    return vertices.data();
}

uint32_t Line::getVertexCount() const {
    return (uint32_t) vertices.size();
}

uint16_t Line::getVertexStride() const {
    return sizeof(Vertex);
}

const uint32_t* Line::getIndexData() const {
    return indices.data();
}

uint32_t Line::getIndexCount() const {
    return (uint32_t) indices.size();
}

const std::vector<VkVertexInputAttributeDescription>& Line::getAttributeDescriptions() const {
    return attributeDescriptions;
}

const std::vector<VkVertexInputBindingDescription>& Line::getBindingDescriptions() const {
    return bindingDescriptions;
}

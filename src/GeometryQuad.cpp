#include "GeometryQuad.h"
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

Quad::Quad(float width, float height) {
    buildQuad(width, height);

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

void Quad::buildQuad(float width, float height) {
    float w = width / 2.0f;
    float h = height / 2.0f;

#define QUAD_VERTEX_COUNT 4
    // Quad in the XY plane (Z = 0)
    float positions[QUAD_VERTEX_COUNT][3] = {
        {-w, -h, 0.0f}, // bottom-left
        { w, -h, 0.0f}, // bottom-right
        { w,  h, 0.0f}, // top-right
        {-w,  h, 0.0f}  // top-left
    };

    vertices.clear();
    indices.clear();

    for (int i = 0; i < QUAD_VERTEX_COUNT; ++i) {
        Vertex vert;
        vert.pos = glm::vec3(positions[i][0], positions[i][1], positions[i][2]);
        vert.hue = ((float)i / QUAD_VERTEX_COUNT);
        vertices.push_back(vert);
    }

    // Two triangles make the quad
    uint32_t indexList[6] = {
        0, 1, 2,
        0, 2, 3
    };

    for (int i = 0; i < 6; i++) {
        indices.push_back(indexList[i]);
    }
}

// -- Interface Implementations --

Geometry::GEOTYPE Quad::getGeoType() const
{
    return GEOTYPE::QUAD;
}

const void* Quad::getVertexData() const {
    return vertices.data();
}

uint32_t Quad::getVertexCount() const {
    return (uint32_t) vertices.size();
}

uint16_t Quad::getVertexStride() const {
    return sizeof(Vertex);
}

const uint32_t* Quad::getIndexData() const {
    return indices.data();
}

uint32_t Quad::getIndexCount() const {
    return (uint32_t) indices.size();
}

const std::vector<VkVertexInputAttributeDescription>& Quad::getAttributeDescriptions() const {
    return attributeDescriptions;
}

const std::vector<VkVertexInputBindingDescription>& Quad::getBindingDescriptions() const {
    return bindingDescriptions;
}

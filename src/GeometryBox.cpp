#include "GeometryBox.h"
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

Box::Box(float width, float height, float depth) {
    buildBox(width, height, depth);

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

void Box::buildBox(float width, float height, float depth) {
    float w = width / 2.0f;
    float h = height / 2.0f;
    float d = depth / 2.0f;

#define BOX_VERTEX_COUNT 8
    float positions[BOX_VERTEX_COUNT][3] = {
        {-w, -h, -d}, {w, -h, -d}, {w,  h, -d}, {-w,  h, -d}, // front
        {-w, -h,  d}, {w, -h,  d}, {w,  h,  d}, {-w,  h,  d}   // back
    };
    for (int i = 0; i < BOX_VERTEX_COUNT; ++i) {
        Vertex vert;
        vert.pos = glm::vec3(positions[i][0], positions[i][1], positions[i][2]);
        vert.hue = ((float)i / BOX_VERTEX_COUNT);
        vertices.push_back(vert);
    }
    //uint32_t indexList[6][6] = {
    //    {0, 3, 1, 1,3,2}, // front
    //    {0,1,4,4,1,5}, // top
    //    {0,3,4,4,3,7}, // left
    //    {4,7,5,5,7,6}, // back
    //    {2,6,3,3,6,7}, // bottom
    //    {1,2,6,6,5,1}  // right
    //};
    //// Clockwise version
    //uint32_t indexList[6][6] = {
    //{0, 1, 3, 1, 2, 3}, // front face  (Z = -d)
    //{1, 5, 2, 5, 6, 2}, // right face  (X = +w)
    //{5, 4, 6, 4, 7, 6}, // back face   (Z = +d)
    //{4, 0, 7, 0, 3, 7}, // left face   (X = -w)
    //{3, 2, 7, 2, 6, 7}, // top face    (Y = +h)
    //{4, 5, 0, 5, 1, 0}  // bottom face (Y = -h)
    //};
    // Counter-Clockwise version
    uint32_t indexList[6][6] = {
    {0, 3, 1, 1, 3, 2}, // front face  (Z = -d)
    {1, 2, 5, 5, 2, 6}, // right face  (X = +w)
    {5, 6, 4, 4, 6, 7}, // back face   (Z = +d)
    {4, 7, 0, 0, 7, 3}, // left face   (X = -w)
    {3, 7, 2, 2, 7, 6}, // top face    (Y = +h)
    {4, 0, 5, 5, 0, 1}  // bottom face (Y = -h)
    };

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            indices.push_back(indexList[i][j]);
        }
    }
}

// -- Interface Implementations --

Geometry::GEOTYPE Box::getGeoType() const
{
    return Geometry::GEOTYPE();
}

const void* Box::getVertexData() const {
    return vertices.data();
}

uint32_t Box::getVertexCount() const {
    return (uint32_t) vertices.size();
}

uint16_t Box::getVertexStride() const {
    return sizeof(Vertex);
}

const uint32_t* Box::getIndexData() const {
    return indices.data();
}

uint32_t Box::getIndexCount() const {
    return (uint32_t) indices.size();
}

const std::vector<VkVertexInputAttributeDescription>& Box::getAttributeDescriptions() const {
    return attributeDescriptions;
}

const std::vector<VkVertexInputBindingDescription>& Box::getBindingDescriptions() const {
    return bindingDescriptions;
}

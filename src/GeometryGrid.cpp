#include "GeometryGrid.h"
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

Grid::Grid(float scale, int row, int col) {
    buildGrid(scale, row, col);

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

void Grid::buildGrid(float scale, int row, int col) {
    // Determine the size of each grid *segment* (quad)
    const float seg_w = scale;
    const float seg_h = scale;

    // The total extent of the grid (now used just for reference, not offsetting)
    // const float total_width = (float)col * seg_w;
    // const float total_height = (float)row * seg_h;

    // STARTING CORNER CHANGE:
    // The grid now starts at (0, 0, 0).
    const float start_x = 0.0f;
    const float start_y = 0.0f;

    vertices.clear();
    indices.clear();

    // 1. Generate Vertices
    // We need (col + 1) vertices horizontally and (row + 1) vertices vertically.
    for (int j = 0; j <= row; ++j) { // Iterate over rows (Y-axis)
        for (int i = 0; i <= col; ++i) { // Iterate over columns (X-axis)
            Vertex vert;

            // Calculate position
            // Start at (0, 0) and step by 'seg_w' and 'seg_h'.
            float x = start_x + (float)i * seg_w;
            float y = start_y + (float)j * seg_h;

            vert.pos = glm::vec3(x, y, 0.0f);

            // Calculate texture/color coordinate (hue)
            float u = (float)i / (float)col;
            float v = (float)j / (float)row;

            vert.hue = u; // Using 'u' for the hue

            vertices.push_back(vert);
        }
    }

    // 2. Generate Indices (Two triangles for each quad segment)
    // This section remains exactly the same as the vertex indexing logic hasn't changed.
    int vertex_per_row = col + 1; // Number of vertices in a single row

    for (int j = 0; j < row; ++j) {
        for (int i = 0; i < col; ++i) {

            // Calculate the 4 vertex indices for the current quad segment (i, j)
            uint32_t bl = j * vertex_per_row + i;
            uint32_t br = bl + 1;
            uint32_t tl = (j + 1) * vertex_per_row + i;
            uint32_t tr = tl + 1;

            // First Triangle (BL, BR, TR)
            indices.push_back(bl);
            indices.push_back(br);
            indices.push_back(tr);

            // Second Triangle (BL, TR, TL)
            indices.push_back(bl);
            indices.push_back(tr);
            indices.push_back(tl);
        }
    }
}

// -- Interface Implementations --

const void* Grid::getVertexData() const {
    return vertices.data();
}

uint32_t Grid::getVertexCount() const {
    return (uint32_t) vertices.size();
}

uint16_t Grid::getVertexStride() const {
    return sizeof(Vertex);
}

const uint32_t* Grid::getIndexData() const {
    return indices.data();
}

uint32_t Grid::getIndexCount() const {
    return (uint32_t) indices.size();
}

const std::vector<VkVertexInputAttributeDescription>& Grid::getAttributeDescriptions() const {
    return attributeDescriptions;
}

const std::vector<VkVertexInputBindingDescription>& Grid::getBindingDescriptions() const {
    return bindingDescriptions;
}

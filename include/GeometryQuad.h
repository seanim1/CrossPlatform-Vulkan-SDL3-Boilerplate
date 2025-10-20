#pragma once
#include "GeometryBase.h"
#include <vector>

class Quad : public Geometry {
public:
    Quad(float width, float height);

    Geometry::GEOTYPE getGeoType() const override;

    const void* getVertexData() const override;
    uint32_t getVertexCount() const override;
    uint16_t getVertexStride() const override;

    const uint32_t* getIndexData() const override;
    uint32_t getIndexCount() const override;

    const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const override;
    const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const override;

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;

    void buildQuad(float width, float height);
};

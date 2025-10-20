#pragma once
#include "GeometryBase.h"
#include <vector>

class Box : public Geometry {
public:
    Box(float width, float height, float depth);

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

    void buildBox(float width, float height, float depth);
};

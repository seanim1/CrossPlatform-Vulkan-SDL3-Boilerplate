
#pragma once
#include "Transformation.h"
#include "Global.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    float hue;
    //glm::vec3 normal; // I would need 24 vertices instead of just 8 if I wanted a flat shaded cube
};
class Geometry : public Transformation {
public:
    enum GEOTYPE {
        QUAD,
        BOX
    };

    virtual ~Geometry() = default;

    // Pure Virtual Function
    virtual GEOTYPE getGeoType() const = 0;

    virtual const void* getVertexData() const = 0;
    virtual uint32_t getVertexCount() const = 0;
    virtual uint16_t getVertexStride() const = 0;

    virtual const uint32_t* getIndexData() const = 0;
    virtual uint32_t getIndexCount() const = 0;

    virtual const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const = 0;
    virtual const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const = 0;
};

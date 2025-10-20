#pragma once

#include <glm/glm.hpp>

class GameCamera {
public:
    virtual ~GameCamera() = default;

    virtual void SetPosition(const glm::vec3& pos);
    void SetTarget(const glm::vec3& tgt);
    void SetAspectRatio(float ratio);

    glm::vec3 GetPosition() const;
    glm::vec3 GetDirection() const;
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;

protected:
    GameCamera(); // Protected constructor: abstract base

    virtual void UpdateProjectionMatrix() = 0; // Each subclass implements this
    void UpdateViewMatrix();

    static constexpr float BASE_ZOOM_ORTHO = 1.;
    static constexpr float BASE_DIST_ORTHO = 4.;

    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    float aspect;
    float nearClip;
    float farClip;
    glm::mat4 view;
    glm::mat4 proj;
};

// ------------------------------
// Perspective Camera
// ------------------------------
class GameCameraPerspective : public GameCamera {
public:
    GameCameraPerspective(float fovDegrees, float aspectRatio);

    void SetFov(float fovDegrees);
    void SetClippingPlanes(float nearPlane, float farPlane);

private:
    void UpdateProjectionMatrix() override;

    float fov;
};

// ------------------------------
// Orthographic Camera
// ------------------------------
class GameCameraOrthographic : public GameCamera {
public:
    GameCameraOrthographic(float aspectRatio);

    void SetBounds(float left, float right, float bottom, float top);
    void SetClippingPlanes(float nearPlane, float farPlane);

private:
    void SetPosition(const glm::vec3& pos) override;
    void UpdateProjectionMatrix() override;

    float left;
    float right;
    float bottom;
    float top;
};

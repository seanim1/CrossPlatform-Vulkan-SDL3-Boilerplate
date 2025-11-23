#pragma once

#include <glm/glm.hpp>

class GameCamera {
public:
    enum DirectionConfig {
        FREE_CAM,
        TARGET_CAM
    };
    virtual ~GameCamera() = default;

    virtual void SetPosition(const glm::vec3& pos);
    void GameCamera::SetDirConfig(DirectionConfig config);
    void SetTarget(const glm::vec3& tgt);
    void SetAspectRatio(float ratio);
    void SetRotation(glm::vec3 rotation);

    glm::vec3 GetPosition() const;
    glm::vec3 GetDirection() const;
    glm::vec3 GetRotation() const;
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;

protected:
    GameCamera(); // Protected constructor: abstract base

    virtual void UpdateProjectionMatrix() = 0; // Each subclass implements this
    void UpdateViewMatrix();
    void UpdateCameraVectors();

    static constexpr float BASE_ZOOM_ORTHO = 1.;
    static constexpr float BASE_DIST_ORTHO = 4.;

    glm::vec3 position;
    glm::vec3 target;

    float aspect;
    float nearClip;
    float farClip;
    glm::mat4 view;
    glm::mat4 proj;

    bool useOrtho = false;

private:
    glm::vec3 GetEulerFromFrontUp(const glm::vec3& front, const glm::vec3& up);

    DirectionConfig camDirConfig;

    float yaw = -90.0f;   // left/right rotation
    float pitch = 0.0f;   // up/down rotation
    float roll = 0.0f;

    glm::vec3 up;
    glm::vec3 front;
    glm::vec3 right;
};

// ------------------------------
// Perspective Camera
// ------------------------------
class GameCameraPerspective : public GameCamera {
public:
    GameCameraPerspective(float fovDegrees, float aspectRatio);

    void SetFov(float fovDegrees);
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

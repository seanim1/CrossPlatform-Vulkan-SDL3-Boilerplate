#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "GameCamera.h"
#include <glm/gtc/matrix_transform.hpp>

// ------------------------------
// Base class
// ------------------------------

GameCamera::GameCamera()
    : position(0.0f, 0.0f, 5.0f),
    target(0.0f, 0.0f, 0.0f),
    up(0.0f, 1.0f, 0.0f),
    aspect(1.78),
    nearClip(0.1f),
    farClip(100.0f)
{
    UpdateViewMatrix();
}

void GameCamera::SetPosition(const glm::vec3& pos) {
    position = pos;
    UpdateViewMatrix();
}

void GameCamera::SetTarget(const glm::vec3& tgt) {
    target = tgt;
    UpdateViewMatrix();
}

void GameCamera::SetAspectRatio(float ratio) {
    aspect = ratio;
    UpdateProjectionMatrix();
}

glm::vec3 GameCamera::GetPosition() const {
    return position;
}

glm::vec3 GameCamera::GetDirection() const {
    return glm::normalize(target - position);
}

const glm::mat4& GameCamera::GetViewMatrix() const {
    return view;
}

const glm::mat4& GameCamera::GetProjectionMatrix() const {
    return proj;
}

void GameCamera::UpdateViewMatrix() {
    view = glm::lookAt(position, target, up);
}

// ------------------------------
// Perspective Camera
// ------------------------------

GameCameraPerspective::GameCameraPerspective(float fovDegrees, float aspectRatio)
    : fov(glm::radians(fovDegrees))
{
    SetAspectRatio(aspectRatio);
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void GameCameraPerspective::SetFov(float fovDegrees) {
    fov = glm::radians(fovDegrees);
    UpdateProjectionMatrix();
}

void GameCameraPerspective::UpdateProjectionMatrix() {
    proj = glm::perspective(fov, aspect, nearClip, farClip);
    proj[1][1] *= -1.0f; // Flip Y for Vulkan
}

// ------------------------------
// Orthographic Camera
// ------------------------------

// New int-based constructor
GameCameraOrthographic::GameCameraOrthographic(float aspectRatio)
{
    SetAspectRatio(aspectRatio);
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void GameCameraOrthographic::SetPosition(const glm::vec3& pos) {
    GameCamera::SetPosition(pos); // position value needs to be updated first
    UpdateProjectionMatrix();
}

void GameCameraOrthographic::UpdateProjectionMatrix() {
    // Adjust zoom based on distance from target (Blender Style orthographic camera, otherwise, I would just make a separate screen space camera)
    float distance = glm::length(target - position);
    float zoom = BASE_ZOOM_ORTHO * (distance / BASE_DIST_ORTHO);
    proj = glm::ortho(-zoom * aspect, zoom * aspect, -zoom, zoom, nearClip, farClip); // proj = glm::ortho(left, right, bottom, top, nearClip, farClip);
    proj[1][1] *= -1.0f; // Flip Y for Vulkan
}

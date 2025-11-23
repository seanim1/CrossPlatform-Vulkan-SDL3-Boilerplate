#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "GameCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
// ------------------------------
// Base class
// ------------------------------

GameCamera::GameCamera()
    : position(0.0f, 0.0f, 5.0f),
    target(0.0f, 0.0f, 0.0f),
    up(0.0f, 1.0f, 0.0f),
    front(0.0f, 0.0, 1.0f),
    aspect(1.78),
    nearClip(0.1f),
    farClip(100.0f),
    camDirConfig(FREE_CAM)
{
    UpdateViewMatrix();
    glm::vec3 angles = GetEulerFromFrontUp(front, up);
    SetRotation(angles);
}

void GameCamera::SetDirConfig(DirectionConfig config) {
    camDirConfig = config;
}

void GameCamera::SetPosition(const glm::vec3& pos) {
    position = pos;
    UpdateViewMatrix();
}

void GameCamera::SetTarget(const glm::vec3& tgt) {
    target = tgt;
}

void GameCamera::SetAspectRatio(float ratio) {
    aspect = ratio;
    UpdateProjectionMatrix();
}

void GameCamera::SetRotation(glm::vec3 rotation)
{
    yaw = rotation.x;
    pitch = rotation.y;
    roll = rotation.z;

    UpdateCameraVectors();   // regenerate orientation from yaw/pitch/roll
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

glm::vec3 GameCamera::GetRotation() const {
    return glm::vec3(yaw, pitch, roll);
}

/*
 * Must be called whenever a camera direction is affected
 * @param n - integer input
 * @return factorial of n
 */
void GameCamera::UpdateViewMatrix() {
    switch (camDirConfig) {
    case FREE_CAM:
        view = glm::lookAt(position, position + front, up);
        break;
    case TARGET_CAM:
        view = glm::lookAt(position, target, up);
        break;
    default:
        view = glm::lookAt(position, target, up);
    }
}

/*
 * Must be called whenever a camera direction/rotation is affected
 * Update Camera's Up, Right, Front vectors whenever a camera direction changes
 * @param n - integer input
 * @return factorial of n
 */
void GameCamera::UpdateCameraVectors()
{
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float rollRad = glm::radians(roll);

    // Yaw is world-space
    glm::quat qYaw = glm::angleAxis(yawRad, glm::vec3(0, 1, 0));

    // Pitch & roll are in *local space*, so rotate their axes by yaw first
    glm::vec3 localRight = qYaw * glm::vec3(1, 0, 0);
    glm::vec3 localForward = qYaw * glm::vec3(0, 0, -1);

    glm::quat qPitch = glm::angleAxis(pitchRad, localRight);
    glm::quat qRoll = glm::angleAxis(rollRad, localForward);

    glm::quat orientation = qYaw * qPitch * qRoll;

    front = glm::normalize(orientation * glm::vec3(0, 0, -1));
    right = glm::normalize(orientation * glm::vec3(1, 0, 0));
    up = glm::normalize(orientation * glm::vec3(0, 1, 0));
}

// Returns yaw, pitch, roll (in degrees) from front and up vectors
glm::vec3 GameCamera::GetEulerFromFrontUp(const glm::vec3& front, const glm::vec3& up)
{
    // Ensure vectors are normalized
    glm::vec3 f = glm::normalize(front);
    glm::vec3 u = glm::normalize(up);

    // Compute right vector
    glm::vec3 r = glm::normalize(glm::cross(f, u));

    // Recompute true up (orthogonal)
    glm::vec3 camUp = glm::cross(r, f);

    // Build rotation matrix (columns: right, up, -front)
    glm::mat3 rotMat(r, camUp, -f); // column-major

    // Convert to quaternion
    glm::quat q(rotMat);

    // Convert quaternion to Euler angles (radians)
    glm::vec3 eulerRad = glm::eulerAngles(q);

    // Convert to degrees
    glm::vec3 eulerDeg = glm::degrees(eulerRad);

    // eulerDeg.x = pitch, eulerDeg.y = yaw, eulerDeg.z = roll
    return eulerDeg;
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

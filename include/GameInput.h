#pragma once
#include "GameCamera.h"
#include <iostream>

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <array>
#include <stdexcept>

class GameInput {
private:
    std::array<bool, SDL_SCANCODE_COUNT> keys;
    bool mouseButtons[5] = { false }; // Support up to 5 mouse buttons
    glm::ivec2 mouseCoord = glm::ivec2(0);
    int mouseWheel = 0;
public:
    GameInput();
    // A getter function to safely expose the state
    const std::array<bool, SDL_SCANCODE_COUNT>& getKeys() const {
        return keys;
    }
    void ProcessEvent(const SDL_Event& event);
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsMousePressed(Uint8 button) const;
    glm::ivec2 GetMousePosition() const;
    int GetMouseWheelDelta() const;
};
#version 450
#include "Global.comp"
#include "Graphics.comp"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inHue;

layout(location = 0) out vec3 fragColor;

// Range: same as cos()
float wavy(float x) {
    return sin(x) + 0.5 * sin(2.3 * x + 1.0) + 0.25 * sin(4.7 * x - 0.5);
}

void main() {

    vec2 aspectRatio = vec2(SCREEN_WIDTH / float(SCREEN_HEIGT), 1.);
    gl_Position = ubo.proj * ubo.view * ubo.models[MODEL_ID] * vec4(inPosition, 1.0);
    fragColor = hsv2rgb_smooth(Vec3T(ubo.elapsedTime * 0.3 + inHue , 1., 1.));
}


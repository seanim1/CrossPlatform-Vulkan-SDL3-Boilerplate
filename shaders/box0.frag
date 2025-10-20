#version 450
#include "Global.comp"
#include "SDF.comp"
#include "Graphics.comp"

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

// Scene distance function
float map(vec3 p) {
    // Blend shapes over time
    float t = 0.5 + 0.5 * sin(ubo.elapsedTime);

    float torus = sdTorus(p, vec2(0.7, 0.15));
    float box   = sdBox(p, vec3(0.7));
    return mix(torus, box, t);
}

// Raymarching loop
float raymarch(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 128; i++) {
        vec3 p = ro + t * rd;
        //float d = sdTorus(p, vec2(0.6, 0.09)); // Torus in local space
        float d = map(p);
        if (d < 0.0001) return t;
        if (t > 100.0) break;
        t += d;
    }
    return -1.0;
}

// Normal estimation
vec3 getNormal(vec3 p)
{
    float h = 0.001;
    vec2 k = vec2(1, -1);
    return normalize(
        k.xyy * map(p + k.xyy * h) +
        k.yyx * map(p + k.yyx * h) +
        k.yxy * map(p + k.yxy * h) +
        k.xxx * map(p + k.xxx * h)
    );
}

// Lighting
vec3 shade(vec3 ro, vec3 rd, float t)
{
    vec3 p = ro + t * rd;
    vec3 n = getNormal(p);
    vec3 lightDir = ubo.dirLightDir;
    float diff = max(dot(n, lightDir), 0.0);

    float ambient = 0.8;
    vec3 baseColor = hsv2rgb_smooth(Vec3T(0.55, 1., 1.));
    return baseColor * (ambient + diff);
}
void main() {
    //debugPrintfEXT("uv: %v2f, frag: %v2f, scrn: %v2i\n", uv, gl_FragCoord.xy, SCREEN_DIM);
    
    // Step 1: Reconstruct world-space ray
    // Normalized pixel coordinates (from 0 to 1)
    // Inverse matrices
    mat4 invView = inverse(ubo.view);
    mat4 invProj = inverse(ubo.proj);
    mat4 invModel = inverse(ubo.models[MODEL_ID]);

    // Reconstruct ray in world space
    vec2 uv = gl_FragCoord.xy / SCREEN_DIM;
    vec2 ndc = uv * 2.0 - 1.0;
    ndc.y = ndc.y; // Flip Y for Vulkan NDC
    //debugPrintfEXT("uv: %v2f, frag: %v2f, scrn: %v2i\n", uv, gl_FragCoord.xy, SCREEN_DIM);

    vec4 clip = vec4(ndc, -1.0, 1.0); // Ray on near plane
    vec4 viewPos = invProj * clip;
    viewPos /= viewPos.w;

    vec3 rayOrigin = ubo.camPos;
    vec3 rayTarget = (invView * viewPos).xyz;
    vec3 rayDir = normalize(rayTarget - rayOrigin);

    // Transform ray into local box space
    vec3 localRayOrigin = (invModel * vec4(rayOrigin, 1.0)).xyz;
    vec3 localRayDir = normalize((invModel * vec4(rayDir, 0.0)).xyz);

    float t = raymarch(localRayOrigin, localRayDir);
    // Raymarch in world space
    
    //for (int i = 0; i < 128; ++i) {
    //    vec3 p = rayOrigin + t * rayDir;
    //    //vec3 p = localRayOrigin + t * localRayDir;
//
    //    // Transform the point into the object's local space inside the SDF
    //    vec3 localP = (inverse(ubo.models[MODEL_ID]) * vec4(p, 1.0)).xyz;
    //    float d = sdTorus(localP, vec2(0.6, 0.09)); // Torus in local space
    //    if (d < 0.0001) {
    //        hit = true;
    //        break;
    //    }
    //    t += d;
    //    if (t > 100.0) break;
    //}

    bool hit = (t > 0.0);
    // Output
    if (hit) {
        vec3 col = shade(localRayOrigin, localRayDir, t);
        outColor = vec4(col, 1.0);
    } else {
        outColor = vec4(0.89, 0.34, 0.34, 1.0);
    }


}

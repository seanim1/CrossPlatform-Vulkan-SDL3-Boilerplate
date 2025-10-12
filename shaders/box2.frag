#version 450
#include "Global.comp"
#include "SDF.comp"
#include "Graphics.comp"

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

// b : [0., 0.5], c : [0., 1.], d : [0., 1.]
vec3 colorGradient( in float t, in vec3 b, in vec3 c, in vec3 d )
{
    return 0.5 + b*cos( 6.283185*(c*t+d) );
}

void main() {
    //debugPrintfEXT("uv: %v2f, frag: %v2f, scrn: %v2i\n", uv, gl_FragCoord.xy, SCREEN_DIM);

    // Reconstruct ray in world space
    vec2 uv = gl_FragCoord.xy / SCREEN_DIM;
    uv = uv * 2.0 - 1.0;
    uv.y = uv.y; // Flip Y for Vulkan NDC

    uv = 2. * fract(uv * 2.) - 1.; // 2*fract(1*x)-1.0. Maps: -1 to 0, 0 to 1,to -1, 1
    float aspectRatio = float(SCREEN_DIM.x) / SCREEN_DIM.y;
    uv.x *= aspectRatio;
    float d = length(uv); // y = x
    vec3 col; // = palette( d, vec3(0.5,0.5,0.5),vec3(0.5,0.5,0.5),vec3(1.0,1.0,1.0),vec3(0.0,0.33,0.67) );
    col = colorGradient( d, vec3(0.5,0.5,0.5), vec3(1.,1.,1.), vec3(0., 0.33,0.66) );
    d = sin(d*8.+ubo.elapsedTime) / 8.; // y = x-0.5
    d = abs(d); // abs(x-0.5)
    //d = step(0.1, d); // step(0.1,abs(x-0.5))
    //d = smoothstep(0.0,0.1,d); // smoothstep(0,0.1,abs(x-0.5));
    d = 0.01/d;

    col *= d;
    // Output
    //col = 0.5 + 0.5*cos(ubo.elapsedTime+uv.xyx+vec3(0,2,4));

    // Output to screen
    outColor = vec4(col, 1.0);


}

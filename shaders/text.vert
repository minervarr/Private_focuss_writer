#version 450

// Vertex attributes
layout(location = 0) in vec2 inPosition;    // Screen position
layout(location = 1) in vec2 inTexCoord;    // Texture coordinates
layout(location = 2) in uint inFragmentMode; // 0=top, 1=bottom

// Output to fragment shader
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragMode;
layout(location = 2) out float fragOpacity;

// Push constants for transformation
layout(push_constant) uniform PushConstants {
    mat4 projection;
    float opacity;  // Opacity for current line
} pc;

void main() {
    gl_Position = pc.projection * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    fragMode = inFragmentMode;
    fragOpacity = pc.opacity;
}

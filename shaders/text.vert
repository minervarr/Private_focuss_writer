#version 450

// Vertex attributes
layout(location = 0) in vec2 inPosition;   // Screen position
layout(location = 1) in vec2 inTexCoord;   // Texture coordinates

// Output to fragment shader
layout(location = 0) out vec2 fragTexCoord;

// Push constants for transformation
layout(push_constant) uniform PushConstants {
    mat4 projection;
} pc;

void main() {
    gl_Position = pc.projection * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
}

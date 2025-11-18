#version 450

// Input from vertex shader
layout(location = 0) in vec2 fragTexCoord;

// Output color
layout(location = 0) out vec4 outColor;

// Font atlas texture (grayscale)
layout(binding = 0) uniform sampler2D fontAtlas;

void main() {
    // Sample the alpha from the atlas (it's a grayscale texture)
    float alpha = texture(fontAtlas, fragTexCoord).r;

    // Output white text with sampled alpha
    outColor = vec4(1.0, 1.0, 1.0, alpha);
}

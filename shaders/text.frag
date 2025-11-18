#version 450

// Input from vertex shader
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint fragMode;
layout(location = 2) in float fragOpacity;

// Output color
layout(location = 0) out vec4 outColor;

// Font atlas texture (grayscale)
layout(binding = 0) uniform sampler2D fontAtlas;

void main() {
    vec2 adjustedTexCoord = fragTexCoord;

    // Fragmentación: mostrar solo mitad superior, inferior, o completo
    if (fragMode == 0) {
        // Mitad superior: Y de 0.0 a 0.5
        adjustedTexCoord.y *= 0.5;
    } else if (fragMode == 1) {
        // Mitad inferior: Y de 0.5 a 1.0
        adjustedTexCoord.y = 0.5 + adjustedTexCoord.y * 0.5;
    }
    // fragMode == 2 (None): usar texCoord sin modificar

    // Sample the alpha from the atlas with adjusted coordinates
    float alpha = texture(fontAtlas, adjustedTexCoord).r;

    // Apply opacity from push constant
    alpha *= fragOpacity;

    // Output white text with sampled alpha
    outColor = vec4(1.0, 1.0, 1.0, alpha);
}

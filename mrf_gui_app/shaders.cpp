#include "shaders.h"
// Define the vertex shader source
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos; // Position attribute
layout(location = 1) in vec2 aTexCoord; // Texture coordinate attribute

out vec2 TexCoord; // Output to the fragment shader

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0); // Transform vertex position
    TexCoord = aTexCoord; // Pass texture coordinates to fragment shader
}
)";

// Define the fragment shader source
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // Input from vertex shader

uniform sampler2D texture1; // Texture sampler

void main() {
    FragColor = texture(texture1, TexCoord); // Get color from texture
}
)";

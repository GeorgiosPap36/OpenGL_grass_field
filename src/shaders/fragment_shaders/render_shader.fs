#version 430 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D renderTexture;

void main() {
    vec4 textureValue = texture(renderTexture, uv);
    // FragColor = vec4(vec3(textureValue), 1.0);
    FragColor = textureValue;
}
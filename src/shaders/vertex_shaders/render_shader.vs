#version 430 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec2 UV;

out vec2 uv;

void main() {
    uv = UV;
    gl_Position = vec4(Pos, 1.0);
}
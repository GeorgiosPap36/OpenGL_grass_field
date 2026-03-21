#version 430 core
layout(location = 0) in vec3 Pos;

out vec3 uv;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vec4 pos = projection * view * vec4(Pos, 1.0f);
    uv = vec3(Pos.x, Pos.y, -Pos.z);
    
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
}
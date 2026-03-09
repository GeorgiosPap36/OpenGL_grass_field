#version 430 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 uv;
out vec3 normal;

void main() {

	// In backpack.obj v was inverted
	// uv = vec2(UV.x, 1 - UV.y);
	uv = UV;

	normal = mat3(transpose(inverse(model))) * Normal;

	gl_Position = projection * view * model * vec4(Pos, 1.0);
}
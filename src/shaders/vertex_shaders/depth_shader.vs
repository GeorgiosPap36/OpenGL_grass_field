#version 430 core

struct InstancedData {
	vec4 position;
	vec4 color;
};

layout (location = 0) in vec3 Pos;
layout(std430, binding = 4) buffer InstancedDataBuffer {
    InstancedData instancedData[];
}; 

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    vec3 tempPos = instancedData[gl_InstanceID].position.xyz;
    gl_Position = lightSpaceMatrix * model * vec4(Pos + tempPos, 1.0);
}  
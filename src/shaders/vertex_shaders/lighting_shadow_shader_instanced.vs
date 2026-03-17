#version 430 core


struct InstancedData {
	vec4 position;
	vec4 color;
};

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;

layout(std140, binding = 3) uniform FrameUniforms {
    mat4 projection;
    mat4 view;

	mat4 lightSpaceMatrix;
	
	vec3 cameraPos;
};

layout(std430, binding = 4) buffer InstancedDataBuffer {
    InstancedData instancedData[];
}; 

out vec3 pos;
out vec3 normal;
out vec2 uv;
out vec3 fragPos;
out vec3 viewPos;
out vec3 color;

out vec4 fragPosLightSpace;

uniform mat4 model;

void main() {
	InstancedData instance = instancedData[gl_InstanceID];

	vec3 tempPos = Pos + instance.position.xyz;

	fragPos = vec3(model * vec4(tempPos, 1.0));
	normal = mat3(transpose(inverse(model))) * Normal;
	uv = UV;
	pos = tempPos;
	color = instance.color.xyz;

	viewPos = cameraPos;

	fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

	gl_Position = projection * view * model * vec4(tempPos, 1.0);
}
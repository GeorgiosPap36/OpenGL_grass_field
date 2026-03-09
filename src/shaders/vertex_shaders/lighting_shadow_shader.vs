#version 430 core
layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;

layout (std140, binding = 3) uniform FrameUniforms {
    mat4 projection;
    mat4 view;

	mat4 lightSpaceMatrix;
	
	vec3 cameraPos;
};

out vec3 pos;
out vec3 normal;
out vec2 uv;
out vec3 fragPos;
out vec3 viewPos;

out vec4 fragPosLightSpace;

uniform mat4 model;

void main()
{
	fragPos = vec3(model * vec4(Pos, 1.0));
	normal = mat3(transpose(inverse(model))) * Normal;
	uv = UV;
	pos = Pos;

	viewPos = cameraPos;

	fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

	gl_Position = projection * view * model * vec4(Pos, 1.0);
}
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


layout(location = 6) uniform sampler2D heightMap;
layout(location = 7) uniform sampler2D normalMap;
uniform mat4 model;
uniform float terrainSize;

out vec3 pos;
out vec3 normal;
out vec2 uv;
out vec3 fragPos;
out vec3 viewPos;

out vec4 fragPosLightSpace;

void main() {
    vec4 worldPos = model * vec4(Pos, 1.0);

    uv = (worldPos.xz / terrainSize) + 0.5;

    float height = texture(heightMap, uv).r;
	
    worldPos.y += height;

    fragPos = worldPos.xyz;
    pos = worldPos.xyz;
    viewPos = cameraPos;
    fragPosLightSpace = lightSpaceMatrix * worldPos;

    vec3 n = texture(normalMap, uv).xyz;
    normal = mat3(transpose(inverse(model))) * n;

    gl_Position = projection * view * worldPos;
}
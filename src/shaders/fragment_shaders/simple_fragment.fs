#version 430 core 

in vec2 uv;
in vec3 normal;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

void main() {
    FragColor = texture(texture_diffuse1, vec2(uv.x , 1 - uv.y));
}
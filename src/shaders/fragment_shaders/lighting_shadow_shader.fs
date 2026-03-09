#version 430 core

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 pos;
in vec3 normal;
in vec2 uv;
in vec3 fragPos;

in vec3 viewPos;

in vec4 fragPosLightSpace;

uniform DirLight dirLight;

uniform sampler2D texture_diffuse11;
uniform sampler2D texture_ambient11;
uniform sampler2D texture_specular11;

uniform bool useTexture;

uniform vec3 color;

layout(location = 5) uniform sampler2D shadowMap;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 ambient, diffuse, specular;

    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0);
    
    if (useTexture) {
        ambient = light.ambient * vec3(texture(texture_ambient11, vec2(uv.x , 1 - uv.y)));
        diffuse = light.diffuse * diff * vec3(texture(texture_diffuse11, vec2(uv.x , 1 - uv.y)));
        specular = light.specular * spec * vec3(texture(texture_specular11, uv));
    } else {
        ambient = light.ambient * color;
        diffuse = light.diffuse * diff * color;
        specular = vec3(0.0);
    }
    
    return (ambient + diffuse + specular);
}

float directLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }    
    }
    shadow /= 9.0;

    return shadow;
}

void main() {    
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 dirLightColor = calcDirLight(dirLight, norm, viewDir);
    float shadow = directLightShadowCalculation(fragPosLightSpace, norm, normalize(-dirLight.direction));

    vec3 result = (1.0 - shadow) * dirLightColor;

    FragColor = vec4(result, 1.0);
}
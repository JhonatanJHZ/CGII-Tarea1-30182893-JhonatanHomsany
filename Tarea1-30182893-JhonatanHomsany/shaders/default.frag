#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace; 
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float ambientIntensity;
uniform float specularStrength;
uniform float shininess;
uniform int shadingMode;
uniform sampler2D shadowMap;
uniform int shadowMode;            
uniform bool showOnlyShadows;       
uniform float biasForShadowMapping; 
uniform bool useAdaptativeBias;
uniform bool usePCF;
uniform int pcfKernelRadius;
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;
    float currentDepth = proj.z;
    float bias = 0.0;
    if (useAdaptativeBias) {
        bias = max(biasForShadowMapping * 10.0 * (1.0 - dot(normal, lightDir)), biasForShadowMapping);
    } else {
        bias = biasForShadowMapping;
    }
    float shadow = 0.0;
    if (usePCF) {
        vec2 texel = 1.0 / textureSize(shadowMap, 0);
        float samplesCount = 0.0;
        for (int x = -pcfKernelRadius; x <= pcfKernelRadius; ++x) {
            for (int y = -pcfKernelRadius; y <= pcfKernelRadius; ++y) {
                float d = texture(shadowMap, proj.xy + vec2(x, y) * texel).r;
                shadow += (currentDepth - bias) > d ? 1.0 : 0.0;
                samplesCount += 1.0;
            }
        }
        shadow /= samplesCount; 
    } else {
        float d = texture(shadowMap, proj.xy).r;
        shadow = (currentDepth - bias) > d ? 1.0 : 0.0;
    }
    return shadow;
}
void main() {
    vec3 N;
    if (shadingMode == 0) {
        N = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    } else {
        N = normalize(Normal);
    }
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 ambient = ambientIntensity * lightColor * objectColor;
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * objectColor * lightIntensity;
    vec3 specular = vec3(0.0);
    if (diff > 0.0) {
        if (shadingMode == 2) {
            vec3 reflectDir = reflect(-lightDir, N);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            specular = specularStrength * spec * lightColor * lightIntensity;
        } else if (shadingMode == 3) {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
            specular = specularStrength * spec * lightColor * lightIntensity;
        }
    }
    float shadow = 0.0;
    if (shadowMode == 2) {
        if (dot(N, lightDir) <= 0.0) {
            shadow = 1.0;
        } else {
            shadow = ShadowCalculation(FragPosLightSpace, N, lightDir);
        }
    }
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    if (shadowMode == 2 && showOnlyShadows) {
        result = vec3(1.0 - shadow);
    }
    FragColor = vec4(result, 1.0);
}

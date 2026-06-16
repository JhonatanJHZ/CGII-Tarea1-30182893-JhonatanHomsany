#version 330 core
out vec4 FragColor;

in vec4 VertexColor;
in vec3 Normal;
in vec3 FragPos;
in vec3 LocalPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace; 

uniform vec3 objectColor;
uniform bool useVertexColor;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float ambientIntensity;
uniform float specularStrength;
uniform float shininess;
uniform int shadingMode;
uniform float exposure;

uniform sampler2D shadowMap;
uniform int shadowMode;            
uniform bool showOnlyShadows;       
uniform float biasForShadowMapping; 
uniform bool useAdaptativeBias;
uniform bool usePCF;
uniform int pcfKernelRadius;

uniform int textureType; 
uniform bool hasNormalMap;
uniform bool hasBumpMap;
uniform bool hasAlbedoMap;
uniform sampler2D normalMap;
uniform sampler2D bumpMap;
uniform sampler2D albedoMap;

uniform bool hasMetallicMap;
uniform sampler2D metallicMap;
uniform float metallicValue;

uniform bool hasRoughnessMap;
uniform sampler2D roughnessMap;
uniform float roughnessValue;

uniform bool hasAoMap;
uniform sampler2D aoMap;
uniform float aoValue;

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

const float PI = 3.14159265359;

vec2 calculateUV(vec3 localPos, int type) {
    vec2 uv = TexCoord; 
    vec3 p = normalize(localPos);
    
    if (type == 1) {
        vec3 absPos = abs(localPos);
        vec2 uvFace;
        vec2 offset;

        if (absPos.x >= absPos.y && absPos.x >= absPos.z) {
            if (localPos.x > 0.0) {
                uvFace = vec2(-localPos.z, localPos.y) / (absPos.x * 2.0) + 0.5;
                offset = vec2(3.0, 1.0);
            } else {
                uvFace = vec2(localPos.z, localPos.y) / (absPos.x * 2.0) + 0.5;
                offset = vec2(1.0, 1.0);
            }
        } else if (absPos.y >= absPos.x && absPos.y >= absPos.z) {
            if (localPos.y > 0.0) {
                uvFace = vec2(localPos.x, -localPos.z) / (absPos.y * 2.0) + 0.5;
                offset = vec2(1.0, 2.0);
            } else {
                uvFace = vec2(localPos.x, localPos.z) / (absPos.y * 2.0) + 0.5;
                offset = vec2(1.0, 0.0);
            }
        } else {
            if (localPos.z > 0.0) {
                uvFace = vec2(localPos.x, localPos.y) / (absPos.z * 2.0) + 0.5;
                offset = vec2(2.0, 1.0);
            } else {
                uvFace = vec2(-localPos.x, localPos.y) / (absPos.z * 2.0) + 0.5;
                offset = vec2(0.0, 1.0);
            }
        }
        uvFace = clamp(uvFace, 0.005, 0.995); 
        uv = (uvFace + offset) * vec2(0.25, 1.0 / 3.0);
    } else if (type == 2) {
        uv.x = atan(p.z, p.x) / (2.0 * PI) + 0.5;
        uv.y = asin(p.y) / PI + 0.5;
        uv *= 6.0; 
    } else if (type == 3) {
        uv.x = atan(p.z, p.x) / (2.0 * PI) + 0.5;
        uv.y = localPos.y + 0.5;
        uv *= 6.0; 
    }
    return uv;
}

mat3 calculateTBN(vec3 normal, vec3 pos, vec2 uv) {
    vec3 Q1 = dFdx(pos);
    vec3 Q2 = dFdy(pos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(-Q1 * st2.s + Q2 * st1.s);
    return mat3(T, B, N);
}


vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N;
    if (shadingMode == 0) {
        N = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    } else {
        N = normalize(Normal);
    }
    
    vec2 uv = TexCoord;
    if (textureType != 0) {
        uv = calculateUV(LocalPos, textureType);
    }
    
    if (hasNormalMap) {
        mat3 TBN = calculateTBN(N, FragPos, uv);
        vec3 normalFromMap = texture(normalMap, uv).rgb;
        normalFromMap = normalFromMap * 2.0 - 1.0;
        N = normalize(TBN * normalFromMap);
    } else if (hasBumpMap) {
        float texelSize = 1.0 / textureSize(bumpMap, 0).x;
        float hL = texture(bumpMap, uv + vec2(-texelSize, 0.0)).r;
        float hR = texture(bumpMap, uv + vec2(texelSize, 0.0)).r;
        float hD = texture(bumpMap, uv + vec2(0.0, -texelSize)).r;
        float hU = texture(bumpMap, uv + vec2(0.0, texelSize)).r;
        vec3 bumpNormal = normalize(vec3(hL - hR, hD - hU, 0.05));
        
        mat3 TBN = calculateTBN(N, FragPos, uv);
        N = normalize(TBN * bumpNormal);
    }

    vec3 finalObjectColor = objectColor;
    if (useVertexColor) {
        finalObjectColor *= VertexColor.rgb;
    }
    if (hasAlbedoMap) {
        finalObjectColor *= texture(albedoMap, uv).rgb;
    }

    vec3 albedo = pow(finalObjectColor, vec3(2.2));
    float metallic = metallicValue;
    if (hasMetallicMap) metallic = texture(metallicMap, uv).r;
    float roughness = roughnessValue;
    if (hasRoughnessMap) roughness = texture(roughnessMap, uv).r;
    float ao = aoValue;
    if (hasAoMap) ao = texture(aoMap, uv).r;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 halfwayDir = normalize(viewDir + lightDir);
    
    float NdotL = max(dot(N, lightDir), 0.0);
    float NdotV = max(dot(N, viewDir), 0.0);
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    vec3 diffuse = kD * albedo / PI;

    vec3 specular = vec3(0.0);
    if (shadingMode == 1) {
        specular = vec3(0.0);
    } else if (shadingMode == 2) {
        vec3 reflectDir = reflect(-lightDir, N);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular = F * (spec * specularStrength) / max(NdotL, 0.0001);
    } else if (shadingMode == 3) {
        float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
        specular = F * (spec * specularStrength) / max(NdotL, 0.0001);
    } else {
        float pbrShininess = mix(256.0, 2.0, roughness);
        float spec = pow(max(dot(N, halfwayDir), 0.0), pbrShininess);
        
        float energyConservation = (8.0 + pbrShininess) / (8.0 * PI);
        
        specular = F * spec * energyConservation * (1.0 - roughness);
    }
    
    vec3 Lo = (diffuse + specular) * lightColor * lightIntensity * NdotL;
    vec3 ambient = ambientIntensity * lightColor * albedo * ao;
    
    float shadow = 0.0;
    if (shadowMode == 2) {
        if (dot(N, lightDir) <= 0.0) shadow = 1.0;
        else shadow = ShadowCalculation(FragPosLightSpace, N, lightDir);
    }
    
    vec3 color = ambient + (1.0 - shadow) * Lo;
    if (shadowMode == 2 && showOnlyShadows) {
        color = vec3(1.0 - shadow);
    } else {
        color *= exposure;
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0/2.2)); 
    }

    FragColor = vec4(color, 1.0);
}

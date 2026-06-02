#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace; // [NUEVO]

uniform vec3 objectColor;
uniform vec3 viewPos;

// Propiedades de la luz
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float ambientIntensity;
uniform float specularStrength;
uniform float shininess;

// 0: Flat, 1: Lambert, 2: Phong, 3: Blinn-Phong
uniform int shadingMode;

uniform sampler2D shadowMap;
uniform int shadowMode;            
uniform bool showOnlyShadows;       
uniform float biasForShadowMapping; 
uniform bool useAdaptativeBias;
uniform bool usePCF;
uniform int pcfKernelRadius;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // 1. División perspectiva
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 2. NDC [-1,1] -> [0,1]
    proj = proj * 0.5 + 0.5;
    // 3. Fuera del frustum de la luz: no hay sombra
    if (proj.z > 1.0) return 0.0;
    // 5. Profundidad real del fragmento
    float currentDepth = proj.z;
    
    // 6. Configurar bias
    float bias = 0.0;
    if (useAdaptativeBias) {
        bias = max(biasForShadowMapping * 10.0 * (1.0 - dot(normal, lightDir)), biasForShadowMapping);
    } else {
        bias = biasForShadowMapping;
    }
    
    // 7. Filtro PCF o Muestreo Básico [NUEVO]
    float shadow = 0.0;
    if (usePCF) {
        vec2 texel = 1.0 / textureSize(shadowMap, 0);
        float samplesCount = 0.0;
        
        // Bucle con radio dinámico enviado desde ImGui
        for (int x = -pcfKernelRadius; x <= pcfKernelRadius; ++x) {
            for (int y = -pcfKernelRadius; y <= pcfKernelRadius; ++y) {
                float d = texture(shadowMap, proj.xy + vec2(x, y) * texel).r;
                shadow += (currentDepth - bias) > d ? 1.0 : 0.0;
                samplesCount += 1.0;
            }
        }
        shadow /= samplesCount; // Promedio de muestras del kernel
    } else {
        // Muestreo básico simple de 1 sola muestra (bordes pixelados duros)
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

    // 1. Iluminación Ambiental
    vec3 ambient = ambientIntensity * lightColor * objectColor;

    // 2. Iluminación Difusa
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * objectColor * lightIntensity;

    // 3. Iluminación Especular
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

    // 4. Calcular el factor de sombra si está activado
    float shadow = 0.0;
    if (shadowMode == 2) {
        // Si el fragmento apunta en dirección opuesta a la luz, está físicamente en sombra total
        if (dot(N, lightDir) <= 0.0) {
            shadow = 1.0;
        } else {
            shadow = ShadowCalculation(FragPosLightSpace, N, lightDir);
        }
    }

    // El factor (1.0 - shadow) afecta a difusa y especular. La luz ambiental queda intacta.
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    // Si está activada la visualización "Solo Sombras", mostramos la sombra en Blanco y Negro (B&W)
    if (shadowMode == 2 && showOnlyShadows) {
        result = vec3(1.0 - shadow);
    }

    FragColor = vec4(result, 1.0);
}

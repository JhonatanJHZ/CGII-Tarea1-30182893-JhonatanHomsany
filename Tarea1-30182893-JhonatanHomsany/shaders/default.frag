#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

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

void main() {
    vec3 N;
    if (shadingMode == 0) {
        // Flat shading: Calculamos la normal por cara con derivadas en pantalla
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
    if (shadingMode == 2) {
        // Phong: reflexión perfecta
        vec3 reflectDir = reflect(-lightDir, N);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular = specularStrength * spec * lightColor * lightIntensity;
    } else if (shadingMode == 3) {
        // Blinn-Phong: vector medio
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
        specular = specularStrength * spec * lightColor * lightIntensity;
    }

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
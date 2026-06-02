#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace; // [NUEVO] Enviará la coordenada proyectada a la luz

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // [NUEVO] Matriz vista-proyección de la luz

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); // [NUEVO] Proyecto el fragmento en la luz
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

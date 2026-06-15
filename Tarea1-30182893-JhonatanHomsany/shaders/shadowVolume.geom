#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightPos;

in vec3 WorldPos[];

const float EPSILON = 0.01;
const float INFINITY = 100.0;
void EmitProjectedVertex(vec3 pos) {
    gl_Position = projection * view * vec4(pos, 1.0);
    EmitVertex();
}

void main()
{
    vec3 v0 = WorldPos[0];
    vec3 v1 = WorldPos[1];
    vec3 v2 = WorldPos[2];

    vec3 normal = normalize(cross(v1 - v0, v2 - v0));
    vec3 lightDir = normalize(((v0 + v1 + v2) / 3.0) - lightPos);
    
    if (dot(normal, lightDir) > 0.0) {
        
        vec3 p0 = v0 + normalize(v0 - lightPos) * INFINITY;
        vec3 p1 = v1 + normalize(v1 - lightPos) * INFINITY;
        vec3 p2 = v2 + normalize(v2 - lightPos) * INFINITY;

        vec3 f0 = v0 + normalize(v0 - lightPos) * EPSILON;
        vec3 f1 = v1 + normalize(v1 - lightPos) * EPSILON;
        vec3 f2 = v2 + normalize(v2 - lightPos) * EPSILON;
        EmitProjectedVertex(p0);
        EmitProjectedVertex(p1);
        EmitProjectedVertex(p2);
        EndPrimitive();
        EmitProjectedVertex(f2);
        EmitProjectedVertex(f1);
        EmitProjectedVertex(f0);
        EndPrimitive();
        
        EmitProjectedVertex(p0);
        EmitProjectedVertex(f0);
        EmitProjectedVertex(p1);
        EmitProjectedVertex(f1);
        EndPrimitive();
        
        EmitProjectedVertex(p1);
        EmitProjectedVertex(f1);
        EmitProjectedVertex(p2);
        EmitProjectedVertex(f2);
        EndPrimitive();
        
        EmitProjectedVertex(p2);
        EmitProjectedVertex(f2);
        EmitProjectedVertex(p0);
        EmitProjectedVertex(f0);
        EndPrimitive();
    }
}

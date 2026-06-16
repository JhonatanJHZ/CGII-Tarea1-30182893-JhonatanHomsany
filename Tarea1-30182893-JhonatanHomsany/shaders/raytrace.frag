#version 430 core
out vec4 FragColor;
in vec2 TexCoords;
uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camUp;
uniform vec3 camRight;
uniform float fov;
uniform float aspect;
uniform float exposure;
uniform int shadingMode;
uniform float shininess;
uniform float specularStrength;
uniform sampler2D globalBumpMap;
struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
};
struct Plane {
    mat4 invModel;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
};
struct Triangle {
    vec4 v0;
    vec4 v1;
    vec4 v2;
    vec4 normal;
    vec4 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
    float pad0;
    float pad1;
    float pad2;
    vec4 local_v0;
    vec4 local_v1;
    vec4 local_v2;
    vec2 uv0;
    vec2 uv1;
    vec2 uv2;
    vec2 pad3;
};
struct Cylinder {
    mat4 invModel;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
};
struct Box {
    mat4 invModel;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
};
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float ambientIntensity;
};
#define MAX_SPHERES 50
uniform int numSpheres;
uniform Sphere spheres[MAX_SPHERES];
#define MAX_PLANES 10
uniform int numPlanes;
uniform Plane planes[MAX_PLANES];
layout(std430, binding = 0) buffer TriangleBuffer {
    Triangle triangles[];
};
uniform int numTriangles;

struct MeshGroup {
    vec3 minAABB;
    int startIndex;
    vec3 maxAABB;
    int count;
};
#define MAX_MESH_GROUPS 10
uniform int numMeshGroups;
uniform MeshGroup meshGroups[MAX_MESH_GROUPS];
#define MAX_CYLINDERS 10
uniform int numCylinders;
uniform Cylinder cylinders[MAX_CYLINDERS];
#define MAX_BOXES 10
uniform int numBoxes;
uniform Box boxes[MAX_BOXES];
#define MAX_LIGHTS 10
uniform int numLights;
uniform Light lights[MAX_LIGHTS];
#define MAX_ALLOWED_BOUNCES 10
uniform int maxBounces;
struct Ray {
    vec3 origin;
    vec3 dir;
};
struct HitRecord {
    bool hit;
    float t;
    vec3 point;
    vec3 normal;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    vec3 localPos;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
    vec2 uv;
};

uniform sampler2D albedo1;
uniform sampler2D albedo2;
uniform sampler2D albedo3;
uniform sampler2D albedo4;
uniform sampler2D albedo5;

vec3 getAlbedoColor(int id, vec2 uv) {
    if (id == 1) return texture(albedo1, uv).rgb;
    if (id == 2) return texture(albedo2, uv).rgb;
    if (id == 3) return texture(albedo3, uv).rgb;
    if (id == 4) return texture(albedo4, uv).rgb;
    if (id == 5) return texture(albedo5, uv).rgb;
    return vec3(1.0);
}

vec2 getUV(vec3 localPos, int textureType) {
    vec2 uv = vec2(0.0);
    if (textureType == 1) { 
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
    } else if (textureType == 2) { 
        vec3 p = normalize(localPos);
        uv.x = atan(p.z, p.x) / (2.0 * 3.14159265) + 0.5;
        uv.y = asin(p.y) / 3.14159265 + 0.5;
    } else if (textureType == 3) {
        vec3 p = localPos;
        uv.x = atan(p.z, p.x) / (2.0 * 3.14159265) + 0.5;
        uv.y = p.y + 0.5;
    } else if (textureType == 4) {
        uv = localPos.xz + 0.5;
    } else if (textureType == 5) {
        uv = localPos.xy;
    }
    return uv;
}
float hitSphere(Ray ray, Sphere sphere) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.dir, ray.dir);
    float b = 2.0 * dot(oc, ray.dir);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return -1.0;
    float t = (-b - sqrt(discriminant)) / (2.0 * a);
    return (t > 0.001) ? t : -1.0;
}

float hitCylinder(Ray ray, Cylinder cyl, out vec3 normalOut) {
    vec3 O = (cyl.invModel * vec4(ray.origin, 1.0)).xyz;
    vec3 D = normalize((cyl.invModel * vec4(ray.dir, 0.0)).xyz);
    float rayScale = length((cyl.invModel * vec4(ray.dir, 0.0)).xyz);
    
    float a = D.x * D.x + D.z * D.z;
    float b = 2.0 * (O.x * D.x + O.z * D.z);
    float c = O.x * O.x + O.z * O.z - 0.25; 
    
    float tHit = 99999.0;
    vec3 hitNormal = vec3(0.0);
    
    if (abs(a) > 1e-6) {
        float discriminant = b * b - 4.0 * a * c;
        if (discriminant >= 0.0) {
            float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
            float y1 = O.y + t1 * D.y;
            if (t1 > 0.001 && y1 >= -0.5 && y1 <= 0.5) {
                tHit = t1;
                hitNormal = vec3(O.x + t1 * D.x, 0.0, O.z + t1 * D.z);
            } else {
                float t2 = (-b + sqrt(discriminant)) / (2.0 * a);
                float y2 = O.y + t2 * D.y;
                if (t2 > 0.001 && t2 < tHit && y2 >= -0.5 && y2 <= 0.5) {
                    tHit = t2;
                    hitNormal = vec3(O.x + t2 * D.x, 0.0, O.z + t2 * D.z);
                }
            }
        }
    }
    
    if (abs(D.y) > 1e-6) {
        float tTop = (0.5 - O.y) / D.y;
        vec3 pTop = O + tTop * D;
        if (tTop > 0.001 && tTop < tHit && (pTop.x * pTop.x + pTop.z * pTop.z) <= 0.25) {
            tHit = tTop;
            hitNormal = vec3(0.0, 1.0, 0.0);
        }
        
        float tBottom = (-0.5 - O.y) / D.y;
        vec3 pBottom = O + tBottom * D;
        if (tBottom > 0.001 && tBottom < tHit && (pBottom.x * pBottom.x + pBottom.z * pBottom.z) <= 0.25) {
            tHit = tBottom;
            hitNormal = vec3(0.0, -1.0, 0.0);
        }
    }
    
    if (tHit < 99999.0) {
        mat4 normalMatrix = transpose(inverse(inverse(cyl.invModel)));
        normalOut = normalize((normalMatrix * vec4(hitNormal, 0.0)).xyz);
        return tHit / rayScale;
    }
    return -1.0;
}

float hitBox(Ray ray, Box box, out vec3 normalOut) {
    vec3 O = (box.invModel * vec4(ray.origin, 1.0)).xyz;
    vec3 D = normalize((box.invModel * vec4(ray.dir, 0.0)).xyz);
    float rayScale = length((box.invModel * vec4(ray.dir, 0.0)).xyz);
    
    vec3 invD = 1.0 / D;
    vec3 t0 = (-0.5 - O) * invD;
    vec3 t1 = (0.5 - O) * invD;
    
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    float tnear = max(max(tmin.x, tmin.y), tmin.z);
    float tfar = min(min(tmax.x, tmax.y), tmax.z);
    
    if (tnear > tfar || tfar < 0.001) return -1.0;
    
    float tHit = tnear > 0.001 ? tnear : tfar;
    
    vec3 p = O + tHit * D;
    vec3 hitNormal = vec3(0.0);
    if (abs(p.x - 0.5) < 1e-4) hitNormal = vec3(1.0, 0.0, 0.0);
    else if (abs(p.x + 0.5) < 1e-4) hitNormal = vec3(-1.0, 0.0, 0.0);
    else if (abs(p.y - 0.5) < 1e-4) hitNormal = vec3(0.0, 1.0, 0.0);
    else if (abs(p.y + 0.5) < 1e-4) hitNormal = vec3(0.0, -1.0, 0.0);
    else if (abs(p.z - 0.5) < 1e-4) hitNormal = vec3(0.0, 0.0, 1.0);
    else hitNormal = vec3(0.0, 0.0, -1.0);
    
    mat4 normalMatrix = transpose(inverse(inverse(box.invModel)));
    normalOut = normalize((normalMatrix * vec4(hitNormal, 0.0)).xyz);
    return tHit / rayScale;
}
float hitPlane(Ray ray, Plane plane, out vec3 normalOut) {
    vec3 O = (plane.invModel * vec4(ray.origin, 1.0)).xyz;
    vec3 D = normalize((plane.invModel * vec4(ray.dir, 0.0)).xyz);
    float rayScale = length((plane.invModel * vec4(ray.dir, 0.0)).xyz);
    
    if (abs(D.z) > 1e-6) {
        float t = -O.z / D.z;
        if (t > 0.001) {
            vec3 p = O + t * D;
            if (abs(p.x) <= 0.5 && abs(p.y) <= 0.5) {
                vec3 localNormal = vec3(0.0, 0.0, 1.0);
                mat4 normalMatrix = transpose(inverse(inverse(plane.invModel)));
                vec3 worldNormal = normalize((normalMatrix * vec4(localNormal, 0.0)).xyz);
                if (dot(ray.dir, worldNormal) > 0.0) worldNormal = -worldNormal;
                normalOut = worldNormal;
                return t / rayScale;
            }
        }
    }
    return -1.0;
}
bool hitAABB(Ray ray, vec3 boxMin, vec3 boxMax) {
    vec3 invDir = 1.0 / ray.dir;
    vec3 t0 = (boxMin - ray.origin) * invDir;
    vec3 t1 = (boxMax - ray.origin) * invDir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);
    return tNear <= tFar && tFar > 0.0;
}

float hitTriangle(Ray ray, Triangle tri, out vec3 normal, out vec3 hitPoint, out vec3 localPos, out vec2 hitUV) {
    vec3 v0 = tri.v0.xyz;
    vec3 v1 = tri.v1.xyz;
    vec3 v2 = tri.v2.xyz;
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 h = cross(ray.dir, e2);
    float a = dot(e1, h);
    if (a > -0.00001 && a < 0.00001) return -1.0;
    float f = 1.0 / a;
    vec3 s = ray.origin - v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return -1.0;
    vec3 q = cross(s, e1);
    float v = f * dot(ray.dir, q);
    if (v < 0.0 || u + v > 1.0) return -1.0;
    float t = f * dot(e2, q);
    if (t > 0.001) {
        normal = normalize(tri.normal.xyz);
        hitPoint = ray.origin + ray.dir * t;
        float w = 1.0 - u - v;
        localPos = (tri.local_v0.xyz * w) + (tri.local_v1.xyz * u) + (tri.local_v2.xyz * v);
        hitUV = (tri.uv0 * w) + (tri.uv1 * u) + (tri.uv2 * v);
        return t;
    }
    return -1.0;
}
HitRecord findClosestHit(Ray ray) {
    HitRecord rec;
    rec.hit = false;
    rec.t = 99999.0;
    int safeSpheres = min(numSpheres, MAX_SPHERES);
    for (int i = 0; i < safeSpheres; i++) {
        float t = hitSphere(ray, spheres[i]);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = normalize(rec.point - spheres[i].center);
            rec.color = spheres[i].color;
            rec.reflectivity = spheres[i].reflectivity;
            rec.transparency = spheres[i].transparency;
            rec.refractiveIndex = spheres[i].refractiveIndex;
            rec.metallic = spheres[i].metallic;
            rec.roughness = spheres[i].roughness;
            rec.ao = spheres[i].ao;
            rec.localPos = rec.point - spheres[i].center;
            rec.textureType = spheres[i].textureType;
            rec.hasBumpMap = spheres[i].hasBumpMap;
            rec.albedoMapID = spheres[i].albedoMapID;
        }
    }
    int safePlanes = min(numPlanes, MAX_PLANES);
    for (int i = 0; i < safePlanes; i++) {
        vec3 tNormal;
        float t = hitPlane(ray, planes[i], tNormal);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = tNormal;
            rec.color = planes[i].color;
            rec.reflectivity = planes[i].reflectivity;
            rec.transparency = planes[i].transparency;
            rec.refractiveIndex = planes[i].refractiveIndex;
            rec.metallic = planes[i].metallic;
            rec.roughness = planes[i].roughness;
            rec.ao = planes[i].ao;
            rec.localPos = (planes[i].invModel * vec4(rec.point, 1.0)).xyz;
            rec.textureType = planes[i].textureType;
            rec.hasBumpMap = planes[i].hasBumpMap;
            rec.albedoMapID = planes[i].albedoMapID;
        }
    }
    for (int g = 0; g < numMeshGroups; g++) {
        if (!hitAABB(ray, meshGroups[g].minAABB, meshGroups[g].maxAABB)) {
            continue;
        }
        for (int i = meshGroups[g].startIndex; i < meshGroups[g].startIndex + meshGroups[g].count; i++) {
            vec3 normal, hitPoint, localPos;
            vec2 hitUV;
            float t = hitTriangle(ray, triangles[i], normal, hitPoint, localPos, hitUV);
            if (t > 0.0 && t < rec.t) {
                rec.hit = true;
                rec.t = t;
                rec.point = hitPoint;
                rec.normal = normal;
                rec.color = triangles[i].color.rgb;
                rec.reflectivity = triangles[i].reflectivity;
                rec.transparency = triangles[i].transparency;
                rec.refractiveIndex = triangles[i].refractiveIndex;
                rec.metallic = triangles[i].metallic;
                rec.roughness = triangles[i].roughness;
                rec.ao = triangles[i].ao;
                rec.localPos = localPos;
                rec.textureType = triangles[i].textureType;
                rec.hasBumpMap = triangles[i].hasBumpMap;
                rec.albedoMapID = triangles[i].albedoMapID;
                rec.uv = hitUV;
            }
        }
    }
    int safeCylinders = min(numCylinders, MAX_CYLINDERS);
    for (int i = 0; i < safeCylinders; i++) {
        vec3 tNormal;
        float t = hitCylinder(ray, cylinders[i], tNormal);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = tNormal;
            rec.color = cylinders[i].color;
            rec.reflectivity = cylinders[i].reflectivity;
            rec.transparency = cylinders[i].transparency;
            rec.refractiveIndex = cylinders[i].refractiveIndex;
            rec.metallic = cylinders[i].metallic;
            rec.roughness = cylinders[i].roughness;
            rec.ao = cylinders[i].ao;
            rec.localPos = (cylinders[i].invModel * vec4(rec.point, 1.0)).xyz;
            rec.textureType = cylinders[i].textureType;
            rec.hasBumpMap = cylinders[i].hasBumpMap;
            rec.albedoMapID = cylinders[i].albedoMapID;
        }
    }
    int safeBoxes = min(numBoxes, MAX_BOXES);
    for (int i = 0; i < safeBoxes; i++) {
        vec3 tNormal;
        float t = hitBox(ray, boxes[i], tNormal);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = tNormal;
            rec.color = boxes[i].color;
            rec.reflectivity = boxes[i].reflectivity;
            rec.transparency = boxes[i].transparency;
            rec.refractiveIndex = boxes[i].refractiveIndex;
            rec.metallic = boxes[i].metallic;
            rec.roughness = boxes[i].roughness;
            rec.ao = boxes[i].ao;
            rec.localPos = (boxes[i].invModel * vec4(rec.point, 1.0)).xyz;
            rec.textureType = boxes[i].textureType;
            rec.hasBumpMap = boxes[i].hasBumpMap;
            rec.albedoMapID = boxes[i].albedoMapID;
        }
    }
    return rec;
}

bool inShadowFast(Ray ray, float maxDist) {
    for (int i = 0; i < numSpheres; i++) {
        float t = hitSphere(ray, spheres[i]);
        if (t > 0.0 && t < maxDist) return true;
    }
    for (int i = 0; i < numPlanes; i++) {
        vec3 tNormal;
        float t = hitPlane(ray, planes[i], tNormal);
        if (t > 0.0 && t < maxDist) return true;
    }
    for (int g = 0; g < numMeshGroups; g++) {
        if (!hitAABB(ray, meshGroups[g].minAABB, meshGroups[g].maxAABB)) {
            continue;
        }
        for (int i = meshGroups[g].startIndex; i < meshGroups[g].startIndex + meshGroups[g].count; i++) {
            vec3 normal, hitPoint, localPos;
            vec2 hitUV;
            float t = hitTriangle(ray, triangles[i], normal, hitPoint, localPos, hitUV);
            if (t > 0.001 && t < maxDist) return true;
        }
    }
    for (int i = 0; i < numCylinders; i++) {
        vec3 tNormal;
        float t = hitCylinder(ray, cylinders[i], tNormal);
        if (t > 0.0 && t < maxDist) return true;
    }
    for (int i = 0; i < numBoxes; i++) {
        vec3 tNormal;
        float t = hitBox(ray, boxes[i], tNormal);
        if (t > 0.0 && t < maxDist) return true;
    }
    return false;
}


vec3 fresnelSchlickPBR(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculateDirectLight(Ray ray, HitRecord rec) {
    vec3 finalColor = vec3(0.0);
    vec3 viewDir = normalize(-ray.dir);
    
    vec3 albedo = pow(rec.color, vec3(2.2)); 
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, rec.metallic);
    
    for (int j = 0; j < numLights; j++) {
        vec3 lightDir = normalize(lights[j].position - rec.point);
        float distToLight = length(lights[j].position - rec.point);
        Ray shadowRay;
        shadowRay.origin = rec.point + rec.normal * 0.05;
        shadowRay.dir = lightDir;
        bool inShadow = inShadowFast(shadowRay, distToLight);
        
        vec3 ambient = lights[j].color * lights[j].ambientIntensity * albedo * rec.ao;
        finalColor += ambient;
        
        if (!inShadow) {
            vec3 halfwayDir = normalize(viewDir + lightDir);
            float NdotL = max(dot(rec.normal, lightDir), 0.0);
            float NdotV = max(dot(rec.normal, viewDir), 0.0);
            
            vec3 F = fresnelSchlickPBR(max(dot(halfwayDir, viewDir), 0.0), F0);
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - rec.metallic;
            vec3 diffuse = kD * albedo / 3.14159265;
            
            vec3 specular = vec3(0.0);
            if (shadingMode == 1) {
                specular = vec3(0.0);
            } else if (shadingMode == 2) {
                vec3 reflectDir = reflect(-lightDir, rec.normal);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
                specular = F * (spec * specularStrength) / max(NdotL, 0.0001);
            } else if (shadingMode == 3) {
                float spec = pow(max(dot(rec.normal, halfwayDir), 0.0), shininess);
                specular = F * (spec * specularStrength) / max(NdotL, 0.0001);
            } else {
                float pbrShininess = mix(256.0, 2.0, rec.roughness);
                float spec = pow(max(dot(rec.normal, halfwayDir), 0.0), pbrShininess);
                float energyConservation = (8.0 + pbrShininess) / (8.0 * 3.14159265);
                specular = F * spec * energyConservation * (1.0 - rec.roughness);
            }
            
            finalColor += (diffuse + specular) * lights[j].color * lights[j].intensity * NdotL;
        }
    }
    return finalColor;
}
float fresnelSchlick(float cosTheta, float ior) {
    float r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
void main() {
    vec2 ndc = TexCoords * 2.0 - 1.0;
    float tanHalfFov = tan(radians(fov) / 2.0);
    vec3 rayDir = normalize(camFront + camRight * ndc.x * aspect * tanHalfFov + camUp * ndc.y * tanHalfFov);
    Ray currentRay;
    currentRay.origin = camPos;
    currentRay.dir = rayDir;
    vec3 finalColor = vec3(0.0);
    vec3 throughput = vec3(1.0); 
    for (int bounce = 0; bounce < MAX_ALLOWED_BOUNCES; bounce++) {
        if (bounce >= maxBounces) break;
        HitRecord rec = findClosestHit(currentRay);
        if (!rec.hit) {
            vec3 skyColor = vec3(0.0, 0.0, 0.0);
            finalColor += throughput * skyColor;
            break;
        }

        if (rec.textureType != 0) {
            vec2 uv = (rec.textureType == 5) ? rec.uv : getUV(rec.localPos, rec.textureType);
            vec3 albedo = getAlbedoColor(rec.albedoMapID, uv);
            rec.color = albedo;
        }

        if (rec.hasBumpMap == 1 && rec.textureType != 0) {
            vec3 p = normalize(rec.localPos);
            vec2 uv;
            uv.x = atan(p.z, p.x) / (2.0 * 3.14159265) + 0.5;
            uv.y = asin(p.y) / 3.14159265 + 0.5;
            uv *= 6.0; 

            float texelSize = 1.0 / textureSize(globalBumpMap, 0).x;
            float hL = texture(globalBumpMap, uv + vec2(-texelSize, 0.0)).r;
            float hR = texture(globalBumpMap, uv + vec2(texelSize, 0.0)).r;
            float hD = texture(globalBumpMap, uv + vec2(0.0, -texelSize)).r;
            float hU = texture(globalBumpMap, uv + vec2(0.0, texelSize)).r;
            vec3 bumpNormal = normalize(vec3(hL - hR, hD - hU, 0.05));

            vec3 T = normalize(vec3(-p.z, 0.0, p.x));
            if (abs(p.y) > 0.999) T = vec3(1.0, 0.0, 0.0);
            vec3 B = normalize(cross(rec.normal, T));
            mat3 TBN = mat3(T, B, rec.normal);
            
            rec.normal = normalize(TBN * bumpNormal);
        }

        vec3 directLight = calculateDirectLight(currentRay, rec);
        finalColor += throughput * directLight;
        if (rec.transparency > 0.0) {
            float ior = rec.refractiveIndex;
            float eta = 1.0 / ior;
            vec3 refractNormal = rec.normal;
            float cosTheta = min(dot(-currentRay.dir, rec.normal), 1.0);
            if (dot(currentRay.dir, rec.normal) > 0.0) {
                refractNormal = -rec.normal;
                eta = ior;
                cosTheta = min(dot(currentRay.dir, rec.normal), 1.0);
            }
            float F = fresnelSchlick(cosTheta, ior); 
            vec3 refractedDir = refract(currentRay.dir, refractNormal, eta);
            if (length(refractedDir) < 0.01) {
                currentRay.dir = normalize(reflect(currentRay.dir, refractNormal));
                currentRay.origin = rec.point + refractNormal * 0.05;
                throughput *= rec.transparency; 
            } else {
                currentRay.dir = normalize(refractedDir);
                currentRay.origin = rec.point - refractNormal * 0.05;
                throughput *= rec.transparency * (1.0 - F);
            }
        } else if (rec.metallic > 0.0 || rec.roughness < 1.0) {
            vec3 reflectDir = normalize(reflect(currentRay.dir, rec.normal));
            vec3 jitter = vec3(
                fract(sin(dot(rec.point.xy, vec2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0,
                fract(sin(dot(rec.point.yz, vec2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0,
                fract(sin(dot(rec.point.zx, vec2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0
            );
            reflectDir = normalize(reflectDir + jitter * rec.roughness * 0.2);
            if (dot(reflectDir, rec.normal) < 0.0) reflectDir = reflectDir - 2.0 * dot(reflectDir, rec.normal) * rec.normal;
            
            vec3 albedo = pow(rec.color, vec3(2.2));
            vec3 F0 = mix(vec3(0.04), albedo, rec.metallic);
            vec3 F = fresnelSchlickPBR(max(dot(rec.normal, normalize(-currentRay.dir)), 0.0), F0);
            
            currentRay.dir = reflectDir;
            currentRay.origin = rec.point + rec.normal * 0.05;
            throughput *= F;
        } else {
            break;
        }
    }
    
    finalColor *= max(exposure, 1.0);
    
    if(isnan(finalColor.x) || isnan(finalColor.y) || isnan(finalColor.z)) {
        finalColor = vec3(1.0, 0.0, 0.0);
    }
    
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(finalColor, 1.0);
}

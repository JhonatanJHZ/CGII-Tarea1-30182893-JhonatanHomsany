#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camUp;
uniform vec3 camRight;
uniform float fov;
uniform float aspect;
uniform sampler2D globalBumpMap;
struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
};
struct Plane {
    vec3 point;
    vec3 normal;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
};
struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 normal;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
    vec3 local_v0;
    vec3 local_v1;
    vec3 local_v2;
};
struct Cylinder {
    mat4 invModel;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
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
#define MAX_TRIANGLES 200
uniform int numTriangles;
uniform Triangle triangles[MAX_TRIANGLES];
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
    vec3 localPos;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
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
        uv *= 6.0;
    } else if (textureType == 3) { 
        vec3 p = normalize(vec3(localPos.x, 0.0, localPos.z));
        uv.x = atan(p.z, p.x) / (2.0 * 3.14159265) + 0.5;
        uv.y = localPos.y + 0.5;
        uv *= 6.0;
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
float hitPlane(Ray ray, Plane plane) {
    float denom = dot(plane.normal, ray.dir);
    if (abs(denom) > 1e-6) {
        float t = dot(plane.point - ray.origin, plane.normal) / denom;
        if (t > 0.001) return t;
    }
    return -1.0;
}
float hitTriangle(Ray ray, Triangle tri, out vec3 normalOut, out float uOut, out float vOut) {
    const float EPSILON = 1e-6;
    vec3 edge1 = tri.v1 - tri.v0;
    vec3 edge2 = tri.v2 - tri.v0;
    vec3 h = cross(ray.dir, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) return -1.0;
    if (tri.transparency == 0.0 && a < 0.0) return -1.0;
    float f = 1.0 / a;
    vec3 s = ray.origin - tri.v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return -1.0;
    vec3 q = cross(s, edge1);
    float v = f * dot(ray.dir, q);
    if (v < 0.0 || u + v > 1.0) return -1.0;
    float t = f * dot(edge2, q);
    if (t > 0.001) {
        normalOut = normalize(cross(edge1, edge2));
        if(dot(normalOut, ray.dir) > 0.0) normalOut = -normalOut;
        uOut = u;
        vOut = v;
        return t;
    }
    return -1.0;
}
HitRecord findClosestHit(Ray ray) {
    HitRecord rec;
    rec.hit = false;
    rec.t = 99999.0;
    for (int i = 0; i < numSpheres; i++) {
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
            rec.localPos = rec.point - spheres[i].center;
            rec.textureType = spheres[i].textureType;
            rec.hasBumpMap = spheres[i].hasBumpMap;
            rec.albedoMapID = spheres[i].albedoMapID;
        }
    }
    for (int i = 0; i < numPlanes; i++) {
        float t = hitPlane(ray, planes[i]);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = dot(ray.dir, planes[i].normal) < 0.0 ? planes[i].normal : -planes[i].normal;
            rec.color = planes[i].color;
            rec.reflectivity = planes[i].reflectivity;
            rec.transparency = planes[i].transparency;
            rec.refractiveIndex = planes[i].refractiveIndex;
            rec.localPos = vec3(0.0);
            rec.textureType = 0;
            rec.hasBumpMap = 0;
            rec.albedoMapID = 0;
        }
    }
    for (int i = 0; i < numTriangles; i++) {
        vec3 tNormal;
        float u, v;
        float t = hitTriangle(ray, triangles[i], tNormal, u, v);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = tNormal;
            rec.color = triangles[i].color;
            rec.reflectivity = triangles[i].reflectivity;
            rec.transparency = triangles[i].transparency;
            rec.refractiveIndex = triangles[i].refractiveIndex;
            float w = 1.0 - u - v;
            rec.localPos = w * triangles[i].local_v0 + u * triangles[i].local_v1 + v * triangles[i].local_v2;
            rec.textureType = triangles[i].textureType;
            rec.hasBumpMap = triangles[i].hasBumpMap;
            rec.albedoMapID = triangles[i].albedoMapID;
        }
    }
    for (int i = 0; i < numCylinders; i++) {
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
            rec.localPos = (cylinders[i].invModel * vec4(rec.point, 1.0)).xyz;
            rec.textureType = cylinders[i].textureType;
            rec.hasBumpMap = cylinders[i].hasBumpMap;
            rec.albedoMapID = cylinders[i].albedoMapID;
        }
    }
    for (int i = 0; i < numBoxes; i++) {
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
        float t = hitPlane(ray, planes[i]);
        if (t > 0.0 && t < maxDist) return true;
    }
    for (int i = 0; i < numTriangles; i++) {
        vec3 tNormal;
        float u, v;
        float t = hitTriangle(ray, triangles[i], tNormal, u, v);
        if (t > 0.0 && t < maxDist) return true;
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

vec3 calculateDirectLight(Ray ray, HitRecord rec) {
    vec3 finalColor = vec3(0.0);
    vec3 viewDir = normalize(-ray.dir);
    for (int j = 0; j < numLights; j++) {
        vec3 lightDir = normalize(lights[j].position - rec.point);
        float distToLight = length(lights[j].position - rec.point);
        Ray shadowRay;
        shadowRay.origin = rec.point + rec.normal * 0.05;
        shadowRay.dir = lightDir;
        bool inShadow = inShadowFast(shadowRay, distToLight);
        vec3 ambient = lights[j].color * lights[j].ambientIntensity * rec.color;
        finalColor += ambient;
        if (!inShadow) {
            float diff = max(dot(rec.normal, lightDir), 0.0);
            vec3 reflectedDir = reflect(-lightDir, rec.normal);
            float spec = pow(max(dot(reflectedDir, viewDir), 0.0), 32.0);
            finalColor += lights[j].color * diff * lights[j].intensity * rec.color;
            finalColor += lights[j].color * spec * lights[j].intensity;
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
            float tSky = 0.5 * (currentRay.dir.y + 1.0);
            vec3 skyColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), tSky);
            finalColor += throughput * skyColor;
            break;
        }

        if (rec.textureType != 0 && rec.albedoMapID != 0) {
            vec2 uv = getUV(rec.localPos, rec.textureType);
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
        } else if (rec.reflectivity > 0.0) {
            currentRay.dir = normalize(reflect(currentRay.dir, rec.normal));
            currentRay.origin = rec.point + rec.normal * 0.05;
            throughput *= rec.reflectivity;
        } else {
            break;
        }
    }
    FragColor = vec4(finalColor, 1.0);
}

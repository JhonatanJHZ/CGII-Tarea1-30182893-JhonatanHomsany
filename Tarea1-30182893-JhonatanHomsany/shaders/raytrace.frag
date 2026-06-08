#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camUp;
uniform vec3 camRight;
uniform float fov;
uniform float aspect;
struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
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
#define MAX_TRIANGLES 50
uniform int numTriangles;
uniform Triangle triangles[MAX_TRIANGLES];
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
};
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
float hitPlane(Ray ray, Plane plane) {
    float denom = dot(plane.normal, ray.dir);
    if (abs(denom) > 1e-6) {
        float t = dot(plane.point - ray.origin, plane.normal) / denom;
        if (t > 0.001) return t;
    }
    return -1.0;
}
float hitTriangle(Ray ray, Triangle tri, out vec3 normalOut) {
    const float EPSILON = 1e-6;
    vec3 edge1 = tri.v1 - tri.v0;
    vec3 edge2 = tri.v2 - tri.v0;
    vec3 h = cross(ray.dir, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) return -1.0;
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
        }
    }
    for (int i = 0; i < numTriangles; i++) {
        vec3 tNormal;
        float t = hitTriangle(ray, triangles[i], tNormal);
        if (t > 0.0 && t < rec.t) {
            rec.hit = true;
            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = tNormal;
            rec.color = triangles[i].color;
            rec.reflectivity = triangles[i].reflectivity;
            rec.transparency = triangles[i].transparency;
            rec.refractiveIndex = triangles[i].refractiveIndex;
        }
    }
    return rec;
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
        HitRecord shadowRec = findClosestHit(shadowRay);
        bool inShadow = shadowRec.hit && shadowRec.t < distToLight;
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

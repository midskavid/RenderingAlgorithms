#include <algorithm>
#include <iostream>

#include "Integrator.h"
#include "Constants.h"


glm::vec3 PathTracerIntegrator::computeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 nr, const material_t& material) {
    glm::vec3 outColor{0,0,0};
    auto f_wi_wo = material.diffuse*INV_PI + material.specular*(material.shininess+2.0f)*INV_TWO_PI*float(pow(std::max(0.f,glm::dot(reflectedDir, wi)),material.shininess));
    auto n_wi = std::max(0.f,glm::dot(nr, wi));
    outColor = f_wi_wo*n_wi;
    return outColor;
}

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction, int depth) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    hitNormal = glm::normalize(hitNormal);
    if (hit) {
        outputColor = hitMaterial.emission;
        auto refl = glm::normalize(direction - 2*glm::dot(hitNormal, direction)*hitNormal);
        if (!hitMaterial.isLightSource && depth<=mMaxDepth) {
            glm::vec3 w_i = sampleW_I(hitNormal);
            outputColor += TWO_PI*computeShading(refl, w_i, hitNormal, hitMaterial)*traceRay(hitPosition, w_i, depth+1);
        }
    }
    return outputColor;
}

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction) {
    return traceRay(origin, direction, 1);
}

glm::vec3 PathTracerIntegrator::sampleW_I(glm::vec3 nr) {
    static thread_local std::mt19937 generator; // do I need mRD here?
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    auto u1 = distribution(generator);
    auto u2 = distribution(generator);

    float theta = std::acos(u1);
    float phi = TWO_PI*u2;

    glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta)); 
    auto w = glm::normalize(nr);
    glm::vec3 a(1,0,0);
    if (glm::length(w-a) < 0.01f) 
        a = glm::vec3(0,1,0);
    
    auto u = glm::normalize(glm::cross(a,w));
    auto v = glm::normalize(glm::cross(w,u));
    return samp.x*u + samp.y*v + samp.z*w;
}
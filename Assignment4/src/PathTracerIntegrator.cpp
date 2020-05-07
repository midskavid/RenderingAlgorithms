#include <algorithm>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include "Integrator.h"
#include "Constants.h"


glm::vec3 PathTracerIntegrator::computeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 nr, const material_t& material) {
    glm::vec3 outColor{0,0,0};
    auto f_wi_wo = material.diffuse*INV_PI + material.specular*(material.shininess+2.0f)*INV_TWO_PI*float(pow(std::max(0.f,glm::dot(reflectedDir, wi)),material.shininess));
    auto n_wi = std::max(0.f,glm::dot(nr, wi));
    outColor = f_wi_wo*n_wi;
    return outColor;
}

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction, int depth, glm::vec3 throughput) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    hitNormal = glm::normalize(hitNormal);
    if (hit) {
        if (_scene->NEE) {
            if (depth>1 && hitMaterial.isLightSource) return outputColor;
            outputColor = mDirectInt.traceRay(origin, direction);
        }
        else 
            outputColor = hitMaterial.emission;
        if (!hitMaterial.isLightSource && depth<mMaxDepth) {
            float q = 0.0f;
            if (_scene->RR) {
                q = 1.0f - std::min(1.0f, std::max({throughput.x, throughput.y, throughput.z}));
                if (GetUniformRandom()<q) return outputColor;
            }
            auto refl = glm::normalize(direction - 2*glm::dot(hitNormal, direction)*hitNormal);
            glm::vec3 w_i = sampleW_I(hitNormal);
            glm::vec3 newThroughput = TWO_PI*computeShading(refl, w_i, hitNormal, hitMaterial);
            newThroughput *= (1.0f/(1.0f-q));
            outputColor += newThroughput*traceRay(hitPosition, w_i, depth+1, newThroughput);
        }
    }
    return outputColor;
}

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction) {
    return traceRay(origin, direction, 1, glm::vec3(1,1,1));
}

float PathTracerIntegrator::GetUniformRandom() {
    static thread_local std::mt19937 generator; // do I need mRD here?
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    return distribution(generator);
}

glm::vec3 PathTracerIntegrator::sampleW_I(glm::vec3 nr) {
    auto u1 = GetUniformRandom();
    auto u2 = GetUniformRandom();

    float theta = std::acos(u1);
    float phi = TWO_PI*u2;

    glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta)); 
    auto w = nr;
    glm::vec3 a(0,1,0);
    //if (glm::length(glm::cross(a,w))<0.01f) //Expensive??
    if (glm::length2(w-a)<0.01f||glm::length2(-w-a)<0.01f)
        a = glm::vec3(1,0,0);

    auto u = glm::normalize(glm::cross(a,w));
    auto v = glm::normalize(glm::cross(w,u));
    auto w_i =  samp.x*u + samp.y*v + samp.z*w;
    return w_i;
}
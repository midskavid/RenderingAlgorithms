#include <algorithm>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include "Integrator.h"
#include "Constants.h"
#include "BRDF.h"

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction, int depth, glm::vec3 throughput) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);
    BRDF* _brdf = _scene->brdf;
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
            glm::vec3 w_i;
            glm::vec3 newThroughput;
            direction = -direction;
            switch (_scene->importanceSampling)
            {
            case ImportanceSampling::kHemisphere:  
            {              
                w_i = _brdf->SampleHemisphereW_I(hitNormal);
                auto n_wi = std::max(0.f,glm::dot(hitNormal, w_i));
                newThroughput = TWO_PI*_brdf->ComputeShading(refl, w_i, direction, hitNormal, hitMaterial)*n_wi;
                break;
            }
            case ImportanceSampling::kCosine: 
            {   
                w_i = _brdf->SampleCosineW_I(hitNormal);  
                auto n_wi = std::max(0.f,glm::dot(hitNormal, w_i));        
                newThroughput = PI*_brdf->ComputeShading(refl, w_i, direction, hitNormal, hitMaterial)*n_wi;
                break;
            }
            case ImportanceSampling::kBRDF:
            {
                w_i = _brdf->Sample_BRDFWi(refl, w_i, direction, hitNormal, hitMaterial);
                auto shading = _brdf->ComputeShading(refl, w_i, direction, hitNormal, hitMaterial);
                float pdf = _brdf->ComputePDF(refl, w_i, direction, hitNormal, hitMaterial);
                auto n_wi = std::max(0.f,glm::dot(hitNormal, w_i));
                newThroughput = shading*n_wi/pdf; 
                //std::cout<<newThroughput.x<<" "<<newThroughput.y<<" "<<newThroughput.z<<std::endl;
                break;
            }
            }
             
            
            newThroughput *= (1.0f/(1.0f-q));
            outputColor += newThroughput*traceRay(hitPosition, w_i, depth+1, newThroughput);
        }
    }
    //std::cout<<outputColor.x<<" "<<outputColor.y<<" "<<outputColor.z<<std::endl;
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
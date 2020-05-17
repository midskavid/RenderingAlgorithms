#include <algorithm>
#include <iostream>
#include <glm/gtx/norm.hpp>
#include "Integrator.h"
#include "Constants.h"
#include "BRDF.h"

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction, int depth, glm::vec3 throughput) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);
    
    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    hitNormal = glm::normalize(hitNormal);
    BRDF* _brdf = hitMaterial.brdf;

    // if (depth>mMaxDepth&&hitMaterial.isLightSource) {
    //     if (glm::dot(hitNormal, direction)<0)
    //         return outputColor; // change this to take normal
    //     else 
    //         return hitMaterial.emission;
    // }

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
                newThroughput = PI*_brdf->ComputeShading(refl, w_i, direction, hitNormal, hitMaterial);
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
    // if (std::isnan(outputColor.x)||std::isnan(outputColor.y)||std::isnan(outputColor.z))
    //     std::cout<<"JHOL\n";

    return outputColor;
}

glm::vec3 PathTracerIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction) {
    if (!_scene->MIS)
        return traceRay(origin, direction, 1, glm::vec3(1,1,1));
    else 
        return traceRayMIS(origin, direction, 1, glm::vec3(1,1,1));
}

glm::vec3 PathTracerIntegrator::traceRayMIS(glm::vec3 origin, glm::vec3 direction, int depth, glm::vec3 throughput) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    hitNormal = glm::normalize(hitNormal);
    BRDF* _brdf = hitMaterial.brdf;

    if (depth>mMaxDepth) {
        if (glm::dot(hitNormal, direction)<0)
            return outputColor; // change this to take normal
        else 
            return hitMaterial.emission;
    }

    if (hit) {
        if (depth>1 && hitMaterial.isLightSource) return outputColor;
        if (hitMaterial.isLightSource) {
            if (glm::dot(hitNormal, direction)<0)
                return outputColor; // change this to take normal
            else 
                return hitMaterial.emission;
        } 
        auto weightedDirColor = GetWeightedDirColor(hitPosition, hitNormal, _brdf, direction, hitMaterial);
        outputColor += weightedDirColor;
        
        glm::vec3 w_i;
        auto refl = glm::normalize(direction - 2*glm::dot(hitNormal, direction)*hitNormal);
        direction = -direction;
        w_i = _brdf->Sample_BRDFWi(refl, w_i, direction, hitNormal, hitMaterial);
        auto shading = _brdf->ComputeShading(refl, w_i, direction, hitNormal, hitMaterial);
        float pdf = _brdf->ComputePDF(refl, w_i, direction, hitNormal, hitMaterial);
        auto n_wi = std::max(0.f,glm::dot(hitNormal, w_i));
        auto wt = GetWeight(w_i, hitPosition, _brdf, false, refl, direction, hitNormal, hitMaterial);
        
        if (pdf>0)
            outputColor += (shading*n_wi*wt/pdf)*traceRay(hitPosition, w_i, depth+1, throughput);
        //newThroughput = shading*n_wi/pdf; 

    }
    return outputColor;
}

glm::vec3 PathTracerIntegrator::GetWeightedDirColor(const glm::vec3& hitPosition, const glm::vec3& hitNormal, BRDF* _brdf, glm::vec3 direction, const material_t& hitMaterial) {
    auto refl = glm::normalize(direction - 2*glm::dot(hitNormal, direction)*hitNormal);
    direction = -direction;
    glm::vec3 outputColor {0,0,0};
    int numLights = _scene->quadLights.size();

    for (const auto& light : _scene->quadLights) {
        glm::vec3 outputColor_ = glm::vec3(0.0f, 0.0f, 0.0f);

        auto ltPt = light._a + GetUniformRandom()*light._ab + GetUniformRandom()*light._ac;
        glm::vec3 toLight = ltPt - hitPosition;
        float lightDistance = glm::length(toLight);
        toLight /= lightDistance;

        bool occluded = _scene->castOcclusionRay(hitPosition, toLight, lightDistance);
        if (!occluded) {
            auto n_wi = std::max(0.f,glm::dot(hitNormal, toLight));
            outputColor_ += (_brdf->ComputeShading(refl, toLight, direction, hitNormal, hitMaterial)*n_wi);
        }
        auto wt = GetWeight(toLight, hitPosition, _brdf, true, refl, direction, hitNormal, hitMaterial);
        auto pne = pdfnee(toLight, hitPosition);
        if (pne>0)
            outputColor += outputColor_*light._intensity*wt/pne;
        // if (std::isnan(outputColor.x)||std::isnan(outputColor.y)||std::isnan(outputColor.z))
        //     std::cout<<"JHOL "<<pdfnee(toLight, hitPosition)<<"\n";

    }
    return outputColor/float(numLights);
}

float PathTracerIntegrator::pdfnee(glm::vec3 wi, glm::vec3 pos) {
    int numLights = _scene->quadLights.size();

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    int idx = 0;
    bool hit = _scene->castRayToLight(pos, wi, &hitPosition, &hitNormal, &hitMaterial, &idx);
    float pdf = 0.0;
    if (hit && hitMaterial.isLightSource) {
        auto Rsq = glm::distance2(hitPosition, pos);
        auto A = _scene->quadLights[idx]._area;
        auto nl =  _scene->quadLights[idx]._normal;
        pdf = Rsq/(A*std::abs(glm::dot(nl,wi)));
    }
    return (1.0f/numLights)*pdf;
}

float PathTracerIntegrator::GetWeight(glm::vec3 wi, glm::vec3 pos, BRDF* _brdf, bool nee, const glm::vec3& reflectedDir, const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    if (nee) {
        auto neePDF = pdfnee(wi, pos);
        auto brdfPDF = _brdf->ComputePDF(reflectedDir, wi, wo, nr, material);
        return (neePDF*neePDF)/ (neePDF*neePDF + brdfPDF*brdfPDF);
    }
    else {
        auto neePDF = pdfnee(wi, pos);
        auto brdfPDF = _brdf->ComputePDF(reflectedDir, wi, wo, nr, material);
        return (brdfPDF*brdfPDF)/ (neePDF*neePDF + brdfPDF*brdfPDF);
    }
    return 0.;
}

float PathTracerIntegrator::GetUniformRandom() {
    static thread_local std::mt19937 generator; // do I need mRD here?
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    return distribution(generator);
}
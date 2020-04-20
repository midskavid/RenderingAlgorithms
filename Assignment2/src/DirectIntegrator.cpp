#include <algorithm>
#include <iostream>

#include "Integrator.h"
#include "Constants.h"

glm::vec3 DirectIntegrator::computeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 nr, glm::vec3 nl, const material_t& material) {
    glm::vec3 outColor{0,0,0};
    auto f_wi_wo = material.diffuse*INV_PI + material.specular*(material.shininess+2.0f)*INV_TWO_PI*float(pow(std::max(0.f,glm::dot(reflectedDir, wi)),material.shininess));
    auto n_wi = std::max(0.f,glm::dot(nr, wi));
    auto nl_wi = std::abs(glm::dot(nl, wi)); //TODO [mkaviday] check this
    outColor = f_wi_wo*n_wi*nl_wi;
    return outColor;
}


glm::vec3 DirectIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    if (hit) {
        if (hitMaterial.isLightSource) {
            outputColor = hitMaterial.emission;
        }
        else {
            auto refl = direction - 2*glm::dot(hitNormal, direction)*hitNormal;
            for (const auto& light : _scene->quadLights) {
                // Generate Samples..
                auto unifSamples = GenerateUniformRandomSamples();
                if (lightStratify) {

                }
                else {
                    for (int ii=0;ii<numLightSamples;++ii) {
                        auto ltPt = light._a + unifSamples[ii].x*light._ab + unifSamples[ii].y*light._ac;
                        glm::vec3 toLight = ltPt - hitPosition;
                        float lightDistance = glm::length(toLight);
                        toLight /= lightDistance;

                        bool occluded = _scene->castOcclusionRay(hitPosition, toLight, lightDistance);
                        if (!occluded) {
                            outputColor += (computeShading(refl, toLight, hitNormal, light._normal, hitMaterial))/(lightDistance*lightDistance);
                        }
                    }
                }
                outputColor = outputColor*(light._intensity*(light._area/numLightSamples));
            }
        }
    }
    return outputColor;
}

std::vector<glm::vec2> DirectIntegrator::GenerateUniformRandomSamples() {
    static thread_local std::mt19937 generator; // do I need mRD here?
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    std::vector<glm::vec2> data;
    //std::cout<<distribution(generator);
    for (int ii=0;ii<numLightSamples;++ii) {
        data.emplace_back(glm::vec2(distribution(generator), distribution(generator)));
    }
    //std::generate(data.begin(), data.end(), [distribution]() { return glm::vec2 (distribution(generator),distribution(generator)); });
    return data;    
}
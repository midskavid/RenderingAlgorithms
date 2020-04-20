#include "Integrator.h"
#include <algorithm>
#include <iostream>

glm::vec3 DirectIntegrator::computeShading(glm::vec3 incidentDirection, glm::vec3 toLight, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material) {
    glm::vec3 outColor{0,0,0};
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
            for (const auto& light : _scene->quadLights) {
                // Generate Samples.. 
                // Check if visible..
                // Compute shading..
                //outputColor += computeShading(hitPosition, light, hitNormal, light._intensity, hitMaterial);
            }
        }
    }
    return outputColor;
}

std::vector<glm::vec2> DirectIntegrator::GenerateUniformRandomSamples(int N) {
    static thread_local std::mt19937 generator(mRD()); // do I need mRD here?
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    std::vector<glm::vec2> data;
    //std::cout<<distribution(generator);
    for (int ii=0;ii<N;++ii) {
        data.emplace_back(glm::vec2(distribution(generator), distribution(generator)));
    }
    //std::generate(data.begin(), data.end(), [distribution]() { return glm::vec2 (distribution(generator),distribution(generator)); });
    return data;    
}

std::vector<glm::vec2> DirectIntegrator::GenerateStratifiedUniformRandomSamples(int n) {
    return { };
}

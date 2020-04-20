#include "Integrator.h"
#include "Constants.h"

glm::vec3 AnalyticIntegrator::computeShading(const glm::vec3& posHit, const quadLight_t& light, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material) {
    glm::vec3 phiR {0,0,0};

    auto theta_1 = glm::acos(glm::dot(glm::normalize(light._a-posHit), glm::normalize(light._b-posHit)));
    auto gamma_1 = glm::normalize(glm::cross(light._a-posHit, light._b-posHit));

    auto theta_2 = glm::acos(glm::dot(glm::normalize(light._b-posHit), glm::normalize(light._d-posHit)));
    auto gamma_2 = glm::normalize(glm::cross(light._b-posHit, light._d-posHit));

    auto theta_3 = glm::acos(glm::dot(glm::normalize(light._d-posHit), glm::normalize(light._c-posHit)));
    auto gamma_3 = glm::normalize(glm::cross(light._d-posHit, light._c-posHit));

    auto theta_4 = glm::acos(glm::dot(glm::normalize(light._c-posHit), glm::normalize(light._a-posHit)));
    auto gamma_4 = glm::normalize(glm::cross(light._c-posHit, light._a-posHit));

    phiR = (.5f)*(theta_1*gamma_1+theta_2*gamma_2+theta_3*gamma_3+theta_4*gamma_4);

    return INV_PI*lightBrightness*material.diffuse*glm::dot(phiR, normal);
}

glm::vec3 AnalyticIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction) {
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
                outputColor += computeShading(hitPosition, light, hitNormal, light._intensity, hitMaterial);
            }
        }
    }
    return outputColor;
}
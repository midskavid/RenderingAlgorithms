#include "Integrator.h"

glm::vec3 AnalyticIntegrator::computeShading(glm::vec3 incidentDirection, const quadLight_t& light, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material) {
    return glm::vec3{1,0,0};
}

glm::vec3 AnalyticIntegrator::traceRay(glm::vec3 origin, glm::vec3 direction) {
    glm::vec3 outputColor = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    material_t hitMaterial;
    bool hit = _scene->castRay(origin, direction, &hitPosition, &hitNormal, &hitMaterial);
    if (hit) {
        for (const auto& light : _scene->quadLights) {
            outputColor += computeShading(-direction, light, hitNormal, light._intensity, hitMaterial);
        }
    }
    return outputColor;
}
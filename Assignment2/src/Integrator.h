#pragma once

#include <glm/glm.hpp>

#include "Scene.h"

enum class IntegratorType {
    kRayTracerIntegrator,
    kAnalyticIntegrator
};

class Integrator {
public:
    void setScene(Scene* scene) {
        _scene = scene;
    }
    void SetIntegrator(IntegratorType _type) {
        mIntegratorType = _type;
    }
    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) = 0;
protected:
    IntegratorType mIntegratorType;
    Scene* _scene;

};


class RayTracerIntegrator : public Integrator {
    glm::vec3 computeShading(
        glm::vec3 incidentDirection,
        glm::vec3 toLight,
        glm::vec3 normal,
        glm::vec3 lightBrightness,
        const material_t& material);

    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth);

public:

    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction);

};

class AnalyticIntegrator : public Integrator {
    glm::vec3 computeShading(glm::vec3 incidentDirection, const quadLight_t& light, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material);

public:
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) override;

};
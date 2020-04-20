#pragma once

#include <glm/glm.hpp>
#include <random>

#include "Scene.h"

enum class IntegratorType {
    kRayTracerIntegrator,
    kAnalyticIntegrator,
    kDirectIntegrator
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
    glm::vec3 computeShading(glm::vec3 incidentDirection, glm::vec3 toLight, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material);
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth);

public:
    virtual glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction);

};

class AnalyticIntegrator : public Integrator {
    glm::vec3 computeShading(const glm::vec3& posHit, const quadLight_t& light, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material);

public:
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) override;

};

class DirectIntegrator : public Integrator {
    glm::vec3 computeShading(glm::vec3 incidentDirection, glm::vec3 toLight, glm::vec3 normal, glm::vec3 lightBrightness, const material_t& material);
    std::vector<glm::vec2> GenerateUniformRandomSamples(int N);
    std::vector<glm::vec2> GenerateStratifiedUniformRandomSamples(int N);

public :
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) override;

private:
    std::random_device mRD;

};
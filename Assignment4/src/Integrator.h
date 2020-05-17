#pragma once

#include <glm/glm.hpp>
#include <random>

#include "Scene.h"

enum class IntegratorType {
    kRayTracerIntegrator,
    kAnalyticIntegrator,
    kDirectIntegrator,
    kPathTracerIntegrator
};

class Integrator {
public:
    virtual void setScene(Scene* scene) {
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
    glm::vec3 computeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 nr, glm::vec3 nl, const material_t& material);
    std::vector<glm::vec2> GenerateUniformRandomSamples();

public :
    DirectIntegrator(bool _stratify, int _num) : lightStratify(_stratify), numLightSamples(_num) { }
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) override;

private :
    std::random_device mRD;
    bool lightStratify;
    int numLightSamples;
    
};

class PathTracerIntegrator : public Integrator {
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction, int depth, glm::vec3 throughput);
    glm::vec3 traceRayMIS(glm::vec3 origin, glm::vec3 direction, int depth, glm::vec3 throughput);
    float GetUniformRandom();
    glm::vec3 GetWeightedDirColor(const glm::vec3& refl, const glm::vec3& hitPosition, const glm::vec3& hitNormal, BRDF* _brdf, const glm::vec3& direction, const material_t& hitMaterial);
    float pdfnee(glm::vec3 wi, glm::vec3 hitPos);
    float GetWeight(glm::vec3 wi, glm::vec3 pos, BRDF* _brdf, bool nee, const glm::vec3& reflectedDir, const glm::vec3& wo,const glm::vec3& nr, const material_t& material);
public :
    PathTracerIntegrator(int _depth, bool _stratify, int _num) : mMaxDepth(_depth), mDirectInt(_stratify, _num) { }
    glm::vec3 traceRay(glm::vec3 origin, glm::vec3 direction) override;
    void setScene(Scene* scene) override {
        _scene = scene;
        mDirectInt.setScene(_scene);
    }

private :
    std::random_device mRD;
    int mMaxDepth;
    DirectIntegrator mDirectInt;
};
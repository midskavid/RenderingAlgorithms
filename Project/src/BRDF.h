#ifndef BRDF_H
#define BRDF_H

#include <random>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <embree3/rtcore.h>

#include "Scene.h"
#include "Constants.h"

class BRDF {
public :
    virtual glm::vec3 Sample_BRDFWi(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) = 0;
    virtual float ComputePDF(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) = 0;
    virtual glm::vec3 ComputeShading(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) = 0;
    glm::vec3 SampleHemisphereW_I(const glm::vec3& nr);
    glm::vec3 SampleCosineW_I(const glm::vec3& nr);
    virtual ~BRDF() { }
protected :
    float GetUniformRandom();

private :
    std::random_device mRD;

};

class PhongBRDF : public BRDF {
public :
    glm::vec3 Sample_BRDFWi(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) override;
    float ComputePDF(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) override;
    glm::vec3 ComputeShading(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) override;
    virtual ~PhongBRDF() { }
};

class GGXBRDF : public BRDF {
    float D_H(float theta_h, float alpha);
    float G1_V(glm::vec3 w, glm::vec3 nr, float alpha);
    glm::vec3 F(glm::vec3 ks, glm::vec3 w, glm::vec3 h);
public :
    glm::vec3 Sample_BRDFWi(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) override;
    float ComputePDF(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) override;
    glm::vec3 ComputeShading(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) override;
    virtual ~GGXBRDF() { }
};

#endif // BRDF_H
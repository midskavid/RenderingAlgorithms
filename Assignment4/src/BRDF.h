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
    virtual glm::vec3 Sample_BRDFWi(glm::vec3 nr, glm::vec3 refl, float t, float s) = 0;
    virtual float ComputePDF(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, float t, const material_t& material) = 0;
    virtual glm::vec3 ComputeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, const material_t& material) = 0;
    glm::vec3 SampleHemisphereW_I(glm::vec3 nr);
    glm::vec3 SampleCosineW_I(glm::vec3 nr);

protected :
    float GetUniformRandom();

private :
    std::random_device mRD;

};

class PhongBRDF : public BRDF {
public :
    glm::vec3 Sample_BRDFWi(glm::vec3 nr, glm::vec3 refl, float t, float s) override;
    float ComputePDF(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, float t, const material_t& material) override;
    glm::vec3 ComputeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, const material_t& material) override;

};

class GGXBRDF : public BRDF {
    float D_H(float theta_h, float alpha);
    float G1_V(glm::vec3 w, glm::vec3 nr, float alpha);
    glm::vec3 F(glm::vec3 ks, glm::vec3 w, glm::vec3 h);
public :
    glm::vec3 Sample_BRDFWi(glm::vec3 nr, glm::vec3 refl, float t, float s) override;
    float ComputePDF(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, float t, const material_t& material) override;
    glm::vec3 ComputeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, const material_t& material) override;
    
};

#endif // BRDF_H
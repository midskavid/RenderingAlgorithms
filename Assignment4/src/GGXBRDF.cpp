#include "BRDF.h"

float GGXBRDF::D_H(float theta_h, float alpha) {
    return alpha*alpha/(PI* std::pow(std::cos(theta_h),4)*(alpha*alpha + std::pow(std::tan(theta_h),2)));
}

float GGXBRDF::G1_V(glm::vec3 w, glm::vec3 nr, float alpha) {
    if (glm::dot(w, nr) > 0) {
        auto theta_v = std::acos(glm::dot(w, nr));
        return 2.0f/ (1.0f + sqrt(1+alpha*alpha*tan(theta_v)*tan(theta_v)));
    }
    return 0.f;
}

glm::vec3 GGXBRDF::F(glm::vec3 ks, glm::vec3 w, glm::vec3 h) {
    return ks + (1.0f-ks)*float(std::pow(1.0f-glm::dot(w,h),5));
}

glm::vec3 GGXBRDF::ComputeShading(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, const material_t& material) {
    auto diffuse = material.diffuse*INV_PI;
    glm::vec3 specular {0,0,0};

    auto wi_n = glm::dot(wi, nr);
    auto wo_n = glm::dot(wo, nr);

    if (wi_n>0&&wo_n>0) {
        auto h = glm::normalize(wi+wo);
        auto theta_h = std::acos(glm::dot(h, nr));
        auto dh = D_H(theta_h, material.alpha);
        auto g_wi_wo = G1_V(wi, nr, material.alpha)*G1_V(wo, nr, material.alpha);
        auto f_wi_h = F(material.specular, wi, h); 

        specular = (f_wi_h*g_wi_wo*dh) / (4.0f*wi_n*wo_n);
    }
    return diffuse + specular;
}

float ComputePDF(glm::vec3 reflectedDir, glm::vec3 wi, glm::vec3 wo, glm::vec3 nr, float t, const material_t& material) {
    auto 
}
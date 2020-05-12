#include "BRDF.h"

glm::vec3 PhongBRDF::Sample_BRDFWi(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    UNUSED(wi);
    UNUSED(wo);
    auto u0 = GetUniformRandom();
    auto u1 = GetUniformRandom();
    auto u2 = GetUniformRandom();
    float theta = 0;

    float kdBar = (material.diffuse.r + material.diffuse.g + material.diffuse.b)/3.0f;
    float ksBar = (material.specular.r + material.specular.g + material.specular.b)/3.0f;
    float t = ksBar/(ksBar+kdBar);


    glm::vec3 w;

    if (u0<=t) {
        theta = std::acos(std::pow(u1, 1.0f/(1.0f+material.shininess)));
        w = reflectedDir;
    }
    else {
        theta = std::acos(sqrt(u1));
        w = nr;
    }
    
    float phi = TWO_PI*u2;

    glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta)); 
    glm::vec3 a(0,1,0);
    //if (glm::length(glm::cross(a,w))<0.01f) //Expensive??
    if (glm::length2(w-a)<0.01f||glm::length2(-w-a)<0.01f)
        a = glm::vec3(1,0,0);

    auto u = glm::normalize(glm::cross(a,w));
    auto v = glm::normalize(glm::cross(w,u));
    auto w_i =  samp.x*u + samp.y*v + samp.z*w;
    return w_i;
}

float PhongBRDF::ComputePDF(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    UNUSED(wo);
    float kdBar = (material.diffuse.r + material.diffuse.g + material.diffuse.b)/3.0f;
    float ksBar = (material.specular.r + material.specular.g + material.specular.b)/3.0f;
    float t = ksBar/(ksBar+kdBar);
    
    auto pdf = (1.0f-t)*(std::max(0.f,glm::dot(nr, wi)))*INV_PI + t*(material.shininess+1.0f)*INV_TWO_PI*float(pow(std::max(0.f,glm::dot(reflectedDir, wi)),material.shininess));
    return pdf;
}

glm::vec3 PhongBRDF::ComputeShading(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    UNUSED(wo);
    glm::vec3 outColor{0,0,0};
    auto f_wi_wo = material.diffuse*INV_PI + material.specular*(material.shininess+2.0f)*INV_TWO_PI*float(pow(std::max(0.f,glm::dot(reflectedDir, wi)),material.shininess));
    
    outColor = f_wi_wo;
    auto n_wi = std::max(0.f,glm::dot(nr, wi));
    outColor *= n_wi;

    return outColor;
}

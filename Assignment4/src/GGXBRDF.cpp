#include "BRDF.h"

float GGXBRDF::D_H(float theta_h, float alpha) {
    return (alpha*alpha)/(PI* std::pow(std::cos(theta_h),4)*std::pow((alpha*alpha + std::pow(std::tan(theta_h),2)),2));
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

glm::vec3 GGXBRDF::ComputeShading(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    UNUSED(reflectedDir);
    auto diffuse = material.diffuse*INV_PI;
    glm::vec3 specular {0,0,0};

    auto wi_n = glm::dot(wi, nr);
    auto wo_n = glm::dot(wo, nr);

    if ((wi_n>0)&&(wo_n>0)) {
        auto h = glm::normalize(wi+wo);
        auto theta_h = std::acos(glm::dot(h, nr));
        auto dh = D_H(theta_h, material.alpha);
        auto g_wi_wo = G1_V(wi, nr, material.alpha)*G1_V(wo, nr, material.alpha);
        auto f_wi_h = F(material.specular, wi, h); 

        specular = (f_wi_h*g_wi_wo*dh) / (4.0f*wi_n*wo_n);
    }
    
    return diffuse + specular;
}

float GGXBRDF::ComputePDF(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    UNUSED(reflectedDir);
    float kdBar = (material.diffuse.r + material.diffuse.g + material.diffuse.b)/3.0f;
    float ksBar = (material.specular.r + material.specular.g + material.specular.b)/3.0f;
    float t = std::max(0.25f,ksBar/(ksBar+kdBar));

    // auto wi_n = glm::dot(wi, nr);
    // auto wo_n = glm::dot(wo, nr);
    auto h = glm::normalize(wi+wo);
    auto h_nr = glm::dot(h, nr);
    auto theta_h = std::acos(h_nr);
    auto h_wi = glm::dot(h,wi);
    //auto pdf = (1.0f-t)*(std::max(0.f,glm::dot(nr, wi)))*INV_PI;
    auto pdf = (1.0f-t)*(glm::dot(nr, wi))*INV_PI;
    if (h_wi!=0)
        pdf += (t*D_H(theta_h, material.alpha)*h_nr)/(4.0f*h_wi);

    return pdf;
}

glm::vec3 GGXBRDF::Sample_BRDFWi(const glm::vec3& reflectedDir,const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& nr, const material_t& material) {
    UNUSED(wi);
    UNUSED(reflectedDir);
    auto u0 = GetUniformRandom();
    auto u1 = GetUniformRandom();
    auto u2 = GetUniformRandom();

    float kdBar = (material.diffuse.r + material.diffuse.g + material.diffuse.b)/3.0f;
    float ksBar = (material.specular.r + material.specular.g + material.specular.b)/3.0f;
    float t = std::max(0.25f,ksBar/(ksBar+kdBar));

    
    if (u0<=t) { //specular
        float phi = TWO_PI*u2;
        float theta = std::atan((material.alpha*sqrt(u1))/(sqrt(1.0f-u1)));
        glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta));
        auto w = nr;
        glm::vec3 a(0,1,0);
        if (glm::length(glm::cross(a,w))<0.01f) //Expensive??
        //if (glm::length2(w-a)<0.01f||glm::length2(-w-a)<0.01f)
            a = glm::vec3(1,0,0);

        auto u = glm::normalize(glm::cross(a,w));
        auto v = glm::normalize(glm::cross(w,u));
        auto h_i =  samp.x*u + samp.y*v + samp.z*w;
        auto w_i = glm::normalize(wo - 2*glm::dot(h_i, wo)*h_i);
        return w_i;
    }
    else {
        float theta = std::acos(sqrt(u1));
        auto w = nr;
        float phi = TWO_PI*u2;

        glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta)); 
        glm::vec3 a(0,1,0);
        if (glm::length(glm::cross(a,w))<0.01f) //Expensive??
        //if (glm::length2(w-a)<0.01f||glm::length2(-w-a)<0.01f)
            a = glm::vec3(1,0,0);

        auto u = glm::normalize(glm::cross(a,w));
        auto v = glm::normalize(glm::cross(w,u));
        auto w_i =  samp.x*u + samp.y*v + samp.z*w;
        return w_i;
    }
    return glm::vec3 {};
}
#include "BRDF.h"

float BRDF::GetUniformRandom() {
#pragma message("Check this")
    static thread_local std::mt19937 generator; // do I need mRD here?
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    return distribution(generator);
}

glm::vec3 BRDF::SampleHemisphereW_I(const glm::vec3& nr) {
    auto u1 = GetUniformRandom();
    auto u2 = GetUniformRandom();

    float theta = std::acos(u1);
    float phi = TWO_PI*u2;

    glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta)); 
    auto w = nr;
    glm::vec3 a(0,1,0);
    //if (glm::length(glm::cross(a,w))<0.01f) //Expensive??
    if (glm::length2(w-a)<0.01f||glm::length2(-w-a)<0.01f)
        a = glm::vec3(1,0,0);

    auto u = glm::normalize(glm::cross(a,w));
    auto v = glm::normalize(glm::cross(w,u));
    auto w_i =  samp.x*u + samp.y*v + samp.z*w;
    return w_i;
}

glm::vec3 BRDF::SampleCosineW_I(const glm::vec3& nr) {
    auto u1 = GetUniformRandom();
    auto u2 = GetUniformRandom();

    float theta = std::acos(sqrt(u1));
    float phi = TWO_PI*u2;

    glm::vec3 samp (cos(phi)*sin(theta), sin(phi)*sin(theta), cos(theta)); 
    auto w = nr;
    glm::vec3 a(0,1,0);
    //if (glm::length(glm::cross(a,w))<0.01f) //Expensive??
    if (glm::length2(w-a)<0.01f||glm::length2(-w-a)<0.01f)
        a = glm::vec3(1,0,0);

    auto u = glm::normalize(glm::cross(a,w));
    auto v = glm::normalize(glm::cross(w,u));
    auto w_i =  samp.x*u + samp.y*v + samp.z*w;
    return w_i;
}


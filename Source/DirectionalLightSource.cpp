#include "LightSource.h"


// TODO : [mkaviday]
bool DirectionalLightSource::Unoccluded(Interaction& itr) const {
    return false;
}

Vector3f DirectionalLightSource::GetDirection(Point3f pt) const {
    return Vector3f();
}

RGBColor DirectionalLightSource::GetAttenuation() const {
    return RGBColor();
}
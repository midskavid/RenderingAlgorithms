#include "LightSource.h"

// TODO : [mkaviday]
bool PointLightSource::Unoccluded(Interaction& itr) const {
    return false;
}

Vector3f PointLightSource::GetDirection(Point3f pt) const {
    return Vector3f();
}

RGBColor PointLightSource::GetAttenuation() const {
    return RGBColor();
}
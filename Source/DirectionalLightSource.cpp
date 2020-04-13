#include "LightSource.h"
#include "RenderScene.h"

bool DirectionalLightSource::Unoccluded(Interaction& itr) const {
    auto pos = itr.GetPosition();

    Ray r = Ray(pos+Float(0.001)*mDirection, mDirection);
    Float its = renderer->mScene->GetClosestDistance(r);

    if(its<0.0) {
        return true;
    }
    return false;
}

Vector3f DirectionalLightSource::GetDirection(Point3f pt) const {
    return mDirection;
}

Float DirectionalLightSource::GetAttenuation(Point3f pos) const {
    return 1.0;
}
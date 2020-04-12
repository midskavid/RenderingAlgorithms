#include "LightSource.h"
#include "RenderScene.h"

bool PointLightSource::Unoccluded(Interaction& itr) const {
    auto pos = itr.GetPosition();
    auto direction = Normalize(mPosition-pos);

    Ray r = Ray(pos+Float(0.01)*direction, direction);
    Float its = renderer->mScene->GetClosestDistance(r);

    if(its<0.0) {
        return true;
    }

    Float dist = (pos-mPosition).Length();
    if(its<dist) {
        return false;
    }
    else {
        return true;
    }
}

Vector3f PointLightSource::GetDirection(Point3f pt) const {
    return Normalize(mPosition-pt);
}

Float PointLightSource::GetAttenuation(Point3f pos) const {
    auto dist = Distance(mPosition,pos);
    Float atten = mAttenuation[0]+mAttenuation[1]*dist+mAttenuation[2]*pow(dist,2);
    return 1/atten;
}
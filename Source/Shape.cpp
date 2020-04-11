#include "Shape.h"
#include "RenderScene.h"

RGBColor Shape::GetColor(Interaction& itrPoint) {
    if (!itrPoint.IsHit()) return RGBColor(0,0,0);
    RGBColor color(0,0,0);

    for (const auto& lt:renderer->mScene->mLight) {
        if (lt->Unoccluded(itrPoint)) {
            Vector3f eyeDir = itrPoint.GetOutDirection();
            Vector3f lightDir = lt->GetDirection(itrPoint.GetPosition());
            Vector3f half = Normalize(eyeDir+lightDir);
            RGBColor lamb = mDiffuse*std::max(Dot(itrPoint.GetNormal(),eyeDir),Float(0.0));
            RGBColor phong = mSpecular*pow(Dot(itrPoint.GetNormal(),half), mShininess);
            //TODO : [mkaviday] check attenuation
            //color = color + lt->GetAttenuation(itrPoint.GetPosition())*(lt->GetColor()*(lamb + phong));
            color = color + lt->GetColor()*(lamb + phong);
        }
    }

    color = color + mAmbient + mEmission;
    return color;
}
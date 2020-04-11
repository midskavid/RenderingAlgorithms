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
            Float NDotL = std::max(Dot(itrPoint.GetNormal(), lightDir),Float(0.));
            RGBColor lamb = mDiffuse*NDotL*lt->GetColor();
            Float NDotH = std::max(Dot(itrPoint.GetNormal(), half),Float(0.));
            RGBColor phong = lt->GetColor()*mSpecular*pow(NDotH, mShininess);
            //TODO : [mkaviday] check attenuation
            color = color + (lamb+phong)*lt->GetAttenuation(itrPoint.GetPosition());
        }
    }

    color = color + mAmbient + mEmission;
    return color;
}
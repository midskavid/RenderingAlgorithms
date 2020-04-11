#include "Shape.h"


RGBColor Shape::GetColor(Interaction& itrPoint, std::vector<LightSource*>& lights) {
    if (!itrPoint.IsHit()) return RGBColor(0,0,0);
    RGBColor color(0,0,0);

    for (const auto& lt:lights) {
        if (lt->Unoccluded(itrPoint)) {
            Vector3f eyeDir = itrPoint.GetOutDirection();
            Vector3f lightDir = lt->GetDirection(itrPoint.GetPosition);
            Vector3f half = Normalize(eyeDir+lightDir);
            RGBColor lamb = mDiffuse*std::max(Dot(itrPoint.GetNormal(),eyeDir),Float(0.0));
            RGBColor phong = mSpecular*pow(Dot(itrPoint.GetNormal(),half), mShininess);
            //TODO : [mkaviday] check attenuation
            color = color + lt->GetAttenuation()*lt->GetColor()*(lamb + phong);
        }
    }

    color = color + mAmbient + mEmission;
    return color;
}
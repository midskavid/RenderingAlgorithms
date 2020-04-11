#ifndef SHAPE_H
#define SHAPE_H

#include "CoreTypes.h"
#include "Geometry.h"
#include "Transform.h"
#include "RGBColor.h"
#include "Interaction.h"
#include "LightSource.h"

// For now keeping material properties inside shape.. Later create a material class.
// Encapsulate both inside a Primitive class as done in PBRT
class Shape {
public:
    Shape(const Transform ObjectToWorld_, RGBColor diffuse, RGBColor ambient, RGBColor specular, RGBColor emission, Float shininess) :
            mObjectToWorld(ObjectToWorld_),
                mDiffuse(diffuse),
                mAmbient(ambient),
                mSpecular(specular),
                mEmission(emission),
                mShininess(shininess) { }

    virtual ~Shape() { }
    //virtual Bounds3f ObjectBounds() const = 0;
    //virtual Bounds3f WorldBounds() const;
    virtual Interaction Intersect(const Ray &ray) const = 0;
    RGBColor GetColor(Interaction& itrPoint);
    RGBColor GetSpecular() const {return mSpecular;}
protected:
    RGBColor mAmbient;
    RGBColor mDiffuse;
    RGBColor mSpecular;
    RGBColor mEmission;
    Float mShininess;
private:
    const Transform mObjectToWorld;
};

#endif
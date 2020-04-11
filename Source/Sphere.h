#ifndef SPHERE_H
#define SPHERE_H

#include "Shape.h"

class Sphere : public Shape {
public:
    Sphere(Point3f cen, Float radius, const Transform trans, RGBColor diffuse, RGBColor ambient, RGBColor specular, RGBColor emission, Float shininess) : 
            mCenter(cen),
            mRadius(radius),
            Shape(trans, diffuse, ambient, specular, emission, shininess) {
                mWorldToObject = Transform(mObjectToWorld.GetInverseMatrix(), mObjectToWorld.GetMatrix());
             }
    //Bounds3f ObjectBounds() const override;
    Interaction Intersect(const Ray& ray) const override;
private:
    Vector3f GetNormal(Point3f pt) const;
private:
    Float mRadius;
    Point3f mCenter;
    Transform mWorldToObject;
};

#endif
#ifndef SPHERE_H
#define SPHERE_H

#include "Shape.h"

class Sphere : public Shape {
public:
    Sphere(Point3f cen, Float radius, const Transform trans, RGBColor diffuse, RGBColor ambient, RGBColor specular, RGBColor emission, Float shininess) : 
            mCenter(cen),
            mRadius(radius),
            Shape(trans, diffuse, ambient, specular, emission, shininess) {
                mWorldToObject = Inverse(trans);
                mBounds = mObjectToWorld(Bounds3f(Point3f(mCenter[0]-mRadius, mCenter[1]-mRadius, mCenter[2]-mRadius), Point3f(mCenter[0]+mRadius, mCenter[1]+mRadius, mCenter[2]+mRadius)));
                mCentroid = mCenter;
             }
    Bounds3f GetWorldBounds() const override {return mBounds;}
    Interaction Intersect(const Ray& ray) const override;
    Point3f GetCentroid() const override {return mCentroid;}
private:
    Vector3f GetNormal(Point3f pt) const;
private:
    Float mRadius;
    Point3f mCenter;
    Transform mWorldToObject;
};

#endif
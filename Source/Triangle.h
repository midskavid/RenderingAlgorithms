#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Shape.h"

class Triangle : public Shape {
public:
    Triangle(Point3f vert1, Point3f vert2, Point3f vert3, const Transform trans, RGBColor diffuse, RGBColor ambient, RGBColor specular, RGBColor emission, Float shininess) : 
                mV1(vert1),
                mV2(vert2),
                mV3(vert3),
                Shape(trans, diffuse, ambient, specular, emission, shininess) {
                    mN = Normalize(Cross(mV2-mV1, mV3-mV1));
                 }
    //Bounds3f ObjectBounds() const override;
    Interaction Intersect(const Ray& ray) const override;
private:
    Point3f mV1;
    Point3f mV2;
    Point3f mV3;
    Vector3f mN;
};

#endif
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
                    mBounds = Bounds3f(mV1, mV2, mV3);
                    mCentroid = (mV1+mV2+mV3)/2;
			        mU = mV2-mV1;
                    mV = mV3-mV1;
                    d1 = Dot(mU, mU);
                    d2 = Dot(mU, mV);
                    d3 = Dot(mV, mV);
                 }
    Bounds3f GetWorldBounds() const override {return mBounds;}
    Interaction Intersect(const Ray& ray) const override;
    Point3f GetCentroid() const override {return mCentroid;}
private:
    Point3f mV1;
    Point3f mV2;
    Point3f mV3;
    Vector3f mN;
    Vector3f mU;
    Vector3f mV;
    Float d1, d2, d3;
};

#endif
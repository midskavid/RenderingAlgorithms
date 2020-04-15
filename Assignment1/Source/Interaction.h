#ifndef INTERACTION_H
#define INTERACTION_H

#include "CoreTypes.h"
#include "Geometry.h"
//TODO: [mkaviday] change this to Surface vs Media Interaction when need comes..
// Following convention used in PBRT..
class Interaction {
public:
    Interaction() : mHit(false), mShape(nullptr) { }
    Interaction(Point3f pos_, Vector3f normal_, Vector3f wo_, Shape* shape_) : mPos(pos_), mNormal(normal_), mWo(wo_) ,mShape(shape_), mHit(true) { }
    Ray SpawnRay(const Vector3f& d) const;
    Ray SpawnRayTo(const Point3f& pt) const;
    Point3f GetPosition() const {return mPos;}
    Vector3f GetNormal() const {return mNormal;}
    Shape* GetShape() const {return mShape;}
    bool IsHit() const {return mHit;}
    Vector3f GetOutDirection() const {return mWo;}
private:
    Point3f mPos;
    Vector3f mNormal; //TODO : [mkaviday] change this to Normal3f
    Shape* mShape; 
    Vector3f mWo;
    bool mHit;
};

#endif
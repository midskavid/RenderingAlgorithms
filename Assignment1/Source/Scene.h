#ifndef SCENE_H
#define SCENE_H

#include "CoreTypes.h"
#include "KDTree.h"

class Scene {
public:
    Scene (std::vector<LightSource*>& light_, std::vector<Shape*>& shape_, Bounds3f boundingBoxAll) : mLight(light_){
        mAcceleratedPrimitives = new KDTree();
        mAcceleratedPrimitives->Build(shape_, boundingBoxAll);
     }
    RGBColor GetColor(Ray& ray, int depth);
    Float GetClosestDistance(Ray& ray);
public:
    std::vector<LightSource*> mLight;
    KDTree* mAcceleratedPrimitives;
};

#endif
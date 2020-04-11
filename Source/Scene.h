#ifndef SCENE_H
#define SCENE_H

#include "CoreTypes.h"

class Scene {
public:
    Scene (std::vector<LightSource*>& light_, std::vector<Shape*>& shape_) : mLight(light_), mShape(shape_) { }
    RGBColor GetColor(Ray& ray, int depth);
    Float GetClosestDistance(Ray& ray);
public:
    std::vector<LightSource*> mLight;
    std::vector<Shape*> mShape;
};

#endif
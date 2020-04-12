#include "Scene.h"
#include "RGBColor.h"
#include "Transform.h"
#include "Shape.h"
#include "RenderScene.h"

RGBColor Scene::GetColor(Ray& ray, int depth) {
    RGBColor color(0,0,0);
    if (depth==renderer->mMaxDepth) return color;
    Float nearest = Infinity;
    Shape* hit = nullptr;
    Interaction closestIntr;

    for (const auto& shape:mShape) {
        auto intr = shape->Intersect(ray);
        if (intr.IsHit()) {
            // update to the closest object...
            auto dist = Distance(ray.o, intr.GetPosition());
            if (dist<nearest) {
                nearest = dist; 
                hit = intr.GetShape();
                closestIntr = intr;
            }
        }
    }

    if (hit) {
        Vector3f normal = closestIntr.GetNormal();
        Vector3f reflected = Normalize(ray.d - 2*Dot(normal,ray.d)*normal);
        // TODO:[mkaviday] offset ray..
        Ray refl(closestIntr.GetPosition()+0.01*reflected, reflected);
        //std::cout<<GetColor(refl, depth-1)<<std::endl;
        return hit->GetColor(closestIntr, depth) + hit->GetSpecular()*GetColor(refl, depth+1);

    }
    return color;
}

Float Scene::GetClosestDistance(Ray& ray) {
    Float closestDistance = -1;
    for (const auto& shape:mShape) {
        auto its = shape->Intersect(ray);
        if (its.IsHit()) {    
            Float dist = Distance(ray.o,its.GetPosition());
            if (closestDistance< 0 || dist < closestDistance) {
                closestDistance = dist;
            }
        }
    }
    return closestDistance;
}
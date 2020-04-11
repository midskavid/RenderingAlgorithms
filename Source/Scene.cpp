#include "Scene.h"
#include "RGBColor.h"
#include "Transform.h"
#include "Shape.h"

RGBColor Scene::GetColor(Ray& ray, int depth) {
    RGBColor color(0,0,0);
    if (depth==0) return color;
    Float nearest = Infinity;
    Shape* hit = nullptr;
    Interaction closestIntr;

    for (const auto& shape:mShape) {
        auto intr = shape->Intersect(ray); // updates tmax too..
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
       
        Vector3f comp = std::abs(Dot(normal,ray.d))*normal;
        Vector3f reflected = Normalize(ray.d + 2*comp);
        // TODO:[mkaviday] offset ray..
        Ray refl(closestIntr.GetPosition()+0.01*reflected, reflected);
        return hit->GetColor(closestIntr, mLight) + hit->GetSpecular()*GetColor(refl, depth-1);

    }

    return color;
}
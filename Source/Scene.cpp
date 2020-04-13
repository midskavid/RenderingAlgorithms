#include "Scene.h"
#include "RGBColor.h"
#include "Transform.h"
#include "Shape.h"
#include "RenderScene.h"

RGBColor Scene::GetColor(Ray& ray, int depth) {
    RGBColor color(0,0,0);
    if (depth==renderer->mMaxDepth) return color;
    Float nearest = Infinity;
    Interaction closestIntr;

    if (mAcceleratedPrimitives->Intersect(ray, &nearest, &closestIntr)) {
        Shape* hit = closestIntr.GetShape();
        Vector3f normal = closestIntr.GetNormal();
        Vector3f reflected = Normalize(ray.d - 2*Dot(normal,ray.d)*normal);
        // TODO:[mkaviday] offset ray..
        Ray refl(closestIntr.GetPosition()+Float(.1)*reflected, reflected);
        //std::cout<<GetColor(refl, depth-1)<<std::endl;
        return hit->GetColor(closestIntr, depth) + hit->GetSpecular()*GetColor(refl, depth+1);

    }
    return color;
}

Float Scene::GetClosestDistance(Ray& ray) {
    Float nearest = Infinity;
    Interaction closestIntr;
    if (mAcceleratedPrimitives->Intersect(ray, &nearest, &closestIntr)) {
        return nearest;
    }
    return -1;
}
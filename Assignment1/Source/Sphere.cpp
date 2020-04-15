#include "Sphere.h"

Interaction Sphere::Intersect(const Ray& ray) const {
    Ray transRay = mWorldToObject(ray);
    auto o = transRay.o;
    auto d = transRay.d;
    Float a = Dot(d,d);
    Float b = Dot((Float)2*d,(o-mCenter));
    Float c = Dot((o-mCenter),(o-mCenter))- mRadius*mRadius;


    Float delta = b*b-4*a*c;
    
    if (delta < 0.0) { return Interaction();}

    Float s1 = (-b + sqrt(delta))/(2*a);
    Float s2 = (-b - sqrt(delta))/(2*a);
    
    Float t;
    if (s1 > 0 && s2 > 0) t = s1<s2?s1:s2;
    else if (s1 > 0) t = s1; 
    else if (s2 > 0) t = s2;
    else return Interaction();
    
    Point3f position = o+t*d;
    position = mObjectToWorld(position);
    Vector3f normal = GetNormal(position);
    return Interaction(position, normal,-ray.d,(Shape*)this);    
}

Vector3f Sphere::GetNormal(Point3f pt) const {
    Normal3f normal = Normal3f(mWorldToObject(pt)-mCenter);
    return Vector3f(Normalize(mObjectToWorld(normal)));
}
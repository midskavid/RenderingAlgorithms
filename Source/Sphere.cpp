#include "Sphere.h"

Interaction Sphere::Intersect(const Ray& ray) const {
    auto transRay = mWorldToObject(ray);
    auto b = Dot((Float)2*transRay.d,(transRay.o-mCenter));
    auto a = Dot(transRay.d,transRay.d);
    auto c = Dot((transRay.o-mCenter),(transRay.o-mCenter))-(Float)pow(mRadius,2);

    Float delta = pow(b,2.0)-4*a*c;
    
    if (delta < 0.0) { return Interaction();}

    Float s1 = (-b + sqrt(delta))/2*a;
    Float s2 = (-b - sqrt(delta))/2*a;
    
    Float t;

    if (s1 > 0 && s2 > 0) {
        if (s1 < s2 || s1 == s2) { t = s1; } 
        else { t = s2; }
    }
    else if (s1 > 0) { t = s1; }
    else if (s2 > 0) { t = s2; }
    else { return Interaction(); }
    
    Point3f position = transRay.o+t*transRay.d;
    position = mObjectToWorld(position);
    Vector3f normal = GetNormal(position);
    return Interaction(position,normal,Vector3f(),(Shape*)this);    
}

Vector3f Sphere::GetNormal(Point3f pt) const {
    Vector3f normal = mWorldToObject(pt)-mCenter;
    normal = mObjectToWorld(normal);
    normal = Normalize(normal);
    return normal;
}
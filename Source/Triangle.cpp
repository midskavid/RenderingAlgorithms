#include "Triangle.h"
// TODO : [mkaviday] Change this to optimised version
Interaction Triangle::Intersect(const Ray& ray) const {
    Float t = Dot(mN,(ray.o-mV1))/Dot(mN,ray.d);
    t = -1.0*t;
    if(t<0){
        return Interaction();
    }

    Point3f position = ray.o+t*ray.d;

    if( ((Dot(Cross((mV2-mV1),(position-mV1)), mN)) >=0) &&
        ((Dot(Cross((mV3-mV2),(position-mV2)), mN)) >=0) &&
        ((Dot(Cross((mV1-mV3),(position-mV3)), mN)) >=0))
    {
            return Interaction(position, mN, Vector3f(), (Shape*)this);
    }
    else{
        return Interaction();
    }
}

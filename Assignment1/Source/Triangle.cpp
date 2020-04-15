#include "Triangle.h"
// TODO : [mkaviday] Change this to optimised version
Interaction Triangle::Intersect(const Ray& ray) const {
#if 1
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
            return Interaction(position, mN, -ray.d, (Shape*)this);
    }
    else{
        return Interaction();
    }
#else
	Float t = -1;
    auto p0 = ray.o;
    auto p1 = ray.d;
    Float d = Dot(p1, mN);
	if (d == 0) return Interaction();

	t = Dot(mV1 - p0, mN) / d;
	auto p = p0 + t*p1;
	if (!Inside(p, mBounds)) return Interaction();
	
	auto u2 = p - mV1;
	Float d4 = Dot(u2, mU);
	Float d5 = Dot(u2, mV);
	Float denom = d1 * d3 - d2 * d2;
	Float beta = (d3 * d4 - d2 * d5) / denom;
	Float gamma = (d1 * d5 - d2 * d4) / denom;
	Float alpha = 1.0f - beta - gamma;

	return (beta >= 0 && beta <= 1
		&& gamma >= 0 && gamma <= 1
		&& beta + gamma <= 1)
		? Interaction(p, mN, -ray.d, (Shape*)this) : Interaction();
#endif
}

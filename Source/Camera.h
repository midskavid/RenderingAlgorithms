#ifndef CAMERA_H
#define CAMERA_H

#include "Geometry.h"

class Camera {
public:
    Camera(Vector3f camPos, Vector3f camLookAt, Vector3f camUp, Float fovy, int height, int width);
    Ray GenerateRay(Point2i pixel);
private :
    Vector3f mW;
    Vector3f mU;
    Vector3f mV;
    Point3f mCamPos;
    Float mFovX;
    Float mFovY;
    int mWidth;
    int mHeight;
};

#endif
#include "Camera.h"

Camera::Camera(Vector3f camPos, Vector3f camLookAt, Vector3f camUp, Float fovy, int height, int width) {
    mHeight = height;
    mWidth = width;
    mCamPos = Point3f(camPos[0], camPos[1], camPos[2]);
    mW = Normalize(camPos-camLookAt);
    mU = Normalize(Cross(mW,camUp));
    mV = Cross(mW,mU);
    mFovY = fovy*M_PI/180.0;
    Float z = tan(mFovY/2);

    z = (1.0/z)*height/2;

    mFovX = 2*atan((width/2)/z);

}

Ray Camera::GenerateRay(Point2i pixel) {
    // TODO:[mkaviday] verify sign here...
    Float alpha = -tan(mFovX / 2) * (pixel.y + 0.5 - (mWidth / 2)) / (mWidth / 2);
    Float beta =  tan(mFovY / 2) * ((mHeight / 2) - pixel.x-0.5) / (mHeight / 2);
    
    Vector3f direction = alpha*mU+ beta*mV-mW;
    direction = Normalize(direction);
    return Ray(mCamPos,direction);

}
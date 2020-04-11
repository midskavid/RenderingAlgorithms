#ifndef RENDERSCENE_H
#define RENDERSCENE_H

#include "CoreTypes.h"
#include "Geometry.h"
#include "Scene.h"

class RenderScene {
public:
    RenderScene(std::string fileName) : mSceneFileName(fileName), mOutFileName("output.png"), mMaxDepth(4) { }
    void ReadFile();
    void Render(std::string outFileName);
private:
    bool ReadVals(std::stringstream &s, const int numvals, Float* values);
private:
    int mWidth, mHeight;
    int mMaxDepth;
    Scene* mScene;
    Camera* mCamera;
    Film* mFilm;
    std::string mSceneFileName;
    std::string mOutFileName;
};

#endif
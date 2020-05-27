#include "AMLD.h"

AMLD::AMLD(int _spp, int _w, int _h) {
    mWidth = _w;
    mHeight = _h;
    mNumPixels = mWidth*mHeight;
    mRemSamples = mNumPixels*_spp;
    mNumItr = 4; //4
    mBlockSize = 8; //8
    mInitSpp = 2; //2
    mPixelColor = new std::vector<glm::vec3> (mNumPixels);
}

void AMLD::AddPixelColor(int idx, glm::vec3& _c) {
    mPixelColor[idx].push_back(_c);
}

void AMLD::CreateImportanceMap(int iteration) {
    
}
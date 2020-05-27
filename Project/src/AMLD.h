#ifndef AMLD_H
#define AMLD_H

#include <vector>
#include <glm/glm.hpp>

class AMLD {
public:
    AMLD(int _spp, int _w, int _h);
    void CreateImportanceMap(int iteration);
    void AddPixelColor(int idx, glm::vec3& _c);
private:
    int mRemSamples;
    int mNumItr; //4
    int mBlockSize; //8
    int mInitSpp; //2
    int mWidth;
    int mHeight;
    int mNumPixels;
    std::vector<glm::vec3>* mPixelColor;
};

#endif //AMLD_H
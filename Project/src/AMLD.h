#ifndef AMLD_H
#define AMLD_H

#include <vector>
#include <glm/glm.hpp>

class AMLD {
    struct BlockData {
        std::vector<glm::vec3> sampleRGB;
        std::vector<float> pixelColorR;
        std::vector<float> pixelColorG;
        std::vector<float> pixelColorB;
    };

public:
    AMLD(int _spp, int _w, int _h);
    void CreateImportanceMap(int iteration);
    void AddPixelColor(int idx, glm::vec3& _c);
    int GetSPPForPixel(int pixel);
    int GetNumSamplesAtPixel(int pixel);

private: 
    void GetBlockDimensions(int& _l, int& _r, int& _t, int& _b, int _ii, int _jj);
    float GenerateImportanceMap();
    float GetContrastMap();
    float NoiseEstimation();
    float Median(float *x_data);
    void AdaptivelySample(int itr);
    std::vector<float> Dilation(std::vector<float> x_input, int x_w, int x_h);
    
public:
    std::vector<glm::vec3>* mPixelColor;

private:
    int mTotSamples;
    int mNumItr; //4
    int mBlockSize; //8
    int mInitSpp; //2
    int mMaxSPP;
    int mWidth;
    int mHeight;
    int mNumPixels;

    int mImportanceMapWidth;
    int mImportanceMapHeight;
    std::vector<float> mImportanceMap;
    std::vector<int> mAddSamples;
    BlockData mBlockData;
    int mSamplesForThisItr;
    std::vector<float> sampleDivision;
};

#endif //AMLD_H
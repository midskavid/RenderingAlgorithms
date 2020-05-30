#ifndef AMLD_H
#define AMLD_H

#include <vector>
#include <glm/glm.hpp>

class AMLD {
    struct BlockData {
        int validSize;
        std::vector<glm::vec3> sampleRGB;

        std::vector<float> pixelColorR;
        std::vector<float> pixelColorG;
        std::vector<float> pixelColorB;
        BlockData(int _s, int _p) {
            sampleRGB = std::vector<glm::vec3> (_s);
            pixelColorR = std::vector<float> (_p);
            pixelColorG = std::vector<float> (_p);
            pixelColorB = std::vector<float> (_p);
        }
    };

public:
    AMLD(int _spp, int _w, int _h);
    void CreateImportanceMap(int iteration);
    void AddPixelColor(int idx, glm::vec3& _c);
    int GetNumSamplesAtPixel(int pixel);
private: 
    void GetBlockDimensions(int itr, int& _l, int& _r, int& _t, int& _b, int _ii, int _jj);
    float GenerateImportanceMap();
    float GetContrastMap();
    float NoiseEstimation();
    float Median(float *x_data, int x_blkSize);
private:
    int mRemSamples;
    int mNumItr; //4
    int mBlockSize; //8
    int mInitSpp; //2
    int mWidth;
    int mHeight;
    int mNumPixels;

    int mImportanceMapWidth;
    int mImportanceMapHeight;
    std::vector<float> mImportanceMap;
    std::vector<glm::vec3>* mPixelColor;
    BlockData mBlockData;
};

#endif //AMLD_H
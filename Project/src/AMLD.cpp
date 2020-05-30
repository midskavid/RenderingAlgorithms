#include <algorithm>

#include "AMLD.h"
#include "Wavelet.h"

AMLD::AMLD(int _spp, int _w, int _h) : mBlockData(_spp*8*8, 8*8) {
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
    
    mImportanceMapWidth = ceil(float(mWidth/mBlockSize));
    mImportanceMapHeight = ceil(float(mHeight/mBlockSize));
    
    mImportanceMap = std::vector<float> (mImportanceMapHeight*mImportanceMapWidth, 0);
    for (int ii=0;ii<mImportanceMapHeight;++ii) {
        for (int jj=0;jj<mImportanceMapWidth;++jj) {
            int _l, _r, _t, _b;
            GetBlockDimensions(iteration, _l, _r, _t, _b, ii, jj);
            int sidx = 0;
            int pcoloridx = 0;
            //mBlockData.pixelColorR = 
            for (int py=_t;py<=_b;++py) {
                for (int px=_l;px<=_r;++px) {
                    if (py<mHeight&&py>=0&&px<mWidth&&px>=0) {
                        int _indPixel = py * mWidth + px;
                        auto pidx = py*mWidth + px;
                        auto sNum = GetNumSamplesAtPixel(pidx);
                        for (int ss=0;ss<sNum;++ss) {
                            mBlockData.sampleRGB[sidx] = mPixelColor[_indPixel][ss];
                            mBlockData.pixelColorR[pcoloridx] += mPixelColor[_indPixel][ss].r;
                            mBlockData.pixelColorG[pcoloridx] += mPixelColor[_indPixel][ss].g;
                            mBlockData.pixelColorB[pcoloridx] += mPixelColor[_indPixel][ss].b;
                            ++sidx;
                        }
                        mBlockData.pixelColorR[pcoloridx] /= sNum;
                        mBlockData.pixelColorG[pcoloridx] /= sNum;
                        mBlockData.pixelColorB[pcoloridx] /= sNum;
#pragma message ("Weird transform")                        
                        // mBlockData.pixelColorR[pcoloridx] = std::max(0.f, std::min(1.f, powf(mBlockData.pixelColorR[pcoloridx]/2.,0.5f)/0.9f));
                    	// mBlockData.pixelColorR[pcoloridx] = int((mBlockData.pixelColorR[pcoloridx])*255+0.5);

                    }
                    else {
                        mBlockData.pixelColorR[pcoloridx] = 0;
                        mBlockData.pixelColorG[pcoloridx] = 0;
                        mBlockData.pixelColorB[pcoloridx] = 0;
                    }
                    ++pcoloridx;
                }
            }
            mBlockData.validSize = sidx;
            mImportanceMap[ii*mImportanceMapWidth+jj] = GenerateImportanceMap();
        }
    }

    // dilate..

    // get extra samples to be added..

    // reset
    mImportanceMap.clear();
    
    if (mBlockSize > 2)
        mBlockSize /= 2; 
}

void AMLD::GetBlockDimensions(int itr, int& _l, int& _r, int& _t, int& _b, int _ii, int _jj) {
	if(itr == mNumItr) {
		_t = _ii - 4;
		_b = _ii + 3;
		_l = _jj - 4;
		_r = _jj + 3;
	}
	else {
		_t = _ii * mBlockSize-(8-mBlockSize)/2;
		_l = _jj * mBlockSize-(8-mBlockSize)/2;
		_b = _ii * mBlockSize+mBlockSize-1+(8-mBlockSize)/2;
		_r = _jj * mBlockSize+mBlockSize-1+(8-mBlockSize)/2;
	}
}    

int AMLD::GetNumSamplesAtPixel(int pixel) {
    return mPixelColor[pixel].size();
}

float AMLD::GenerateImportanceMap() {
	float contrastMap = GetContrastMap(); // Equation 6
	float noise = NoiseEstimation(); // Equation 3
	//m_maxNoise = std::max(m_maxNoise, noise);
	return sqrt(contrastMap * noise); // Equation 7!!!
}

float AMLD::GetContrastMap() {
    
}

float AMLD::NoiseEstimation() {
	int _length = 64;
	float _factor = 1 / 0.6745;

	wavelet_transfer_2d(8, 8, &mBlockData.pixelColorR.front(), 3, 1);
	float *_diagWaveCoeffR = getDiagDetail(&mBlockData.pixelColorR.front(), 8);
	float _noiseR = Median(_diagWaveCoeffR, 8) * _factor;

	wavelet_transfer_2d(8, 8, &mBlockData.pixelColorG.front(), 3, 1);
	float *_diagWaveCoeffG = getDiagDetail(&mBlockData.pixelColorG.front(), 8);
	float _noiseG = Median(_diagWaveCoeffG, 8) * _factor;

	wavelet_transfer_2d(8, 8, &mBlockData.pixelColorB.front(), 3, 1);
	float *_diagWaveCoeffB = getDiagDetail(&mBlockData.pixelColorB.front(), 8);
	float _noiseB = Median(_diagWaveCoeffB, 8) * _factor;

	return (powf(_noiseR,2) + powf(_noiseG,2) + powf(_noiseB,2))/3;

}

float AMLD::Median(float *x_data, int x_blkSize)
{
	int _size = x_blkSize * x_blkSize * .25;
	for (int i = 0; i < _size; i++)
	{
		x_data[i] = fabs(x_data[i]);
	}
	std::sort (x_data, x_data+_size);
	return (fabs(x_data[int(_size*.5)])+fabs(x_data[int(_size*.5)-1]))/2;
}
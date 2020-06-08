#include <algorithm>
#include <iostream>

#include "AMLD.h"
#include "Wavelet.h"

constexpr float eps = 1e-5;

AMLD::AMLD(int _spp, int _w, int _h) : sampleDivision{7.0f,3.0f,1.0f, 1.0f} {
    mWidth = _w;
    mHeight = _h;
    mNumPixels = mWidth*mHeight;
    mTotSamples = mNumPixels*_spp;
    mNumItr = 4; //4
    mBlockSize = 8; //8
    mInitSpp = 2; //2
	mMaxSPP = _spp*8; //heurestic as seen from author's code;
    mPixelColor = new std::vector<glm::vec3> [mNumPixels];
	mAddSamples = std::vector<int> (mNumPixels, 2); //initial samples..
	mSamplesForThisItr = (mTotSamples - 2*mNumPixels) / sampleDivision[0];
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
            GetBlockDimensions(_l, _r, _t, _b, ii, jj);
            int sidx = 0;
            int pcoloridx = 0;
            mBlockData.pixelColorR.clear();
			mBlockData.pixelColorG.clear();
			mBlockData.pixelColorB.clear();
			mBlockData.sampleRGB.clear();

            for (int py=_t;py<=_b;++py) {
                for (int px=_l;px<=_r;++px) {
					mBlockData.pixelColorR.push_back(0);
					mBlockData.pixelColorG.push_back(0);
					mBlockData.pixelColorB.push_back(0);
                    if ((py<mHeight)&&(py>=0)&&(px<mWidth)&&(px>=0)) {
                        int _indPixel = py * mWidth + px;
                        auto sNum = GetNumSamplesAtPixel(_indPixel);
                        for (int ss=0;ss<sNum;++ss) {
                            mBlockData.sampleRGB.push_back(mPixelColor[_indPixel][ss]);
                            mBlockData.pixelColorR[pcoloridx] += mPixelColor[_indPixel][ss].r;
                            mBlockData.pixelColorG[pcoloridx] += mPixelColor[_indPixel][ss].g;
                            mBlockData.pixelColorB[pcoloridx] += mPixelColor[_indPixel][ss].b;
                            ++sidx;
                        }
                        mBlockData.pixelColorR[pcoloridx] /= sNum;
                        mBlockData.pixelColorG[pcoloridx] /= sNum;
                        mBlockData.pixelColorB[pcoloridx] /= sNum;
                    }
                    else {
                        mBlockData.pixelColorR[pcoloridx] = 0;
                        mBlockData.pixelColorG[pcoloridx] = 0;
                        mBlockData.pixelColorB[pcoloridx] = 0;
                    }
                    ++pcoloridx;
                }
            }
            mImportanceMap[ii*mImportanceMapWidth+jj] = GenerateImportanceMap();
        }
    }
    // get extra samples to be added..
    mAddSamples = std::vector<int> (mWidth*mHeight,0);
    AdaptivelySample(iteration);
    // reset
    mImportanceMap.clear();
    
    if (mBlockSize > 2)
        mBlockSize /= 2; 
}

void AMLD::GetBlockDimensions(int& _l, int& _r, int& _t, int& _b, int _ii, int _jj) {
	_t = _ii * mBlockSize-(8-mBlockSize)/2;
	_l = _jj * mBlockSize-(8-mBlockSize)/2;
	_b = _ii * mBlockSize+mBlockSize-1+(8-mBlockSize)/2;
	_r = _jj * mBlockSize+mBlockSize-1+(8-mBlockSize)/2;
}    

int AMLD::GetNumSamplesAtPixel(int pixel) {
    return mPixelColor[pixel].size();
}

float AMLD::GenerateImportanceMap() {
	float contrastMap = GetContrastMap(); // Equation 6
	float noise = NoiseEstimation(); // Equation 3
	return sqrt(contrastMap * noise); // Equation 7!!!
}

float AMLD::GetContrastMap() {
	float gammaR = 0;
	float gammaG = 0;
	float gammaB = 0;
	float muR = 0;
	float muG = 0;
	float muB = 0;
	int _size = mBlockData.sampleRGB.size();

	for(int ii=0;ii<_size;++ii)
	{
		muR += mBlockData.sampleRGB[ii].r;
		muG += mBlockData.sampleRGB[ii].g;
		muB += mBlockData.sampleRGB[ii].b;
	}
	muR /= _size;
	muG /= _size;
	muB /= _size;

    if(muR>eps)	{ // trick from authors code.. else does not converge..
        for(int ii=0;ii<_size;++ii)
            gammaR += fabs(mBlockData.sampleRGB[ii].r-muR);
        gammaR /= (muR*_size);
    }

    if(muG>eps)	{
        for(int ii=0;ii<_size;++ii)
            gammaG += fabs(mBlockData.sampleRGB[ii].g-muG);
        gammaG /= (muG*_size);
    }

    if(muB>eps)	{
        for(int ii=0;ii<_size;++ii)
            gammaB += fabs(mBlockData.sampleRGB[ii].b-muB);
        gammaB /= (muB*_size);
    }
	return (gammaR+gammaG+gammaB)/3.0;
}

float AMLD::NoiseEstimation() {
    WaveletSampling(8, 8, &mBlockData.pixelColorR.front());
    float *diagR = DiagDetail(&mBlockData.pixelColorR.front());
    float noiseR = Median(diagR)*1.4825796f;

    WaveletSampling(8, 8, &mBlockData.pixelColorG.front());
    float *diagG = DiagDetail(&mBlockData.pixelColorG.front());
    float noiseG = Median(diagG)*1.4825796f;

    WaveletSampling(8, 8, &mBlockData.pixelColorB.front());
    float *diagB = DiagDetail(&mBlockData.pixelColorB.front());
    float noiseB = Median(diagB)*1.4825796f;
	
	delete diagR;
	delete diagG;
	delete diagB;
    
	return (powf(noiseR,2) + powf(noiseG,2) + powf(noiseB,2))/3;

}

float AMLD::Median(float *x_data)
{
	int _size = 16;
	for (int i = 0; i < _size; i++)
	{
		x_data[i] = fabs(x_data[i]);
	}
	std::sort (x_data, x_data+_size);
	return fabs(x_data[int(_size*.5)]);
}

void AMLD::AdaptivelySample(int itr) {
	float allImpo = 0;
	for (const auto& im:mImportanceMap) 
		allImpo += im;

	if (allImpo<=0)
		std::cout<<"Importance is WRONG~!!!!\n";

	int samplesForThis = mSamplesForThisItr;
	for (int ii=0;ii<mImportanceMapHeight;++ii) {
		for (int jj=0;jj<mImportanceMapWidth;++jj) {
			int _l, _r, _t, _b;
			int blkIdx = ii*mImportanceMapWidth + jj;
			_l = jj*mBlockSize;
			_r = std::min(mWidth,(jj+1)*mBlockSize-1);
			_t = ii*mBlockSize;
			_b = std::min(mHeight,(ii+1)*mBlockSize-1);

			int _newSamp = ceil(((mImportanceMap[blkIdx]/allImpo)*samplesForThis+.5f)/((_r-_l+1)*(_b-_t+1)));
			allImpo -= mImportanceMap[blkIdx]; // clever trick in author's code to handle the issue when not all samples can be used as we have hit the max spp for a any pixel!!!

			for (int pii=_t;pii<=_b;++pii) {
				for (int pjj=_l;pjj<=_r;++pjj) {
					int pxidx = pii*mWidth + pjj;
					auto  _add = mMaxSPP - GetNumSamplesAtPixel(pxidx);
					if (_add>_newSamp) {
						mAddSamples[pxidx] += _newSamp;
						samplesForThis -= _newSamp;
					}
					else {
						mAddSamples[pxidx] += _add;
						samplesForThis -= _add;
					}
				}
			}
		}
	}
	// Dump image of samples..

	// Recalculate for next iteration!!!
	int usedSamples = 0;
	for (int ii=0;ii<mNumPixels;++ii) 
		usedSamples += mAddSamples[ii] + GetNumSamplesAtPixel(ii);
	
	mSamplesForThisItr = (mTotSamples - usedSamples)/sampleDivision[itr];
}

int AMLD::GetSPPForPixel(int pixel) {
	return mAddSamples[pixel];
}
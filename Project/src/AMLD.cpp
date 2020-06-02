#include <algorithm>
#include <iostream>

#include "AMLD.h"
#include "Wavelet.h"

constexpr float eps = 1e-5;

AMLD::AMLD(int _spp, int _w, int _h) : mBlockData(_spp*8*8, 8*8), sampleDivision{7.0f,3.0f,1.0f, 1.0f} {
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
    mImportanceMap = Dilation(mImportanceMap, mImportanceMapWidth, mImportanceMapHeight);
    // get extra samples to be added..
    mAddSamples = std::vector<int> (mWidth*mHeight,0);
    AdaptivelySample(iteration);
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
	float noise = 1.0f;//NoiseEstimation(); // Equation 3
	//m_maxNoise = std::max(m_maxNoise, noise);
	return sqrt(contrastMap * noise); // Equation 7!!!
}

float AMLD::GetContrastMap() {
	float gammaR = 0;
	float gammaG = 0;
	float gammaB = 0;
	float muR = 0;
	float muG = 0;
	float muB = 0;

	for(int ii=0;ii<mBlockData.validSize;++ii)
	{
		muR += mBlockData.sampleRGB[ii].r;
		muG += mBlockData.sampleRGB[ii].g;
		muB += mBlockData.sampleRGB[ii].b;
	}
	muR /= mBlockData.validSize;
	muG /= mBlockData.validSize;
	muB /= mBlockData.validSize;

    if(muR>eps)	{
        for(int ii=0;ii<mBlockData.validSize;++ii)
            gammaR += abs(mBlockData.sampleRGB[ii].r-muR);
        gammaR /= (muR*mBlockData.validSize);
    }

    if(muG>eps)	{
        for(int ii=0;ii<mBlockData.validSize;++ii)
            gammaG += abs(mBlockData.sampleRGB[ii].g-muG);
        gammaG /= (muG*mBlockData.validSize);
    }

    if(muB>eps)	{
        for(int ii=0;ii<mBlockData.validSize;++ii)
            gammaB += abs(mBlockData.sampleRGB[ii].b-muB);
        gammaB /= (muB*mBlockData.validSize);
    }
	return (gammaR+gammaG+gammaB)/3.0;
}

float AMLD::NoiseEstimation() {
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
	
	delete _diagWaveCoeffR;
	delete _diagWaveCoeffG;
	delete _diagWaveCoeffB;
    
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

std::vector<float> AMLD::Dilation(std::vector<float> x_input, int x_w, int x_h)
{
	std::vector<float> _output(x_h*x_w);
	for (int yy = 0; yy<x_h; yy++)
		for(int xx = 0; xx<x_w; xx++)
		{
			// compare the N8 neighbors
			int pind = yy*x_w+xx;
			_output[pind] = x_input[pind]; //important
			int ImaxInd = pind;
			float Imax = x_input[ImaxInd];
			int nind;			  
			// left-up
			nind = std::max(0,yy-1) * x_w + std::max(0,xx-1);
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// middle-up
			nind = std::max(0,yy-1) * x_w + xx;
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// right-up
			nind = std::max(0,yy-1) * x_w + std::min(x_w-1,xx+1);
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// right-middle
			nind = yy * x_w + std::min(x_w-1,xx+1);
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// right-bottom
			nind = std::min(x_h-1,yy+1) * x_w + std::min(x_w-1,xx+1);
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// middle-bottom
			nind = std::min(x_h-1,yy+1) * x_w + xx;
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// left-bottom
			nind = std::min(x_h-1,yy+1) * x_w + std::max(0,xx-1);
			if(x_input[nind]>Imax)
				Imax = x_input[nind];
			// left-middle
			nind = yy * x_w + std::max(0,xx-1);
			if(x_input[nind]>Imax)
				Imax = x_input[nind];

			// replace with Imax
			if(Imax>x_input[pind])
				_output[pind] = Imax;			  
		}
	return _output;	
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

			int _newSamp = ((mImportanceMap[blkIdx]/allImpo)*samplesForThis+.5f)/((_r-_l+1)*(_b-_t+1));
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
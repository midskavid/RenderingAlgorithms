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
	Dilation(mImportanceMap, mImportanceMapWidth, mImportanceMapHeight);
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

	std::vector<float> diagC(16);
    WaveletSampling(mBlockData.pixelColorR);
    DiagDetail(mBlockData.pixelColorR, diagC);
    float noiseR = Median(diagC)*1.4825796f;

    WaveletSampling(mBlockData.pixelColorG);
    DiagDetail(mBlockData.pixelColorG, diagC);
    float noiseG = Median(diagC)*1.4825796f;

    WaveletSampling(mBlockData.pixelColorB);
    DiagDetail(mBlockData.pixelColorB, diagC);
    float noiseB = Median(diagC)*1.4825796f;
	
	return (powf(noiseR,2) + powf(noiseG,2) + powf(noiseB,2))/3;

}

float AMLD::Median(std::vector<float>& diagC) {
	for (auto& ii:diagC) ii = fabs(ii);
	std::sort (diagC.begin(), diagC.end());
	return (fabs(diagC[8]) + fabs(diagC[9]))/2.;
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

void AMLD::Dilation(std::vector<float>& impMap, int W, int H)
{
#if 0
	std::vector<float> dilatedImpMap(W*H, 0);
	for (int ii=0; ii<H; ++ii) {
		for(int jj=0; jj<W; ++jj) {
			auto idx = ii*W+jj;
			for (auto kk:{-1,0,1}) {
				for (auto ll:{-1,0,1}) {
					if (ii+kk>=0&&ii+kk<=H&&jj+ll>=0&&jj+ll<=W) {
						auto currIdx = (ii+kk)*W+jj+ll;
						dilatedImpMap[idx] = std::max(impMap[currIdx], dilatedImpMap[idx]);
					}
				}
			}
		}
	}
	impMap.assign(dilatedImpMap.begin(), dilatedImpMap.end());
#else
	auto x_h = H;
	auto x_w = W;
	std::vector<float>& x_input = impMap;
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
	impMap.assign(_output.begin(), _output.end());
#endif
	
}
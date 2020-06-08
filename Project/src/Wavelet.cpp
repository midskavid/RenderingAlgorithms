#include <stdlib.h>
#include <vector>

#include "Wavelet.h"


const std::vector<float> h{0.707106781186547f,0.707106781186547f};
const std::vector<float> g{-0.707106781186547f,0.707106781186547f};

void Wave1d(std::vector<float>& input, int idx, std::vector<float>& output, int step)
{
    float *a = &input[idx];
    int n = 8;
    int i,j;
    int half  = n >> 1;
    int len = 2;

    for(i = 0; i < n; i++)
    {
        output[i] = 0;
    }

    for(i = 0; i < half; i++)
    {
        int low = i;
        int high = ((len/2 - 1 + i) + half)%half + half;

        for(j = 0; j < len; j++)
        {
            output[low] += h[j] * a[((i*2+j)%n)*step];
            output[high] += g[j] * a[((i*2+j)%n)*step];
        }

    }
}

void WaveletSampling(std::vector<float>& pixCol)
{
    int _size = 8;
    std::vector<float> tmp(_size);

    for(int ii=0;ii<int(pixCol.size());ii+=8){
        Wave1d(pixCol, ii, tmp, 1);
        for (int jj=0;jj<8;++jj) {
            pixCol[ii+jj] = tmp[jj];
        }
    }

    // Along columns
    for(int ii=0;ii<8;++ii){
        Wave1d(pixCol,ii, tmp, 8);
        for(int jj=0;jj<8;++jj){
            pixCol[ii+jj*8] = tmp[jj];
        }
    }
}

float* DiagDetail(float *x_Coeff)
{
	float *_detCoeff = new float[16];
	for (int i = 0; i < 16; i++)
		_detCoeff[i] = x_Coeff[36+(i%4)+(i/4)*8];
	return _detCoeff;
}
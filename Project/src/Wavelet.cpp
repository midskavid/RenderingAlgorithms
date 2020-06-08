#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "Wavelet.h"


const std::vector<float> h{0.707106781186547f,0.707106781186547f};
const std::vector<float> g{-0.707106781186547f,0.707106781186547f};

void Wave1d(int n,int step, float *a, float *output)
{
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
            output[low] += h[j] * a[((i*2+j)%n) * step];
            output[high] += g[j] * a[((i*2+j)%n) * step];
        }

    }
}

void WaveletSampling(int numRows, int numCols, float *input)
{
    int length_temp;

    int step;
    float *temp;
    float *ptr = NULL, *pp = NULL;

    int i,j;
    length_temp = (numRows>numCols)?numRows:numCols;
    temp = (float*)malloc(sizeof(float)*length_temp);

    ptr = input;
    step = 1;
    for(i = 0; i < numCols; i++){
        Wave1d(numRows,step,ptr,temp);
        pp = ptr;
        for( j = 0; j <numRows; j++){
            *pp = temp[j];
            pp+=step;
        }
        ptr+= numCols;
    }


    /* first do transfer for rows */
    ptr = input;
    step = numCols; /* tricky here */
    for(i = 0; i < numRows; i++){
        Wave1d(numCols,step,ptr,temp);
        pp = ptr;
        for( j = 0; j <numCols; j++){
            *pp = temp[j];
            pp+=step;
        }
        ptr += 1;
    }


    free(temp);
}

float* DiagDetail(float *x_Coeff)
{
	float *_detCoeff = new float[16];
	for (int i = 0; i < 16; i++)
		_detCoeff[i] = x_Coeff[36+(i%4)+(i/4)*8];
	return _detCoeff;
}
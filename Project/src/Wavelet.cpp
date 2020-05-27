#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>

#include "Wavelet.h"
typedef std::vector<float> vf;

vf h2{0.707106781186547f,0.707106781186547f};
vf g2{-0.707106781186547f,0.707106781186547f};
vf h4{0.482962913144831f,0.836516303737708f,0.224143868041922f,-0.129409522550955f};
vf g4{0.129409522550955f,0.224143868041922f,-0.836516303737708f,0.482962913144831f};

vf h8{0.230377813308959f,0.714846570552874f,0.630880767929889f,-0.027983769416995f,
               -0.187034811718967f,0.030841381835995f,0.032883011666994f,-0.010597401784998f};

vf g8{0.010597401784998f,0.032883011666994f,-0.030841381835995f,-0.187034811718967f,
               0.027983769416995f,0.630880767929889f,-0.714846570552874f,0.230377813308959f};

void wave_transfer_1d(int n,int step, float *a, float *output,vf& h,vf& g, int len)
{
    int i,j;
    int half  = n >> 1;
    
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

void wavelet_transfer_2d_sub(int num_rows, int num_cols, int step_col, float *input,
                             vf& h, vf& g, int len)
{
    int length_temp;

    int step;
    float *temp;
    float *ptr = NULL, *pp = NULL;

    int i,j;
    length_temp = (num_rows>num_cols)?num_rows:num_cols;
    temp = (float*)malloc(sizeof(float)*length_temp);
    /* input is a colunm vector */

    /* second do transfer for columns */
    ptr = input;
    step = 1;
    for(i = 0; i < num_cols; i++){
        wave_transfer_1d(num_rows,step,ptr,temp,h,g,len);
        pp = ptr;
        for( j = 0; j <num_rows; j++){
            *pp = temp[j];
            pp+=step;
        }
        ptr+= step_col;
    }


    /* first do transfer for rows */
    ptr = input;
    step = step_col; /* tricky here */
    for(i = 0; i < num_rows; i++){
        wave_transfer_1d(num_cols,step,ptr,temp,h,g,len);
        pp = ptr;
        for( j = 0; j <num_cols; j++){
            *pp = temp[j];
            pp+=step;
        }
        ptr += 1;
    }


    free(temp);
}

void wavelet_transfer_2d(int num_rows, int num_cols, float *input, int type, int level)
{
    int i = 0;
    int cnum_rows = num_rows, cnum_cols = num_cols;
    while(cnum_rows >= num_rows/pow(float(2), level-1) && cnum_cols >= num_cols/pow(float(2), level-1)){
		if(type == 1)
			wavelet_transfer_2d_sub(cnum_rows,cnum_cols,num_rows,input,h8,g8,8);
		else if(type == 2)
			wavelet_transfer_2d_sub(cnum_rows,cnum_cols,num_rows,input,h4,g4,4);
		else
			wavelet_transfer_2d_sub(cnum_rows,cnum_cols,num_rows,input,h2,g2,2);

        cnum_cols >>= 1;
        cnum_rows >>= 1;
    }
}

vf getDiagDetail(float *x_Coeff, int x_blkSize)
{
	int _coeffLength = x_blkSize * x_blkSize / 4;
	vf _detCoeff(_coeffLength);
	for (int i = 0; i < _coeffLength; i++)
	{
		_detCoeff[i] = x_Coeff[_coeffLength * 2 + x_blkSize / 2 + i % (x_blkSize / 2) + (i/(x_blkSize/2))*x_blkSize];
	}
	return _detCoeff;
}
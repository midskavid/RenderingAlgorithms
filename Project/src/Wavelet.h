#ifndef WAVELET_H
#define WAVELET_H

void wavelet_transfer_2d(int num_rows, int num_cols, float *input, int type, int i);
float* getDiagDetail(float *Coeff, int x_blkSize);

#endif // WAVELET_H
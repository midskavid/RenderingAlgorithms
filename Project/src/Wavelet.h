#ifndef WAVELET_H
#define WAVELET_H

void WaveletSampling(int num_rows, int num_cols, float *input);
float* DiagDetail(float *Coeff);

#endif // WAVELET_H
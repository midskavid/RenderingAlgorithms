#include <vector>

#include "Wavelet.h"

const std::vector<float> h{0.707106781186547f,0.707106781186547f};
const std::vector<float> g{-0.707106781186547f,0.707106781186547f};

void Wave1d(std::vector<float>& input, int idx, std::vector<float>& output, int step) {
    float *a = &input[idx];
    int n = 8;
    int half = 4;
    for (auto& o:output) o = 0;

    // referred from authors code and http://web.iitd.ac.in/~sumeet/WaveletTutorial.pdf
    for(int ii = 0; ii<half; ++ii) {
        int low = ii;
        int high = (ii+half)%half+half;

        for(int jj=0; jj<2; ++jj) {
            output[low] += h[jj] * a[((ii*2+jj)%n)*step];
            output[high] += g[jj] * a[((ii*2+jj)%n)*step];
        }

    }
}

void WaveletSampling(std::vector<float>& pixCol) {
    int _size = 8;
    std::vector<float> tmp(_size);

    for(int ii=0;ii<int(pixCol.size());ii+=8) {
        Wave1d(pixCol, ii, tmp, 1);
        for (int jj=0;jj<8;++jj) {
            pixCol[ii+jj] = tmp[jj];
        }
    }

    // Along columns
    for(int ii=0;ii<8;++ii) {
        Wave1d(pixCol,ii, tmp, 8);
        for(int jj=0;jj<8;++jj) {
            pixCol[ii+jj*8] = tmp[jj];
        }
    }
}

void DiagDetail(std::vector<float>& X, std::vector<float>& diagC) {
	for (int i = 0; i < 16; i++)
		diagC[i] = X[36+(i%4)+(i/4)*8];
}
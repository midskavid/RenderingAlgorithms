#include "Film.h"
#include "../FreeImage/FreeImage.h"

void Film::AddColor(Point2i pt, RGBColor color) {
    mFilm[pt[0]][pt[1]] = mFilm[pt[0]][pt[1]] + color;
}

void Film::WriteToImage(std::string fname) {
    FreeImage_Initialise();
    FIBITMAP* img = FreeImage_Allocate(mWidth,mHeight,24);
    RGBQUAD c;
    for(int jj=0;jj<mHeight;++jj) {
        auto& row = mFilm[jj];
        for(int ii=0;ii<mWidth;++ii) {
            c.rgbRed = row[ii].GetBlue()*255;
            c.rgbGreen = row[ii].GetGreen()*255;
            c.rgbBlue = row[ii].GetRed()*255;
            FreeImage_SetPixelColor(img, ii, jj, &c);
        }
    }
    FreeImage_Save(FIF_PNG, img, fname.c_str(),0);
}
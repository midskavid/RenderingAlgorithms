#ifndef FILM_H
#define FILM_H

#include "CoreTypes.h"
#include "Geometry.h"
#include "RGBColor.h"

class Film {
public:
    Film(int height, int width) : mHeight(height), mWidth(width){
        mFilm = std::vector<std::vector<RGBColor>> (mHeight, std::vector<RGBColor>(mWidth, RGBColor(0,0,0)));
    }
    void WriteToImage(std::string fname);
    void AddColor(Point2i pt, RGBColor color);
private:
    std::vector<std::vector<RGBColor>> mFilm;
    int mHeight;
    int mWidth;
};

#endif
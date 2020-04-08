#ifndef FILM_H
#define FILM_H

#include "CoreTypes.h"
#include "Geometry.h"

class Film {
public:
    Film(Bounds2i bounds);
    void WriteToImage(std::string fname);
    void AddColor(Point2i pt, PixelColor color);
private:
    std::vector<std::vector<PixelColor>> mfilm;
};

#endif
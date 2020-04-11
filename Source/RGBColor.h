#ifndef RGBCOLOR_H
#define RGBCOLOR_H

#include "CoreTypes.h"

class RGBColor {
public:
    RGBColor() : mR(0), mG(0), mB(0) { }
    RGBColor(Float r_, Float g_, Float b_) : mR(r_), mG(g_), mB(b_) { }

    Float GetRed() const {return mR;}
    Float GetGreen() const {return mG;}
    Float GetBlue() const {return mB;}

    RGBColor operator +(const RGBColor& col) {
        return RGBColor(mR+col.GetRed(), mG+col.GetGreen(), mB+col.GetBlue());
    }
    RGBColor operator -(const RGBColor& col) {
        return RGBColor(mR-col.GetRed(), mG-col.GetGreen(), mB-col.GetBlue());
    }
    RGBColor operator *(const RGBColor& col) {
        return RGBColor(mR*col.GetRed(), mG*col.GetGreen(), mB*col.GetBlue());
    }
    RGBColor operator *(Float s_) {
        return RGBColor(mR*s_, mG*s_, mB*s_);
    }

private:
    Float mR, mG, mB;
};

#endif
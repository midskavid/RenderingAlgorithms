#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "CoreTypes.h"
#include "RGBColor.h"
#include "Geometry.h"
#include "Interaction.h"

class LightSource {
protected:
    enum class LightType {
        kDefault,
        kPointLight,
        kDirectionalLight,
        kSurfaceLight
    };
public:
    LightSource(RGBColor color_) : mColor(color_), mLightType(LightType::kDefault) { }
    LightSource(RGBColor color_, LightType ltty_) : mColor(color_), mLightType(ltty_) { }
    RGBColor GetColor() const {return mColor;}
    virtual bool Unoccluded(Interaction& itr) const = 0;
    virtual Vector3f GetDirection(Point3f pt) const = 0;
    virtual Float GetAttenuation(Point3f pos) const = 0;
protected:
    RGBColor mColor; 
    LightType mLightType;    
};

class PointLightSource : public LightSource {
public:
    PointLightSource(RGBColor col_, Point3f pos_, std::array<Float,3>& att_) : LightSource(col_, LightType::kPointLight), mPosition(pos_), mAttenuation(att_) { }
    bool Unoccluded(Interaction& itr) const override;
    Vector3f GetDirection(Point3f pt) const override;
    Float GetAttenuation(Point3f pos) const override;
private:
    Point3f mPosition;
    std::array<Float,3> mAttenuation;
};

class DirectionalLightSource : public LightSource {
public:
    DirectionalLightSource(RGBColor col_, Vector3f dir, std::array<Float,3>& att_) : LightSource(col_, LightType::kDirectionalLight), mDirection(dir), mAttenuation(att_) { }
    bool Unoccluded(Interaction& itr) const override;
    Vector3f GetDirection(Point3f pt) const override;
    Float GetAttenuation(Point3f pos) const override;
private:
    std::array<Float,3> mAttenuation;
    Vector3f mDirection;    
};

#endif
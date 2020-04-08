#ifndef CORETYPES_H
#define CORETYPES_H

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <iterator>
#include <stack>
#include <deque>
#include <fstream>
#include <sstream>


#ifdef FLOAT_AS_DOUBLE
  typedef double Float;
#else
  typedef float Float;
#endif 

#define MaxFloat std::numeric_limits<Float>::max()
#define Infinity std::numeric_limits<Float>::infinity()

static constexpr Float Pi = 3.14159265358979323846;
static constexpr Float InvPi = 0.31830988618379067154;
static constexpr Float Inv2Pi = 0.15915494309189533577;


inline Float Radians(Float deg) { return (Pi / 180) * deg; }
inline Float Degrees(Float rad) { return (180 / Pi) * rad; }


template <typename T>
class Vector2;
template <typename T>
class Vector3;
template <typename T>
class Point3;
template <typename T>
class Point2;
template <typename T>
class Normal3;
class Ray;
template <typename T>
class Bounds2;
template <typename T>
class Bounds3;
class Transform;
class PixelColor;
class Shape;
class Film;

#endif
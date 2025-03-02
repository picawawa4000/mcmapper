#ifndef SPLINE_HPP
#define SPLINE_HPP

#include <vector>
#include <variant>
#include <array>
#include <mcmapper/util.hpp>

enum SplineType {
    CONTINENTALNESS = 0,
    EROSION = 1,
    RIDGES = 2,
    WEIRDNESS = 3,
};

/*
A spline curve is a way of fitting a curve to multiple points.
*/
struct Spline {
    std::vector<f32> locations; //locations of the points
    std::vector<std::variant<Spline, f32>> values; //values at the points
    std::vector<f32> derivatives; //slopes at the points
    SplineType type; //which input to sample (extremely useful for compound splines)

    explicit Spline(SplineType type = CONTINENTALNESS) : type(type) {}

    void add(f32 location, std::variant<Spline, f32> value, f32 derivative = 0.f) {
        this->locations.push_back(location);
        this->values.push_back(value);
        this->derivatives.push_back(derivative);
    }

    f32 sample(std::array<f32, 4> values) const;
};

//IMPORTANT NOTE: if amplified, apply transformation (value > 0 ? value * 2 : value) to the input
Spline createOffsetSpline();
Spline createFactorSpline();

// Please use these instead of the above ones.
// "Performance".
inline const std::shared_ptr<Spline> offsetSpline() {
    static std::shared_ptr<Spline> SPLINE = std::make_shared<Spline>(createOffsetSpline());
    return SPLINE;
}

inline const std::shared_ptr<Spline> factorSpline() {
    static std::shared_ptr<Spline> SPLINE = std::make_shared<Spline>(createFactorSpline());
    return SPLINE;
}

#endif
#ifndef DENSITYFUNCTION_MISC_HPP
#define DENSITYFUNCTION_MISC_HPP

#include <mcmapper/densityfunction/base.hpp>
#include <mcmapper/rng/noise.hpp>
#include <mcmapper/terrain/internoise.hpp>

typedef std::shared_ptr<DensityFunction> DFuncPtr;
typedef std::shared_ptr<DoublePerlinNoise> NoisePtr;

/// @brief A function that returns a constant value.
struct ConstantFunction : public DensityFunction {
    explicit ConstantFunction(double value);
    virtual double operator()(Pos3D pos);

private:
    double value;
};

/// @brief A function that computes the absolute value of its input.
struct AbsFunction : public DensityFunction {
    explicit AbsFunction(DFuncPtr target);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
};

/// @brief A function that squares its input.
struct SquareFunction : public DensityFunction {
    explicit SquareFunction(DFuncPtr target);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
};

/// @brief A function that cubes its input.
struct CubeFunction : public DensityFunction {
    explicit CubeFunction(DFuncPtr target);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
};

/// @brief A function that halves its input if it is negative, and
/// does nothing if it is positive.
struct HalfNegativeFunction : public DensityFunction {
    explicit HalfNegativeFunction(DFuncPtr target);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
};

/// @brief A function that quarters its input if it is negative, and
/// does nothing if it is positive.
struct QuarterNegativeFunction : public DensityFunction {
    explicit QuarterNegativeFunction(DFuncPtr target);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
};

/// @brief A function that clamps its input between -1 and 1 before
/// applying the transformation (x / 2) - (x * x * x / 24).
struct SqueezeFunction : public DensityFunction {
    explicit SqueezeFunction(DFuncPtr target);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
};

/// @brief Adds two density functions together.
struct AddFunction : public DensityFunction {
    AddFunction(DFuncPtr target1, DFuncPtr target2);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr a, b;
};

/// @brief Multiplies two density functions.
struct MulFunction : public DensityFunction {
    MulFunction(DFuncPtr target1, DFuncPtr target2);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr a, b;
};

/// @brief Takes the minimum of two density functions.
struct MinFunction : public DensityFunction {
    MinFunction(DFuncPtr target1, DFuncPtr target2);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr a, b;
};

/// @brief Takes the maximum of two density functions.
struct MaxFunction : public DensityFunction {
    MaxFunction(DFuncPtr target1, DFuncPtr target2);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr a, b;
};

/// @brief A noise function that uses shift and scale operations
/// before sampling the noise, and that does not use the Y-coordinate
/// of its input.
struct FlatScaledNoiseFunction : public DensityFunction {
    /// @brief Creates a new `FlatScaledNoiseFunction`.
    /// @param noise The noise to sample. The rationale for making
    /// this a `shared_ptr` instead of using an in-place construction
    /// is to allow for changing the seed without needing to
    /// reconstruct every single `DensityFunction`.
    /// @param shiftX The X shift to use on this function.
    /// @param shiftZ The Z shift to use on this function.
    /// @param xzScale The X-Z scale of the coordinates.
    FlatScaledNoiseFunction(NoisePtr noise, DFuncPtr shiftX, DFuncPtr shiftZ, f64 xzScale);
    virtual double operator()(Pos3D pos);

private:
    NoisePtr noise;
    DFuncPtr shiftX, shiftZ;
    f64 xzScale;
};

/// @brief A noise function that uses shift and scale operations
/// before sampling the noise. Likely to be unused in practice.
struct ShiftedNoiseFunction : public DensityFunction {
    ShiftedNoiseFunction(NoisePtr noise, DFuncPtr shiftX, DFuncPtr shiftY, DFuncPtr shiftZ, f64 xzScale, f64 yScale);
    virtual double operator()(Pos3D pos);

private:
    NoisePtr noise;
    DFuncPtr shiftX, shiftY, shiftZ;
    f64 xzScale, yScale;
};

/// @brief A simple noise funciton.
struct NoiseFunction : public DensityFunction {
    NoiseFunction(NoisePtr noise, f64 xzScale, f64 yScale);
    virtual double operator()(Pos3D pos);

private:
    NoisePtr noise;
    f64 xzScale, yScale;
};

/// @brief A noise function that samples the noise at (x/4, 0, z/4)
/// before multiplying the output by 4.
struct ShiftAFunction : public DensityFunction {
    explicit ShiftAFunction(NoisePtr noise);
    virtual double operator()(Pos3D pos);

private:
    NoisePtr noise;
};

/// @brief A noise function that samples the noise at (z/4, x/4, 0)
/// before multiplying the output by 4.
struct ShiftBFunction : public DensityFunction {
    explicit ShiftBFunction(NoisePtr noise);
    virtual double operator()(Pos3D pos);

private:
    NoisePtr noise;
};

/// @brief A noise function that samples the noise at (x/4, y/4, z/4)
/// before multiplying the output by 4.
struct ShiftFunction : public DensityFunction {
    explicit ShiftFunction(NoisePtr noise);
    virtual double operator()(Pos3D pos);

private:
    NoisePtr noise;
};

/// @brief A function that clamps the input in a specified range.
struct ClampFunction : public DensityFunction {
    ClampFunction(DFuncPtr target, double min, double max);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
    double min, max;
};

/// @brief A function that clamps the Y component to be between two
/// values, before linearly mapping it to a range.
struct YGradientFunction : public DensityFunction {
    YGradientFunction(i32 fromY, i32 toY, double fromValue, double toValue);
    virtual double operator()(Pos3D pos);

private:
    i32 fromY, toY;
    double fromValue, toValue;
};

/// @brief A function that selects one of two functions based on the
/// input of a selection function.
struct RangeChoiceFunction : public DensityFunction {
    RangeChoiceFunction(DFuncPtr select, DFuncPtr inRange, DFuncPtr outRange, double min, double max);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr select, inRange, outRange;
    double min, max;
};

/// @brief A function that scales a noise in certain places based on
/// its input.
struct WeirdScaledFunction : public DensityFunction {
    WeirdScaledFunction(DFuncPtr target, NoisePtr noise, bool usesTunnelMapper);
    virtual double operator()(Pos3D pos);

private:
    DFuncPtr target;
    NoisePtr noise;
    bool usesTunnelMapper;
};

/// @brief A noise function that uses `InterpolatedNoise`.
struct InterpolatedNoiseFunction : public DensityFunction {
    InterpolatedNoiseFunction(std::shared_ptr<InterpolatedNoise> noise);
    virtual double operator()(Pos3D pos);

private:
    std::shared_ptr<InterpolatedNoise> noise;
};

struct SplineFunction : public DensityFunction {
    SplineFunction(DFuncPtr input);
    void addPoint(float location, std::variant<float, SplineFunction> value, float derivative = 0.);
    virtual double operator()(Pos3D pos);

private:
    std::vector<float> locations;
    std::vector<std::variant<float, SplineFunction>> values;
    std::vector<float> derivatives;
    DFuncPtr input;
};

struct ErrorFunction : public DensityFunction {
    ErrorFunction(const std::string& msg);
    virtual double operator()(Pos3D pos);

private:
    std::string msg;
};

#endif
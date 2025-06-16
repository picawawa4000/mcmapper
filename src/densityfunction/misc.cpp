#include <mcmapper/densityfunction/misc.hpp>

ConstantFunction::ConstantFunction(double value) : value(value) {}

double ConstantFunction::operator()(Pos3D pos) {
    return this->value;
}

AbsFunction::AbsFunction(DFuncPtr target) : target(target) {}

double AbsFunction::operator()(Pos3D pos) {
    return std::abs(this->target->operator()(pos));
}

SquareFunction::SquareFunction(DFuncPtr target) : target(target) {}

double SquareFunction::operator()(Pos3D pos) {
    double d = this->target->operator()(pos);
    return d * d;
}

CubeFunction::CubeFunction(DFuncPtr target) : target(target) {}

double CubeFunction::operator()(Pos3D pos) {
    double d = this->target->operator()(pos);
    return d * d * d;
}

HalfNegativeFunction::HalfNegativeFunction(DFuncPtr target) : target(target) {}

double HalfNegativeFunction::operator()(Pos3D pos) {
    double d = this->target->operator()(pos);
    return d < 0. ? d / 2. : d;
}

QuarterNegativeFunction::QuarterNegativeFunction(DFuncPtr target) : target(target) {}

double QuarterNegativeFunction::operator()(Pos3D pos) {
    double d = this->target->operator()(pos);
    return d < 0. ? d / 4. : d;
}

SqueezeFunction::SqueezeFunction(DFuncPtr target) : target(target) {}

double SqueezeFunction::operator()(Pos3D pos) {
    double d = std::clamp(this->target->operator()(pos), -1., 1.);
    return (d / 2.) - (d * d * d / 24.);
}

AddFunction::AddFunction(DFuncPtr a, DFuncPtr b) : a(a), b(b) {}

double AddFunction::operator()(Pos3D pos) {
    return this->a->operator()(pos) + this->b->operator()(pos);
}

MulFunction::MulFunction(DFuncPtr a, DFuncPtr b) : a(a), b(b) {}

double MulFunction::operator()(Pos3D pos) {
    return this->a->operator()(pos) * this->b->operator()(pos);
}

MinFunction::MinFunction(DFuncPtr a, DFuncPtr b) : a(a), b(b) {}

double MinFunction::operator()(Pos3D pos) {
    return std::min(this->a->operator()(pos), this->b->operator()(pos));
}

MaxFunction::MaxFunction(DFuncPtr a, DFuncPtr b) : a(a), b(b) {}

double MaxFunction::operator()(Pos3D pos) {
    return std::max(this->a->operator()(pos), this->b->operator()(pos));
}

FlatScaledNoiseFunction::FlatScaledNoiseFunction(std::shared_ptr<DoublePerlinNoise> noise,  DFuncPtr shiftX,  DFuncPtr shiftZ, f64 xzScale): noise(noise), xzScale(xzScale), shiftX(shiftX), shiftZ(shiftZ) {}

double FlatScaledNoiseFunction::operator()(Pos3D pos) {
    double x = pos.x * this->xzScale + this->shiftX->operator()(pos);
    double z = pos.z * this->xzScale + this->shiftZ->operator()(pos);
    return this->noise->sample(x, 0., z);
}

ShiftedNoiseFunction::ShiftedNoiseFunction(NoisePtr noise, DFuncPtr shiftX, DFuncPtr shiftY, DFuncPtr shiftZ, f64 xzScale, f64 yScale) : noise(noise), shiftX(shiftX), shiftY(shiftY), shiftZ(shiftZ), xzScale(xzScale), yScale(yScale) {}

double ShiftedNoiseFunction::operator()(Pos3D pos) {
    double x = pos.x * this->xzScale + this->shiftX->operator()(pos);
    double y = pos.y * this->yScale + this->shiftY->operator()(pos);
    double z = pos.z * this->xzScale + this->shiftZ->operator()(pos);
    return this->noise->sample(x, y, z);
}

NoiseFunction::NoiseFunction(NoisePtr noise, f64 xzScale, f64 yScale) : noise(noise), xzScale(xzScale), yScale(yScale) {}

double NoiseFunction::operator()(Pos3D pos) {
    return this->noise->sample(pos.x * this->xzScale, pos.y * this->yScale, pos.z * this->xzScale);
}

ShiftAFunction::ShiftAFunction(NoisePtr noise) : noise(noise) {}

double ShiftAFunction::operator()(Pos3D pos) {
    return 4. * this->noise->sample(pos.x / 4., 0., pos.z / 4.);
}

ShiftBFunction::ShiftBFunction(NoisePtr noise) : noise(noise) {}

double ShiftBFunction::operator()(Pos3D pos) {
    return 4. * this->noise->sample(pos.z / 4., pos.x / 4., 0.);
}

ShiftFunction::ShiftFunction(NoisePtr noise) : noise(noise) {}

double ShiftFunction::operator()(Pos3D pos) {
    return 4. * this->noise->sample(pos.x / 4., pos.y / 4., pos.z / 4.);
}

ClampFunction::ClampFunction(DFuncPtr target, double min, double max) : target(target), min(min), max(max) {}

double ClampFunction::operator()(Pos3D pos) {
    return std::clamp(this->target->operator()(pos), min, max);
}

YGradientFunction::YGradientFunction(i32 fromY, i32 toY, double fromValue, double toValue) : fromY(fromY), toY(toY), fromValue(fromValue), toValue(toValue) {}

double YGradientFunction::operator()(Pos3D pos) {
    return yClampedGradient(pos.y, this->fromY, this->toY, this->fromValue, this->toValue);
}

RangeChoiceFunction::RangeChoiceFunction(DFuncPtr select, DFuncPtr inRange, DFuncPtr outRange, double min, double max) : select(select), inRange(inRange), outRange(outRange), min(min), max(max) {}

double RangeChoiceFunction::operator()(Pos3D pos) {
    double d = this->select->operator()(pos);
    return this->min <= d && d < this->max ? this->inRange->operator()(pos) : this->outRange->operator()(pos);
}

WeirdScaledFunction::WeirdScaledFunction(DFuncPtr target, NoisePtr noise, bool usesTunnelMapper) : target(target), noise(noise), usesTunnelMapper(usesTunnelMapper) {}

double WeirdScaledFunction::operator()(Pos3D pos) {
    double scale;
    double value = this->target->operator()(pos);
    if (this->usesTunnelMapper) {
        if (value < -0.5) scale = 0.75;
        else if (value < 0.) scale = 1.;
        else if (value < 0.5) scale = 1.5;
        else scale = 2.;
    } else {
        if (value < -0.75) scale = 0.5;
        else if (value < -0.5) scale = 0.75;
        else if (value < 0.5) scale = 1.;
        else if (value < 0.75) scale = 2.;
        else scale = 3.;
    }
    return scale * std::abs(this->noise->sample((double)pos.x / scale, (double)pos.y / scale, (double)pos.z / scale));
}

InterpolatedNoiseFunction::InterpolatedNoiseFunction(std::shared_ptr<InterpolatedNoise> noise) : noise(noise) {}

double InterpolatedNoiseFunction::operator()(Pos3D pos) {
    return this->noise->sample(pos.x, pos.y, pos.z);
}

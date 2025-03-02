#include <mcmapper/terrain/spline.hpp>

f32 getOffsetValue(f32 weirdness, f32 continentalness) {
    f32 f0 = 1.0F - (1.0F - continentalness) * 0.5F;
    f32 f1 = 0.5F * (1.0F - continentalness);
    f32 f2 = (weirdness + 1.17F) * 0.46082947F;
    f32 off = f2 * f0 - f1;
    if (weirdness < -0.7F) return off > -0.2222F ? off : -0.2222F;
    else return off > 0 ? off : 0;
}

f32 sampleVariant(const std::variant<Spline, f32>& variant, std::array<f32, 4> values) {
    if (std::holds_alternative<Spline>(variant)) {
        return std::get<Spline>(variant).sample(values);
    } else if (std::holds_alternative<f32>(variant)) {
        return std::get<f32>(variant);
    } else throw std::runtime_error("variant didn't contain value!");
}

f32 Spline::sample(std::array<f32, 4> values) const {
    if (this->locations.size() != this->values.size()) throw std::runtime_error("spline: locations.size() != values.size()!");
    if (this->locations.size() != this->derivatives.size()) throw std::runtime_error("spline: locations.size() != derivatives.size()!");

    f32 f = values[this->type];

    i32 i;
    for (i = 0; i < this->locations.size(); ++i) if (this->locations[i] >= f) break;

    if (i == 0 || i == this->locations.size()) {
        if (i != 0) --i;
        f32 v = sampleVariant(this->values[i], values);
        return v + this->derivatives[i] * (f - this->locations[i]);
    }
    std::variant<Spline, f32> sp1 = this->values[i-1];
    std::variant<Spline, f32> sp2 = this->values[i];
    f32 g = this->locations[i-1];
    f32 h = this->locations[i];
    f32 k = (f - g) / (h - g);
    f32 l = this->derivatives[i-1];
    f32 m = this->derivatives[i];
    f32 n = sampleVariant(sp1, values);
    f32 o = sampleVariant(sp2, values);
    f32 p = l * (h - g) - (o - n);
    f32 q = -m * (h - g) + (o - n);
    return lerp(k, n, o) + k * (1.F - k) * lerp(k, p, q);
}

f32 getOffsetValue(f32 f, f32 g, f32 h) {
    f32 i = (1.f - g) * 0.5f;
    f32 j = 1.f - i;
    f32 k = (f + 1.17f) * 0.46082947f;
    f32 l = k * j - i;
    if (f < h) return std::max(l, -0.2222f);
    return std::max(l, 0.f);
}

f32 getOffsetValue2(f32 f) {
    f32 i = 1.f - (1.f - f) * 0.5f;
    f32 j = 0.5f * (1.f - f);
    return j / (0.46082947f * i) - 1.17f;
}

f32 riseOverRun(f32 y1, f32 y2, f32 x1, f32 x2) {
    return (y2 - y1) / (x2 - x1);
}

Spline createSpline1(float f, bool bl) {
    Spline out(RIDGES);
    f32 g = getOffsetValue(-1.f, f, -0.7f);
    f32 h = getOffsetValue(1.f, f, -0.7f);
    f32 i = getOffsetValue2(f);
    if (-0.65f < i and i < 1.f) {
        f32 j = getOffsetValue(-0.65f, f, -0.7f);
        f32 k = getOffsetValue(-0.75f, f, -0.7f);
        f32 l = riseOverRun(g, k, -1.f, -0.75f);
        out.add(-1.f, g, l);
        out.add(-0.75f, k);
        out.add(-0.65f, j);
        f32 m = getOffsetValue(i, f, -0.7f);
        f32 n = riseOverRun(m, h, i, 1.f);
        out.add(i - 0.01f, m);
        out.add(i, m, n);
        out.add(1.f, h, n);
    } else {
        f32 n = riseOverRun(g, h, -1.f, 1.f);
        if (bl) {
            out.add(-1.f, std::max(0.2f, g));
            out.add(0.f, (f32)lerp(0.5f, g, h), n);
        } else {
            out.add(-1.f, g, n);
        }
        out.add(1.f, h, n);
    }
    return out;
}

Spline createFlatOffsetSpline(f32 continentalness, f32 g, f32 h, f32 i, f32 j, f32 k) {
    Spline out(RIDGES);
    f32 l = std::max(0.5f * (g - continentalness), k);
    f32 m = 5.f * (h - g);
    out.add(-1.f, continentalness, l);
    out.add(-0.4f, g, std::min(l, m));
    out.add(0.f, h, m);
    out.add(0.4f, i, 2.f * (i - h));
    out.add(1.f, j, 0.7f * (j - i));
    return out;
}

Spline createContinentalOffsetSpline(f32 continentalness, f32 a, f32 b, f32 c, f32 d, f32 e, bool bl) {
    Spline sp1 = createSpline1(lerp(c, 0.6f, 1.5f), bl);
    Spline sp2 = createSpline1(lerp(c, 0.6f, 1.5f), bl);
    Spline sp3 = createSpline1(c, bl);

    Spline sp4 = createFlatOffsetSpline(continentalness - 0.15f, 0.5f * c, lerp(0.5f, 0.5f, 0.5f) * c, 0.5f * c, 0.5f * c, 0.5f);
    Spline sp5 = createFlatOffsetSpline(continentalness, d * c, a * c, 0.5f * c, 0.6f * c, 0.5f);
    Spline sp6 = createFlatOffsetSpline(continentalness, d, d, a, b, 0.5f);
    Spline sp7 = createFlatOffsetSpline(continentalness, d, d, a, b, 0.5f);
    Spline sp8 = createFlatOffsetSpline(-0.02f, e, e, a, b, 0.f);

    Spline sp9(RIDGES);
    sp9.add(-1.f, continentalness);
    sp9.add(-0.4f, sp6);
    sp9.add(0.f, b + 0.07f);

    Spline out(EROSION);
    out.add(-0.85f, sp1);
    out.add(-0.7f, sp2);
    out.add(-0.4f, sp3);
    out.add(-0.35f, sp4);
    out.add(-0.1f, sp5);
    out.add(0.2f, sp6);
    if (bl) {
        out.add(0.4f, sp7);
        out.add(0.45f, sp9);
        out.add(0.55f, sp9);
        out.add(0.58f, sp7);
    }
    out.add(0.7f, sp8);

    return out;
}

Spline createOffsetSpline() {
    Spline out(CONTINENTALNESS);
    out.add(-1.1f, 0.044f);
    out.add(-1.02f, -0.2222f);
    out.add(-0.51f, -0.2222f);
    out.add(-0.44f, -0.12f);
    out.add(-0.18f, -0.12f);

    Spline sp1 = createContinentalOffsetSpline(-0.15f, 0.f, 0.f, 0.1f, 0.f, -0.03f, false);
    Spline sp2 = createContinentalOffsetSpline(-0.1f, 0.03f, 0.1f, 0.1f, 0.01f, -0.03f, false);
    Spline sp3 = createContinentalOffsetSpline(-0.1f, 0.03f, 0.1f, 0.7f, 0.01f, -0.03f, true);
    Spline sp4 = createContinentalOffsetSpline(-0.05f, 0.03f, 0.1f, 1.0f, 0.01f, 0.01f, true);

    out.add(-0.16f, sp1);
    out.add(-0.15f, sp1);
    out.add(-0.1f, sp2);
    out.add(0.25f, sp3);
    out.add(1.0f, sp4);
    return out;
}

Spline createFactorErosionSpline(f32 f, bool bl) {
    Spline sp(WEIRDNESS);
    sp.add(-0.2f, 6.3f);
    sp.add(0.2f, f);

    Spline sp2(WEIRDNESS);
    sp2.add(-0.05f, 6.3f);
    sp2.add(0.05f, 2.67f);
    
    Spline sp3(WEIRDNESS);
    sp3.add(-0.05f, 2.67f);
    sp3.add(0.05f, 6.3f);

    Spline sp4(EROSION);
    sp4.add(-0.6f, sp);
    sp4.add(-0.5f, sp2);
    sp4.add(-0.35f, sp);
    sp4.add(-0.25f, sp);
    sp4.add(-0.1f, sp3);
    sp4.add(0.03f, sp);

    if (bl) {
        Spline sp5(WEIRDNESS);
        sp5.add(0.0f, f);
        sp5.add(0.1f, 0.625f);

        Spline sp6(RIDGES);
        sp6.add(-0.9f, f);
        sp6.add(-0.69f, sp5);

        sp4.add(0.35f, f);
        sp4.add(0.45f, sp6);
        sp4.add(0.55f, sp6);
        sp4.add(0.62f, f);
    } else {
        Spline sp5(RIDGES);
        sp5.add(-0.7f, sp);
        sp5.add(-0.15f, 1.37f);
        
        Spline sp6(RIDGES);
        sp6.add(0.45f, sp);
        sp6.add(0.7f, 1.56f);

        sp4.add(0.05f, sp6);
        sp4.add(0.4f, sp6);
        sp4.add(0.45f, sp5);
        sp4.add(0.55f, sp5);
        sp4.add(0.58f, f);
    }

    return sp4;
}

Spline createFactorSpline() {
    Spline out(CONTINENTALNESS);
    out.add(-0.19f, 3.95f);
    //TODO for later: this is incorrect for amplified = true. This spline should have the identity applied, not the factor amplified scaler.
    //fixing this will probably require a large-scale rework of the spline system though, and I don't really want to do that now.
    //(see VanillaTerrainParametersCreator$createFactorSpline)
    out.add(-0.15f, createFactorErosionSpline(6.25f, true));
    //this spline is correct this way though
    out.add(-0.1f, createFactorErosionSpline(5.47f, true));
    out.add(0.03f, createFactorErosionSpline(5.08f, true));
    out.add(0.06f, createFactorErosionSpline(4.69f, false));
    return out;
}

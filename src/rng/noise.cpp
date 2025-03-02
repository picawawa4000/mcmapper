#include <mcmapper/rng/noise.hpp>

i32 gradients[16][3] = {
    {1, 1, 0}, 
    {-1, 1, 0}, 
    {1, -1, 0}, 
    {-1, -1, 0}, 
    {1, 0, 1}, 
    {-1, 0, 1}, 
    {1, 0, -1},
    {-1, 0, -1}, 
    {0, 1, 1}, 
    {0, -1, 1}, 
    {0, 1, -1}, 
    {0, -1, -1}, 
    {1, 1, 0}, 
    {0, -1, 1}, 
    {-1, 1, 0}, 
    {0, -1, -1}
};

f64 grad(i32 hash, f64 x, f64 y, f64 z) {
    i32 * gradient = gradients[hash & 0xF];
    return gradient[0] * x + gradient[1] * y + gradient[2] * z;
}

PerlinNoise::PerlinNoise(Random& rng) {
    this->originX = rng.next_f64() * 256.;
    this->originY = rng.next_f64() * 256.;
    this->originZ = rng.next_f64() * 256.;

    for (i32 i = 0; i < 256; ++i) {
        permutation[i] = i;
    }
    for (i32 i = 0; i < 256; ++i) {
        i32 idx = rng.next_i32(256 - i);
        u8 value = this->permutation[i];
        this->permutation[i] = this->permutation[i + idx];
        this->permutation[i + idx] = value;
    }
}

f64 PerlinNoise::sample(f64 x, f64 y, f64 z, f64 yScale, f64 yMax) const {
    f64 s;
    f64 i = x + this->originX;
    f64 j = y + this->originY;
    f64 k = z + this->originZ;
    i32 l = std::floor(i);
    i32 m = std::floor(j);
    i32 n = std::floor(k);
    f64 o = i - (f64)l;
    f64 p = j - (f64)m;
    f64 q = k - (f64)n;
    if (yScale != 0.0) {
        f64 r = yMax >= 0.0 && yMax < p ? yMax : p;
        s = (f64)std::floor(r / yScale + (f64)1.0E-7f) * yScale;
    } else {
        s = 0.0;
    }
    return this->sample(l, m, n, o, p - s, q, p);
}

f64 PerlinNoise::sample(i32 sectionX, i32 sectionY, i32 sectionZ, f64 localX, f64 localY, f64 localZ, f64 fadeLocalY) const {
    i32 l = this->map(sectionX);
    i32 m = this->map(sectionX + 1);
    i32 n = this->map(l + sectionY);
    i32 o = this->map(l + sectionY + 1);
    i32 p = this->map(m + sectionY);
    i32 q = this->map(m + sectionY + 1);
    f64 h = grad(this->map(n + sectionZ), localX, localY, localZ);
    f64 r = grad(this->map(p + sectionZ), localX - 1.0, localY, localZ);
    f64 s = grad(this->map(o + sectionZ), localX, localY - 1.0, localZ);
    f64 t = grad(this->map(q + sectionZ), localX - 1.0, localY - 1.0, localZ);
    f64 u = grad(this->map(n + sectionZ + 1), localX, localY, localZ - 1.0);
    f64 v = grad(this->map(p + sectionZ + 1), localX - 1.0, localY, localZ - 1.0);
    f64 w = grad(this->map(o + sectionZ + 1), localX, localY - 1.0, localZ - 1.0);
    f64 x = grad(this->map(q + sectionZ + 1), localX - 1.0, localY - 1.0, localZ - 1.0);
    f64 y = perlinFade(localX);
    f64 z = perlinFade(fadeLocalY);
    f64 aa = perlinFade(localZ);
    return lerp3(y, z, aa, h, r, s, t, u, v, w, x);
}

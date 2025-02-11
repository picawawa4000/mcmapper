#ifndef NOISE_HPP
#define NOISE_HPP

#include <mcmapper/rng.hpp>
#include <cmath>
#include <vector>
#include <memory>

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

struct PerlinNoise {
    f64 originX, originY, originZ;
    u8 permutation[256];

    PerlinNoise(Random& rng) {
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

    f64 sample(f64 x, f64 y, f64 z, f64 yScale = 0, f64 yMax = 0) const {
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

private:
    u8 map(i32 input) const {return this->permutation[input & 0xFF] & 0xFF;}

    f64 sample(i32 sectionX, i32 sectionY, i32 sectionZ, f64 localX, f64 localY, f64 localZ, f64 fadeLocalY) const {
        i32 l = this->map(sectionX);
        i32 m = this->map(sectionX + 1);
        i32 n = this->map(l + sectionY);
        i32 o = this->map(l + sectionY + 1);
        i32 p = this->map(m + sectionY);
        i32 q = this->map(m + sectionY + 1);
        //std::cout << l << ", " << m << ", " << n << ", " << o << ", " << p << ", " << q << std::endl;
        f64 h = grad(this->map(n + sectionZ), localX, localY, localZ);
        f64 r = grad(this->map(p + sectionZ), localX - 1.0, localY, localZ);
        f64 s = grad(this->map(o + sectionZ), localX, localY - 1.0, localZ);
        f64 t = grad(this->map(q + sectionZ), localX - 1.0, localY - 1.0, localZ);
        f64 u = grad(this->map(n + sectionZ + 1), localX, localY, localZ - 1.0);
        f64 v = grad(this->map(p + sectionZ + 1), localX - 1.0, localY, localZ - 1.0);
        f64 w = grad(this->map(o + sectionZ + 1), localX, localY - 1.0, localZ - 1.0);
        f64 x = grad(this->map(q + sectionZ + 1), localX - 1.0, localY - 1.0, localZ - 1.0);
        //std::cout << h << ", " << r << ", " << s << ", " << t << ", " << u << ", " << v << ", " << w << ", " << x << std::endl;
        f64 y = perlinFade(localX);
        f64 z = perlinFade(fadeLocalY);
        f64 aa = perlinFade(localZ);
        //std::cout << y << ", " << z << ", " << aa << std::endl;
        return lerp3(y, z, aa, h, r, s, t, u, v, w, x);
    }
};

//pre-calculated tables to make things just a bit faster

const u64 md5_hashes[][2] = {
    {0xb198de63a8012672, 0x7b84cad43ef7b5a8}, // md5 "octave_-12"
    {0x0fd787bfbc403ec3, 0x74a4a31ca21b48b8}, // md5 "octave_-11"
    {0x36d326eed40efeb2, 0x5be9ce18223c636a}, // md5 "octave_-10"
    {0x082fe255f8be6631, 0x4e96119e22dedc81}, // md5 "octave_-9"
    {0x0ef68ec68504005e, 0x48b6bf93a2789640}, // md5 "octave_-8"
    {0xf11268128982754f, 0x257a1d670430b0aa}, // md5 "octave_-7"
    {0xe51c98ce7d1de664, 0x5f9478a733040c45}, // md5 "octave_-6"
    {0x6d7b49e7e429850a, 0x2e3063c622a24777}, // md5 "octave_-5"
    {0xbd90d5377ba1b762, 0xc07317d419a7548d}, // md5 "octave_-4"
    {0x53d39c6752dac858, 0xbcd1c5a80ab65b3e}, // md5 "octave_-3"
    {0xb4a24d7a84e7677b, 0x023ff9668e89b5c4}, // md5 "octave_-2"
    {0xdffa22b534c5f608, 0xb9b67517d3665ca9}, // md5 "octave_-1"
    {0xd50708086cef4d7c, 0x6e1651ecc7f43309}, // md5 "octave_0"
};
const f64 lacunarity_init[] =   {1, .5, .25,  1./8, 1./16, 1./32,  1./64,  1./128,  1./256,   1./512,   1./1024, 1./2048, 1./4096};
const f64 persistance_init[] =  {0, 1,  2./3, 4./7, 8./15, 16./31, 32./63, 64./127, 128./255, 256./511};

struct OctavePerlinNoise {
    std::vector<std::shared_ptr<PerlinNoise>> octaves;
    std::vector<f64> amplitudes;
    f64 lacunarity;
    f64 persistance;

    OctavePerlinNoise(XoroshiroRandom& rng, std::vector<f64> amplitudes, i32 firstOctave) : amplitudes(amplitudes) {
        i32 size = amplitudes.size();
        this->lacunarity = lacunarity_init[-firstOctave];
        this->persistance = persistance_init[size];
        u64 temp_lo = rng.next_u64();
        u64 temp_hi = rng.next_u64();
        int n = 0;

        for (int i = 0; i < size; i++) {
            if (amplitudes[i] == 0) {
                this->octaves.push_back(nullptr);
            } else {
                XoroshiroRandom temp(temp_lo ^ md5_hashes[12 + firstOctave + i][0], temp_hi ^ md5_hashes[12 + firstOctave + i][1]);
                this->octaves.push_back(std::shared_ptr<PerlinNoise>(new PerlinNoise(temp)));
            }
            n++;
        }
    }

    OctavePerlinNoise(CheckedRandom& rng, std::vector<f64> amplitudes, i32 firstOctave) : amplitudes(amplitudes) {
        i32 i;
        i32 size = amplitudes.size();
        i32 lastOctave = firstOctave + size - 1;
        this->persistance = 1. / ((1LL << size) - 1.);
        this->lacunarity = 1LL << lastOctave;

        if (lastOctave == 0) {
            this->octaves.push_back(std::shared_ptr<PerlinNoise>(new PerlinNoise(rng)));
            i = 1;
        } else {
            rng.skip(-lastOctave * 262);
            i = 0;
        }

        for (; i < size; ++i) {
            if (amplitudes[i] != 0.) this->octaves.push_back(std::shared_ptr<PerlinNoise>(new PerlinNoise(rng)));
            else this->octaves.push_back(nullptr);
        }
    }

    OctavePerlinNoise(){}

    f64 sample(f64 x, f64 y, f64 z) const {
        f64 ret = 0.0;
        f64 lac = this->lacunarity;
        f64 per = this->persistance;
        //std::cout << "oct: " << this->octaves.size() << std::endl;
        for (i32 i = 0; i < this->octaves.size(); ++i) {
            if (this->octaves[i] != nullptr) {
                //std::cout << "pln: " << this->octaves[i]->sample(x, y, z) << std::endl;
                ret += per * this->amplitudes[i] * this->octaves[i]->sample(x * lac, y * lac, z * lac);
            }
            lac *= 2.;
            per /= 2.;
            //std::cout << "oct: " << ret << ", " << lac << ", " << per << std::endl;
        }
        return ret;
    }
};

const f64 amplitude_init[] = {0, 5./6, 10./9, 15./12, 20./15, 25./18, 30./21, 35./24, 40./27, 45./30};
const f64 multiplier = 337. / 331.;

struct DoublePerlinNoise {
    f64 amplitude;
    OctavePerlinNoise first, second;

    DoublePerlinNoise(CheckedRandom& rng, std::vector<f64> amplitudes, i32 firstOctave) : first(rng, amplitudes, firstOctave), second(rng, amplitudes, firstOctave) {this->construct(amplitudes);}
    DoublePerlinNoise(XoroshiroRandom& rng, std::vector<f64> amplitudes, i32 firstOctave) : first(rng, amplitudes, firstOctave), second(rng, amplitudes, firstOctave) {this->construct(amplitudes);}
    DoublePerlinNoise(){};

    f64 sample(f64 x, f64 y, f64 z) const {
        f64 i = this->first.sample(x, y, z);
        f64 j = this->second.sample(x * multiplier, y * multiplier, z * multiplier);
        //std::cout << "dbl:" << i << ", " << j << std::endl;
        return (i + j) * this->amplitude;
    }

private:
    void construct(std::vector<f64> amplitudes) {
        i32 size = amplitudes.size();
        for (i32 i = size - 1; i >= 0 && amplitudes[i] == 0.0; i--) size--;
        for (i32 i = 0; amplitudes[i] == 0.0; i++) size--;
        this->amplitude = amplitude_init[size];
    }
};

#endif
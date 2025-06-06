#ifndef RNG_HPP
#define RNG_HPP

#include <mcmapper/util.hpp>
#include <cmath>

//Minecraft has two pseudo-random number generators, both of which are implemented here.

// Note to self: this interface is actually somewhat useless and can probably be removed for a performance boost.
struct Random {
    virtual void setSeed(u64 seed) = 0;
    virtual i64 next_i64() = 0;
    virtual i32 next_i32(i32 bound) = 0;
    virtual f64 next_f64() = 0;
    virtual f32 next_f32() = 0;
    virtual bool next_bool() = 0;
    virtual void skip(i32 count) = 0;
};

struct XoroshiroRandom : public Random {
    u64 hi;
    u64 lo;

    XoroshiroRandom(u64 lo, u64 hi) {
        this->hi = hi;
        this->lo = lo;
        if ((hi | lo) == 0) {
            this->lo = (u64)(-7046029254386353131);
            this->hi = 7640891576956012809;
        }
    }

    XoroshiroRandom(u64 seed) {
        this->setSeed(seed);
    }

    virtual void setSeed(u64 seed) {
        const u64 XL = 0x9e3779b97f4a7c15ULL;
        const u64 XH = 0x6a09e667f3bcc909ULL;
        const u64 A = 0xbf58476d1ce4e5b9ULL;
        const u64 B = 0x94d049bb133111ebULL;
        u64 l = seed ^ XH;
        u64 h = l + XL;
        l = (l ^ (l >> 30)) * A;
        h = (h ^ (h >> 30)) * A;
        l = (l ^ (l >> 27)) * B;
        h = (h ^ (h >> 27)) * B;
        l = l ^ (l >> 31);
        h = h ^ (h >> 31);
        this->lo = l;
        this->hi = h;
    }

    u64 next_u64() {
        u64 lo = this->lo;
        u64 hi = this->hi;
        u64 i = rol(lo + hi, 17) + lo;
        hi ^= lo;
        this->lo = rol(lo, 49) ^ hi ^ (hi << 21);
        this->hi = rol(hi, 28);
        return i;
    }

    virtual i32 next_i32(i32 bound) {
        u64 r = (this->next_u64() & 0xFFFFFFFF) * bound;
        if ((u32)r < bound) while ((u32)r < (~bound + 1) % bound) r = (this->next_u64() & 0xFFFFFFFF) * bound;
        return r >> 32;
    }

    virtual f64 next_f64() {
        return (this->next_u64() >> 11) * 1.1102230246251565E-16;
    }

    virtual f32 next_f32() {
        return (this->next_u64() >> 40) * 5.9604645E-8F;
    }

    virtual void skip(i32 count) {
        for (i32 i = 0; i < count; ++i) this->next_u64();
    }

    virtual bool next_bool() {
        return (this->next_u64() & 1) == 1;
    }

    //convert u64 to i64 using two's complement
    virtual i64 next_i64() {
        throw std::runtime_error("shouldn't need XoroshiroRandom::next_i64!");
    }
};

struct CheckedRandom : public Random {
    u64 seed;

    CheckedRandom(u64 seed) {
        this->setSeed(seed);
    }

    virtual void setSeed(u64 seed) {
        this->seed = (seed ^ 0x5deece66d) & ((1ULL << 48) - 1);
    }

    i32 next(i32 bits) {
        this->seed = (this->seed * 0x5deece66d + 11) & ((1ULL << 48) - 1);
        return (i32)(this->seed >> (48 - bits));
    }

    virtual i32 next_i32(i32 bound) {
        // if bound is a power of two
        if (((bound - 1) & bound) == 0) {
            return (i32)(((u64)bound * (u64)this->next(31)) >> 31);
        }
        i32 j, k;
        do {
            j = this->next(31);
            k = j % bound;
        } while (j - k + bound - 1 < 0);
        return k;
    }

    virtual i64 next_i64() {
        return ((i64)this->next(32) << 32) + (i64)this->next(32);
    }

    virtual f32 next_f32() {
        return (f32)this->next(24) * 5.9604645E-8f;
    }

    virtual f64 next_f64() {
        i32 i = this->next(26);
        i32 j = this->next(27);
        i64 k = ((i64)i << 27) + (i64)j;
        return (f64)k * (f64)1.110223E-16f;
    }

    virtual bool next_bool() {
        return this->next(1) != 0;
    }

   virtual void skip(i32 count) {
        for (i32 i = 0; i < count; ++i) this->next(32);
    }
};

XoroshiroRandom split(i64 seedLo, i64 seedHi, i32 blockX, i32 blockY, i32 blockZ);
bool isSlimeChunk(i64 worldSeed, i32 chunkX, i32 chunkZ);

#endif
#include <mcmapper/rng/noise.hpp>

#include <iostream>

int main() {
    XoroshiroRandom rng(3447);
    PerlinNoise noise(rng);

    std::cout << "permutation = [";
    for (int i = 0; i < 256; ++i) std::cout << std::hex << +noise.permutation[i] << ", ";
    std::cout << "]" << std::endl;

    return 0;
}
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <set>

#include <mcmapper.hpp>
#include <structure.hpp>

int main() {
    XoroshiroRandom rng(25125321);
    PerlinNoise noise(rng);

    for (int i = 0; i < 16; ++i) for (int j = 0; j < 16; ++j) std::cout << noise.sample(i, 0, j) << std::endl;

    return 0;
}
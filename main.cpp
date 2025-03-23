#include <mcmapper/rng/noise.hpp>

#include <sstream>
#include <iostream>

int main() {
    uint64_t data = (0x80ULL << 56) - 1;
    int64_t signedData = data;
    std::cout << signedData << std::endl << (int64_t)++signedData << std::endl;

    return 0;
}
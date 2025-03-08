#include <mcmapper/rng/rng.hpp>

#include <iostream>

int main() {
    XoroshiroRandom rng(3447);

    std::cout << rng.next_u64() << std::endl;
    std::cout << rng.next_u64() << std::endl;

    return 0;
}
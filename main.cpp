#include <mcmapper/rng/rng.hpp>
#include <iostream>

int main() {
    XoroshiroRandom rng(14759979931341644915ULL, 310579217366ULL);
    std::cout << rng.lo << ", " << rng.hi << std::endl;
    std::cout << rng.next_u64() << std::endl;
    std::cout << rng.next_u64() << std::endl;

    return 0;
}
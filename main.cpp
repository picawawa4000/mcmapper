#include <mcmapper/rng.hpp>

#include <iostream>

int main() {
    XoroshiroRandom rng(537207492169ULL);
    std::cout << rng.next_u64() << std::endl;
    std::cout << rng.next_u64() << std::endl;
    return 0;
}
#include <mcmapper/structure/gen/mansion.hpp>

int main() {
    XoroshiroRandom rng(3447ULL);
    MansionLayout layout(rng);
    
    return 0;
}
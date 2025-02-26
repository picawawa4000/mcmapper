#include <mcmapper/builtin_loot.hpp>

#include <iostream>

int main() {
    // expected: 6
    CheckedRandom random(134598025224048LL);
    std::cout << random.next_i32(8) << std::endl;
}
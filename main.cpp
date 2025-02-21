#include <mcmapper/util.hpp>

#include <iostream>

inline u64 ror(u64 input, u32 shift) {
    if ((shift &= 63) == 0) return input;
    return (input >> shift) | (input << (64 - shift));
}

int main() {
    u64 dummy = 0b1001100010010101001010100101010100101010010101010010100101010010ULL;

    for (int i = 0; i < 64; ++i)
        std::cout << ror(dummy, i) << ": " << rol(dummy, 64 - i) << std::endl;

    return 0;
}
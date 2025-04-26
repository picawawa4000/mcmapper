#include <mcmapper/structure/gen/mansion.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

int main() {
    const u64 mansionSeed = 3447;
    CheckedRandom rng(mansionSeed);

    MansionLayout layout(rng);

    std::ofstream outfile("mansion_gen_results.log");
    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
            outfile << std::dec << "[" << i << ", " << j << "] " << std::hex << layout.firstFloor.get(i, j) << std::endl;
        }
        outfile << "\n";
    }
    outfile.close();

    return 0;
}
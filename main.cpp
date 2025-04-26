#include <mcmapper/structure/gen/mansion.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

void printMatrix(std::ofstream& file, BaseMansionFlags& matrix) {
    for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
            file << std::dec << "[" << i << ", " << j << "] " << std::hex << matrix.get(i, j) << std::endl;
        }
        file << "\n";
    }
}

int main() {
    const u64 mansionSeed = 3447;
    CheckedRandom rng(mansionSeed);

    MansionLayout layout(rng);

    std::ofstream outfile("mansion_gen_results.log");

    outfile << "BASE LAYOUT\n" << std::endl;
    printMatrix(outfile, layout.baseLayout);

    outfile << "\nFIRST FLOOR ROOMS\n" << std::endl;
    printMatrix(outfile, layout.firstFloor);

    outfile << "\nSECOND FLOOR ROOMS\n" << std::endl;
    printMatrix(outfile, layout.secondFloor);

    outfile << "\nTHIRD FLOOR LAYOUT\n" << std::endl;
    printMatrix(outfile, layout.thirdFloorLayout);

    outfile << "\nTHIRD FLOOR ROOMS\n" << std::endl;
    printMatrix(outfile, layout.thirdFloor);

    outfile.close();

    return 0;
}
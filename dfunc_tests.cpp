#include <mcmapper/datapack/loader.hpp>

#include <iostream>

int main() {
    DFuncGenInfoCache cache{
        .useCaches = false,
        .horizontalCellBlockCount = 4,
        .verticalCellBlockCount = 8
    };
    DFuncPtr func = loadDensityFunction("test_dfunc.json", "test:test_dfunc", cache);
    std::cout << "loaded func" << std::endl;

    std::cout << "func at (0, -64, 0) is " << func->operator()({0, -64, 0}) << std::endl;
    std::cout << "func at (0, 64, 0) is " << func->operator()({0, 64, 0}) << std::endl;
    std::cout << "func at (0, 0, 0) is " << func->operator()({0, 0, 0}) << std::endl;
    std::cout << "func at (0, 128, 0) is " << func->operator()({0, 128, 0}) << std::endl;

    return 0;
}
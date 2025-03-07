#include <mcmapper/misc/generator.hpp>

#include <iostream>

int main() {
#if __cpp_modules >= 201810L
    std::cout << "Modules!";
#else
    std::cout << "No modules!";
#endif

    return 0;
}
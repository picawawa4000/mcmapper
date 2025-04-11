#include <mcmapper/terrain/internoise.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

int main() {
    InterpolatedNoise noise(0.25, 0.125, 80.0, 160.0, 8.0);

    std::ofstream stream("interpolated_noise_results.log");

    for (int x = 16; x > 0; --x)
        for (int z = 16; z > 0; --z)
            for (int y = 256; y > 0; --y)
                stream << "[" << x << ", " << z << ", " << y << "] " << noise.sample(x, y, z) << "\n";
    
    stream.flush();
    stream.close();

    return 0;
}
cmake_minimum_required(VERSION 3.15)
project(mcmapper)

set(CMAKE_CXX_STANDARD 20)
# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
# Don't know why this is required (we're using CMake 3.15 > 3.0), but whatever.
if (POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif ()

set(srcfiles
    src/biome/biomeextra.cpp
    src/biome/biomes.cpp
    src/biome/biometree.cpp

    src/loot/builtin_loot.cpp

    src/misc/generator.cpp

    src/rng/noise.cpp
    src/rng/noises.cpp
    src/rng/rng.cpp

    src/structure/mansion.cpp

    src/terrain/internoise.cpp
    src/terrain/spline.cpp
    src/terrain/terrain.cpp
    src/terrain/terrainextra.cpp
)

add_library(mcmapper ${srcfiles})
target_include_directories(mcmapper PUBLIC include)

add_executable(vein vein.cpp)
target_link_libraries(vein PUBLIC mcmapper)

add_executable(monster_room monster_room.cpp)
target_link_libraries(monster_room PUBLIC mcmapper)

add_executable(main main.cpp)
target_link_libraries(main PUBLIC mcmapper)

add_executable(mansion_test mansion_test.cpp)
target_link_libraries(mansion_test PUBLIC mcmapper)
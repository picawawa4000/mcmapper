#include "tree/deserialize.hpp"

#include <mcmapper/rng/noises.hpp>

#include <fstream>

int main() {
    DeserializeData data;
    data.file = new std::ifstream("btree_1_21_4.bin", std::ios_base::binary | std::ios_base::in);

    std::shared_ptr<SearchTree> tree = deserialize(data);

    ClimateNoises noises(3447);
}
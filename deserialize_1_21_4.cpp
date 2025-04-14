#include <mcmapper/biome/tree/deserialize.hpp>

#include <mcmapper/biome/biomes.hpp>
#include <mcmapper/rng/noises.hpp>

#include <fstream>
#include <chrono>

inline void indent(std::ostream& log, int indentLevel) {
    for (int i = 0; i < indentLevel; ++i) log << "\t";
}

inline void printParams(std::ostream& log, std::vector<ParameterRange> params) {
    log << "[";
    for (int i = 0; i < params.size(); ++i) {
        log << "[" << params[i].min << ", " << params[i].max << "], ";
    }
    log << "] ";
}

void walkTree(std::ostream& log, std::shared_ptr<TreeNode> node, int indentLevel = 0) {
    if (indentLevel) indent(log, indentLevel);
    printParams(log, node->parameters);
    if (node->value) {
        log << biomeRepr(static_cast<Biome>(node->value)) << std::endl;
        return;
    }
    if (node->children.size() == 0)
        log << "No Children!!" << std::endl;
    log << std::endl;
    for (int i = 0; i < node->children.size(); ++i)
        walkTree(log, node->children[i], indentLevel + 1);
}

int main() {
    DeserializeData data;
    data.file = new std::ifstream("btree_1_21_4.bin", std::ios_base::binary | std::ios_base::in);

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    std::shared_ptr<SearchTree> tree = deserialize(data);

    auto end = clock.now();
    std::cout << "Deserialized in " << (end - start).count() << "ns" << std::endl;

    ClimateNoises noises(3447);
    NoisePoint point = noises.sample(0, 256, 512);

    u16 b = tree->get(point);
    std::cout << b << ": " << biomeRepr(static_cast<Biome>(b)) << std::endl;

    return 0;
}
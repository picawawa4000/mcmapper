#include "tree/serialize.hpp"

#include <fstream>

int main() {
    std::shared_ptr<SearchTree> tree = getSearchTree();
    std::ofstream stream("btree_1_21_4.bin");

    SerializeData data;
    data.tree = &stream;

    serialize(tree->root, data);

    return 0;
}
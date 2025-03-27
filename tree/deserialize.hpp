#ifndef DESERIALIZE_HPP
#define DESERIALIZE_HPP

#include <mcmapper/biome/biometree.hpp>

#include <istream>
#include <unordered_map>

struct DeserializeData {
    std::istream * file;
    std::vector<ParameterRange> params;
    // Destroying this is going to be insanely slow
    std::vector<std::shared_ptr<TreeNode>> nodes;

    inline u64 getULong() {
        u64 ret;
        *this->file >> ret;
        return ret;
    }
    inline i16 getShort() {
        i16 ret;
        *this->file >> ret;
        return ret;
    }
    inline u16 getUShort() {
        u16 ret;
        *this->file >> ret;
        return ret;
    }
};

inline void getHeader(DeserializeData& data) {
    u64 headerSize = data.getULong();
    data.params.reserve(headerSize);

    for (u64 i = 0; i < headerSize; ++i)
        data.params.push_back(ParameterRange((i64)data.getShort(), (i64)data.getShort()));
}

inline void getNodes(DeserializeData& data) {
    // There must be a better way to do this...
    u16 id, parentId, value;
    std::array<u16, 7> params;
    *data.file >> id >> parentId >> params[0] >> params[1] >> params[2] >> params[3] >> params[4] >> params[5] >> params[6] >> value;

    std::vector<ParameterRange> paramsFormat = {
        data.params[params[0]], data.params[params[1]], data.params[params[2]], data.params[params[3]],
        data.params[params[4]], data.params[params[5]], data.params[params[6]]
    };

#ifndef NDEBUG
    if (data.nodes.size() != id)
        throw std::runtime_error("Node has wrong id!");
#endif

    data.nodes.push_back(std::make_shared<TreeNode>(paramsFormat, value, std::vector<std::shared_ptr<TreeNode>>()));

    // Root node of the entire tree; no parent to attach to
    if (id == 0) return;
    
    // Attach the node to its parent
    // (this is where I become glad that I used shared pointers)
    // Does not prevent the removal of id
    data.nodes[parentId]->children.push_back(data.nodes[id]);
}

std::shared_ptr<SearchTree> deserialize(DeserializeData& data) {
    getHeader(data);
    getNodes(data);

    return std::make_shared<SearchTree>(data.nodes[0]);
}

#endif
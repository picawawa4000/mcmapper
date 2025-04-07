#ifndef DESERIALIZE_HPP
#define DESERIALIZE_HPP

#include <mcmapper/biome/biometree.hpp>

#include <istream>
#include <unordered_map>

// Determine endianness of host (from https://stackoverflow.com/a/2103095)
#include <limits.h>
#include <stdint.h>

#if CHAR_BIT != 8
#error "unsupported char size"
#endif

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul
};

static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order =
    { { 0, 1, 2, 3 } };

#define O32_HOST_ORDER (o32_host_order.value)

struct DeserializeData {
    std::istream * file;
    std::vector<ParameterRange> params;
    // Destroying this is going to be insanely slow
    std::vector<std::shared_ptr<TreeNode>> nodes;

    inline u64 getul() {
        u64 ret;
        // Optimise for big-endian
        if (O32_HOST_ORDER == O32_BIG_ENDIAN)
            this->file->read((char*)(&ret), 8);
        // We have to do this the hard way (little-endian)
        else
            ret = ((u64)this->file->get() << 56) | ((u64)this->file->get() << 48) | ((u64)this->file->get() << 40) | ((u64)this->file->get() << 32) \
                | ((u64)this->file->get() << 24) | ((u64)this->file->get() << 16) | ((u64)this->file->get() << 8) | ((u64)this->file->get());
        return ret;
    }
    inline u16 getus() {
        u16 ret;
        // Optimise for big-endian
        if (O32_HOST_ORDER == O32_BIG_ENDIAN)
            this->file->read((char*)(&ret), 2);
        // We have to do this the hard way (little-endian)
        else
            ret = (this->file->get() << 8) | (this->file->get());
        return ret;
    }
    inline i16 getss() {
        i16 ret;
        // Optimise for big-endian
        if (O32_HOST_ORDER == O32_BIG_ENDIAN)
            this->file->read((char*)(&ret), 2);
        // We have to do this the hard way (little-endian)
        else
            ret = (this->file->get() << 8) | (this->file->get());
        return ret;
    }
};

inline void getHeader(DeserializeData& data) {
    u64 headerSize = data.getul();
    data.params.reserve(headerSize);

    for (u64 i = 0; i < headerSize; ++i)
        data.params.push_back(ParameterRange((i64)data.getss(), (i64)data.getss()));
}

bool getNode(DeserializeData& data) {
    // There must be a better way to do this...
    u16 id = data.getus();
    u16 parentId = data.getus();
    u16 value = data.getus();

    std::array<u16, 7> params;
    for (int i = 0; i < 7; ++i)
        params[i] = data.getus();
    
    // EOF check (EOF isn't set until EOF is read,
    // so any node at the end of the file won't be
    // skipped)
    if (!data.file) return false;

    std::vector<ParameterRange> paramsFormat = {
        data.params[params[0]], data.params[params[1]], data.params[params[2]], data.params[params[3]],
        data.params[params[4]], data.params[params[5]], data.params[params[6]]
    };

#ifndef NDEBUG
    if (data.nodes.size() != id)
        throw std::runtime_error("Node has wrong id!");
#endif

    std::shared_ptr<TreeNode> ptr = std::make_shared<TreeNode>(paramsFormat, value, std::vector<std::shared_ptr<TreeNode>>());
    data.nodes.push_back(ptr);

    // Root node of the entire tree; no parent to attach to
    if (id == 0) return true;
    
    // Attach the node to its parent
    // (this is where I become glad that I used shared pointers)
    // Does not prevent the removal of id
    data.nodes[parentId]->children.push_back(data.nodes[id]);
    
    return true;
}

inline void getNodes(DeserializeData& data) {
    while (getNode(data));
}

std::shared_ptr<SearchTree> deserialize(DeserializeData& data) {
    getHeader(data);
    getNodes(data);

    return std::make_shared<SearchTree>(data.nodes[0]);
}

#endif
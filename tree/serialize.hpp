#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <mcmapper/biome/biometree.hpp>

#include <ostream>

struct SerializeData {
    std::ostream * tree;
    std::vector<ParameterRange> parameterTable;

    // Write operation guaranteeing big-endianness
    void write(uint16_t data) {
        this->tree->put((char)((data & 0xFF00) >> 8));
        this->tree->put((char)(data & 0xFF));
    }
    void write(uint64_t data) {
        this->tree->put((char)((data & (0xFFULL << 56)) >> 56));
        this->tree->put((char)((data & (0xFFULL << 48)) >> 48));
        this->tree->put((char)((data & (0xFFULL << 40)) >> 40));
        this->tree->put((char)((data & (0xFFULL << 32)) >> 32));
        this->tree->put((char)((data & (0xFFULL << 24)) >> 24));
        this->tree->put((char)((data & (0xFFULL << 16)) >> 16));
        this->tree->put((char)((data & (0xFFULL << 8)) >> 8));
        this->tree->put((char)(data & 0xFFULL));
    }
    void write(int16_t data) {
        this->write((uint16_t)data);
    }
    
    uint16_t newNodeId() {
        return this->nextNodeId++;
    }

private:
    uint16_t nextNodeId = 1;
};

static inline std::size_t get(const ParameterRange& parameters, SerializeData& data) {
    int i = 0;
    for (; i < data.parameterTable.size(); ++i)
        if (data.parameterTable[i] == parameters) return i;
    
    throw std::runtime_error("Did not find parameter range!");
}

static std::size_t putIfAbsent(const ParameterRange& params, SerializeData& data) {
    for (const ParameterRange& tparams : data.parameterTable)
        if (params == tparams) return 0;
    
    data.parameterTable.push_back(params);
    return 1;
}

static std::size_t putParameters(const std::shared_ptr<TreeNode>& node, SerializeData& data) {
    std::size_t ret = 0;
    for (int i = 0; i < 7; ++i)
        ret += putIfAbsent(node->parameters[i], data);
    // Leaf node
    if (node->value) return ret;
    // Branch node
    for (const std::shared_ptr<TreeNode>& child : node->children)
        ret += putParameters(child, data);
    return ret;
}

static void serialize(const std::vector<ParameterRange>& parameters, SerializeData& data) {
    for (int i = 0; i < 7; ++i) {
        const ParameterRange& params = parameters[i];
        std::size_t index = get(params, data);
#ifndef NDEBUG
        if (index >= std::numeric_limits<uint16_t>::max())
            throw std::runtime_error("Too many parameters!");
#endif
        data.write((uint16_t)index);
    }
}

// Node format: [u16 id, u16 parentId, u16 params[7], u16 value]
static std::size_t serialize(const std::shared_ptr<TreeNode>& node, uint16_t parentId, SerializeData& data) {
    std::size_t bytesWritten = 20;

    // This part is likely redundant, as the nodes are (almost certainly) guaranteed to be in ascending order.
    uint16_t id = data.newNodeId();
    data.write(id);
    data.write(parentId);
    serialize(node->parameters, data);
    data.write((uint16_t)node->value);

#ifndef NDEBUG
    // Redundant (hopefully) check for branch node
    if (!node->value)
#endif
        for (const std::shared_ptr<TreeNode>& child : node->children)
            bytesWritten += serialize(child, id, data);
    
    return bytesWritten;
}

// Serialized tree format: [paramCount, paramsHeader, nodes]
std::size_t serialize(const std::shared_ptr<TreeNode>& node, SerializeData& data) {
    // Pre-allocate space for the header
    std::size_t paramCount = putParameters(node, data);
    std::size_t paramByteCount = paramCount * 2 * sizeof(uint16_t);
    data.write((uint64_t)paramCount);
    for (int i = 0; i < paramByteCount; ++i)
        data.tree->put('\0');
    // Write the tree
    std::size_t treeByteCount = serialize(node, 0, data);
    // Write params header
    data.tree->seekp(8);
    for (const ParameterRange& range : data.parameterTable) {
        // These can be written as shorts instead of longs...
        data.write((int16_t)range.min);
        data.write((int16_t)range.max);
    }
#ifndef NDEBUG
    if (data.tree->tellp() != paramByteCount + 8)
        throw std::runtime_error("Wrote the wrong number of bytes!"
                " Expected " + std::to_string(paramByteCount + 8) + ""
                " but got " + std::to_string(data.tree->tellp()));
#endif
    return 8 + paramByteCount + treeByteCount;
}

#endif
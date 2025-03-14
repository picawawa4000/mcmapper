#ifndef ITEM_HPP
#define ITEM_HPP

#include <mcmapper/data/enchantment.hpp>

// An undefined count should be interpreted as 1.
// I don't know why I don't do that here, but I don't.
struct ItemStack {
    std::string id;
    u32 count = 1;
    std::optional<std::vector<EnchantmentInstance>> enchantments{};

    explicit ItemStack(std::string id) : id(id) {}
    ItemStack(std::string id, u32 count) : id(id), count(count) {}
    ItemStack(std::string id, std::vector<EnchantmentInstance> enchantments) : id(id), enchantments(enchantments) {}
    ItemStack(std::string id, u32 count, std::vector<EnchantmentInstance> enchantments) : id(id), count(count), enchantments(enchantments) {}
};

#endif
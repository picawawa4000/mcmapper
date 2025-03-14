#ifndef ENCHANTMENT_HPP
#define ENCHANTMENT_HPP

#include <mcmapper/util.hpp>

#include <string>
#include <array>

// TODO: give items their own file
struct Enchantment {
    const std::string id;
    u32 maxLevel = 1;

    explicit Enchantment(const std::string& id) : id(id) {}
    Enchantment(const std::string& id, u32 maxLevel) : id(id), maxLevel(maxLevel) {}
};

struct EnchantmentWithPower {
    const std::string id;
    u32 maxLevel;
    u32 minBaseCost, minCostPerLevel;
    u32 maxBaseCost, maxCostPerLevel;

    EnchantmentWithPower(const std::string& id, u32 maxLevel, u32 minBaseCost, u32 minCostPerLevel, u32 maxBaseCost, u32 maxCostPerLevel) : id(id), maxLevel(maxLevel), minBaseCost(minBaseCost), minCostPerLevel(minCostPerLevel), maxBaseCost(maxBaseCost), maxCostPerLevel(maxCostPerLevel) {}
};

struct EnchantmentInstance {
    const std::string id;
    u32 level;
    
    EnchantmentInstance(Enchantment type, u32 level) : id(type.id), level(level) {}
    EnchantmentInstance(EnchantmentWithPower type, u32 level) : id(type.id), level(level) {}
};

const std::array<const Enchantment, 39> vanillaEnchants{
    Enchantment("minecraft:protection", 4),
    Enchantment("minecraft:fire_protection", 4),
    Enchantment("minecraft:feather_falling", 4),
    Enchantment("minecraft:blast_protection", 4),
    Enchantment("minecraft:projectile_protection", 4),
    Enchantment("minecraft:respiration", 3),
    Enchantment("minecraft:aqua_affinity"),
    Enchantment("minecraft:thorns", 3),
    Enchantment("minecraft:depth_strider", 3),
    Enchantment("minecraft:frost_walker", 2),
    Enchantment("minecraft:binding_curse"),
    Enchantment("minecraft:sharpness", 5),
    Enchantment("minecraft:smite", 5),
    Enchantment("minecraft:bane_of_arthropods", 5),
    Enchantment("minecraft:knockback", 2),
    Enchantment("minecraft:fire_aspect", 2),
    Enchantment("minecraft:looting", 3),
    Enchantment("minecraft:sweeping_edge", 3),
    Enchantment("minecraft:efficiency", 5),
    Enchantment("minecraft:silk_touch"),
    Enchantment("minecraft:unbreaking", 3),
    Enchantment("minecraft:fortune", 3),
    Enchantment("minecraft:power", 5),
    Enchantment("minecraft:punch", 2),
    Enchantment("minecraft:flame"),
    Enchantment("minecraft:infinity"),
    Enchantment("minecraft:luck_of_the_sea", 3),
    Enchantment("minecraft:lure", 3),
    Enchantment("minecraft:loyalty", 3),
    Enchantment("minecraft:impaling", 5),
    Enchantment("minecraft:riptide", 3),
    Enchantment("minecraft:channeling"),
    Enchantment("minecraft:multishot"),
    Enchantment("minecraft:quick_charge", 3),
    Enchantment("minecraft:piercing", 4),
    Enchantment("minecraft:density", 5),
    Enchantment("minecraft:breach", 4),
    Enchantment("minecraft:mending"),
    Enchantment("minecraft:vanishing_curse")
};

const std::array<EnchantmentWithPower, 39> vanillaPowerEnchants{
    EnchantmentWithPower("minecraft:protection", 4, 1, 11, 12, 11),
    EnchantmentWithPower("minecraft:fire_protection", 4, 10, 8, 18, 8),
    EnchantmentWithPower("minecraft:feather_falling", 4, 5, 6, 11, 6),
    EnchantmentWithPower("minecraft:blast_protection", 4, 5, 8, 13, 8),
    EnchantmentWithPower("minecraft:projectile_protection", 4, 3, 6, 9, 6),
    EnchantmentWithPower("minecraft:respiration", 3, 10, 10, 40, 10),
    EnchantmentWithPower("minecraft:aqua_affinity", 1, 1, 0, 41, 0),
    EnchantmentWithPower("minecraft:thorns", 3, 10, 20, 60, 20),
    EnchantmentWithPower("minecraft:depth_strider", 3, 10, 10, 25, 10),
    EnchantmentWithPower("minecraft:frost_walker", 2, 10, 10, 25, 10),
    EnchantmentWithPower("minecraft:binding_curse", 1, 25, 0, 50, 0),
    EnchantmentWithPower("minecraft:sharpness", 5, 1, 11, 21, 11),
    EnchantmentWithPower("minecraft:smite", 5, 5, 8, 25, 8),
    EnchantmentWithPower("minecraft:bane_of_arthropods", 5, 5, 8, 25, 8),
    EnchantmentWithPower("minecraft:knockback", 2, 5, 20, 55, 20),
    EnchantmentWithPower("minecraft:fire_aspect", 2, 10, 20, 60, 20),
    EnchantmentWithPower("minecraft:looting", 3, 15, 9, 65, 9),
    EnchantmentWithPower("minecraft:sweeping_edge", 3, 5, 9, 20, 9),
    EnchantmentWithPower("minecraft:efficiency", 5, 1, 10, 51, 10),
    EnchantmentWithPower("minecraft:silk_touch", 1, 15, 0, 65, 0),
    EnchantmentWithPower("minecraft:unbreaking", 3, 5, 8, 55, 8),
    EnchantmentWithPower("minecraft:fortune", 3, 15, 9, 65, 9),
    EnchantmentWithPower("minecraft:power", 5, 1, 10, 16, 10),
    EnchantmentWithPower("minecraft:punch", 2, 12, 20, 37, 20),
    EnchantmentWithPower("minecraft:flame", 1, 20, 0, 50, 0),
    EnchantmentWithPower("minecraft:infinity", 1, 20, 0, 50, 0),
    EnchantmentWithPower("minecraft:luck_of_the_sea", 3, 15, 9, 65, 9),
    EnchantmentWithPower("minecraft:lure", 3, 15, 9, 65, 9),
    EnchantmentWithPower("minecraft:loyalty", 3, 12, 7, 50, 0),
    EnchantmentWithPower("minecraft:impaling", 5, 1, 8, 21, 8),
    EnchantmentWithPower("minecraft:riptide", 3, 17, 7, 50, 0),
    EnchantmentWithPower("minecraft:channeling", 1, 25, 0, 50, 0),
    EnchantmentWithPower("minecraft:multishot", 1, 20, 0, 50, 0),
    EnchantmentWithPower("minecraft:quick_charge", 3, 12, 20, 50, 0),
    EnchantmentWithPower("minecraft:piercing", 4, 1, 10, 50, 0),
    EnchantmentWithPower("minecraft:density", 5, 5, 8, 25, 8),
    EnchantmentWithPower("minecraft:breach", 4, 15, 9, 65, 9),
    EnchantmentWithPower("minecraft:mending", 1, 25, 0, 50, 0),
    EnchantmentWithPower("minecraft:vanishing_curse", 1, 25, 0, 50, 0),
};

#endif
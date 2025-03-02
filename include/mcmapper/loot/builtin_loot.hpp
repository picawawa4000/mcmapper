#include <mcmapper/loot/loot.hpp>

std::shared_ptr<LootTable> desertPyramidLoot();
std::shared_ptr<LootTable> dungeonLoot();

// Deprecated until enchant with levels becomes a thing.
/*
std::shared_ptr<LootTable> jungleTempleLoot() {
    static std::shared_ptr<LootTable> JUNGLE_TEMPLE{};
    if (JUNGLE_TEMPLE) return JUNGLE_TEMPLE;

    LootPoolBuilder * p = new LootPoolBuilder(2, 6);
    p->entry(3, "minecraft:diamond", 1, 3);
    p->entry(10, "minecraft:iron_ingot", 1, 5);
    p->entry(15, "minecraft:gold_ingot", 2, 7);
    p->entry(15, "minecraft:bamboo", 1, 3);
    p->entry(2, "minecraft:emerald", 1, 3);
    p->entry(20, "minecraft:bone", 4, 6);
    p->entry(16, "minecraft:rotten_flesh", 3, 7);
    p->entry(3, "minecraft:saddle");
    p->entry(1, "minecraft:iron_horse_armour");
    p->entry(1, "minecraft:gold_horse_armour");
    p->entry(1, "minecraft:diamond_horse_armour");
    // note to self: enchant with levels (30)
    p->entry(1, "minecraft:enchanted_book");

    LootPoolBuilder * t = new LootPoolBuilder(1);
    t->entry(2);
    t->entry(1, "minecraft:wild_armor_trim_smithing_template");
}
*/

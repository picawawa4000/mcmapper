#include <mcmapper/loot.hpp>

std::shared_ptr<LootTable> getOrBuildDesertPyramidLootTable() {
    static std::shared_ptr<LootTable> DESERT_PYRAMID{};
    if (DESERT_PYRAMID) return DESERT_PYRAMID;

    LootPoolBuilder * p = new LootPoolBuilder(2, 4);
    p->entry(5, "minecraft:diamond", 1, 3);
    p->entry(15, "minecraft:iron_ingot", 1, 5);
    p->entry(15, "minecraft:gold_ingot", 2, 7);
    p->entry(15, "minecraft:emerald", 1, 3);
    p->entry(25, "minecraft:bone", 4, 6);
    p->entry(25, "minecraft:spider_eye", 1, 3);
    p->entry(25, "minecraft:rotten_flesh", 3, 7);
    p->entry(20, "minecraft:saddle");
    p->entry(15, "minecraft:iron_horse_armor");
    p->entry(10, "minecraft:golden_horse_armor");
    p->entry(5, "minecraft:diamond_horse_armor");
    /// TODO: item modifiers (i.e. enchantments)
    p->entry(20, "minecraft:enchanted_book", bool_t(true));
    p->entry(20, "minecraft:golden_apple");
    p->entry(2, "minecraft:enchanted_golden_apple");
    p->entry(15);

    LootPoolBuilder * j = new LootPoolBuilder(4); // j for junk
    j->entry(10, "minecraft:bone", 1, 8);
    j->entry(10, "minecraft:gunpowder", 1, 8);
    j->entry(10, "minecraft:rotten_flesh", 1, 8);
    j->entry(10, "minecraft:string", 1, 8);
    j->entry(10, "minecraft:sand", 1, 8);

    LootPoolBuilder * t = new LootPoolBuilder(1); // t for trim
    t->entry(6);
    t->entry(1, "minecraft:dune_armor_trim_smithing_template", 2);

    LootTableBuilder * b = new LootTableBuilder();
    b->pool(p->build());
    b->pool(j->build());
    b->pool(t->build());

    DESERT_PYRAMID = std::shared_ptr<LootTable>(b->build());

    delete p;
    delete j;
    delete t;
    delete b;

    return DESERT_PYRAMID;
}

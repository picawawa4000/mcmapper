#include <mcmapper/loot/builtin_loot.hpp>

std::shared_ptr<LootTable> desertPyramidLoot() {
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

std::shared_ptr<LootTable> dungeonLoot() {
    static std::shared_ptr<LootTable> DUNGEON{};
    if (DUNGEON) return DUNGEON;

    LootPoolBuilder * p = new LootPoolBuilder(1, 3);
    p->entry(20, "minecraft:saddle");
    p->entry(15, "minecraft:golden_apple");
    p->entry(2, "minecraft:enchanted_golden_apple");
    p->entry(2, "minecraft:music_disc_otherside");
    p->entry(15, "minecraft:music_disc_13");
    p->entry(15, "minecraft:music_disc_cat");
    p->entry(20, "minecraft:name_tag");
    p->entry(10, "minecraft:golden_horse_armour");
    p->entry(15, "minecraft:iron_horse_armour");
    p->entry(5, "minecraft:diamond_horse_armour");
    p->entry(10, "minecraft:enchanted_book", bool_t(true));

    LootPoolBuilder * q = new LootPoolBuilder(1, 4);
    q->entry(10, "minecraft:iron_ingot", 1, 4);
    q->entry(5, "minecraft:gold_ingot", 1, 4);
    q->entry(20, "minecraft:bread");
    q->entry(20, "minecraft:wheat", 1, 4);
    q->entry(10, "minecraft:bucket");
    q->entry(15, "minecraft:redstone", 1, 4);
    q->entry(15, "minecraft:coal", 1, 4);
    q->entry(10, "minecraft:melon_seeds", 2, 4);
    q->entry(10, "minecraft:pumpkin_seeds", 2, 4);
    q->entry(10, "minecraft:beetroot_seeds", 2, 4);

    LootPoolBuilder * r = new LootPoolBuilder(3);
    r->entry(10, "minecraft:bone", 1, 8);
    r->entry(10, "minecraft:gunpowder", 1, 8);
    r->entry(10, "minecraft:rotten_flesh", 1, 8);
    r->entry(10, "minecraft:string", 1, 8);
    
    LootTableBuilder * b = new LootTableBuilder();
    b->pool(p->build());
    b->pool(q->build());
    b->pool(r->build());

    DUNGEON = std::shared_ptr<LootTable>(b->build());

    delete p;
    delete q;
    delete r;
    delete b;

    return DUNGEON;
}

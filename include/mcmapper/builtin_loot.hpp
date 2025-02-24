#include <mcmapper/loot.hpp>

#include <iostream>

struct LootPoolBuilder {
    explicit LootPoolBuilder(i32 count) {
        this->rolls = std::make_unique<ConstantLootNumberProvider>(count);
    }
    LootPoolBuilder(i32 min, i32 max) {
        this->rolls = std::make_unique<UniformLootNumberProvider>(min, max);
    }

    // NothingEntry
    void entry(u32 weight) {
        this->weights.weights.push_back(LootWeight(weight));
        NothingEntry * entry = new NothingEntry();
        this->entries.push_back(std::unique_ptr<LootEntry>(entry));
    }

    // Helpers
    void entry(u32 weight, const std::string& stackID) {
        this->weights.weights.push_back(LootWeight(weight));
        this->entries.push_back(std::make_unique<ItemEntry>(stackID));
    }
    void entry(u32 weight, const std::string& stackID, u32 count) {
        this->weights.weights.push_back(LootWeight(weight));
        ConstantLootNumberProvider provider(count);
        this->entries.push_back(std::make_unique<ItemEntry>(stackID, count));
    }
    void entry(u32 weight, const std::string& stackID, u32 min, u32 max) {
        this->weights.weights.push_back(LootWeight(weight));
        UniformLootNumberProvider provider(min, max);
        this->entries.push_back(std::make_unique<ItemEntry>(stackID, min, max));
    }

    LootPool * build() {
        for (LootWeight& weight : this->weights.weights) std::cout << weight.baseWeight << std::endl;
        return new LootPool(this->rolls, this->weights, this->entries);
    }

private:
    std::unique_ptr<LootNumberProvider> rolls;
    LootWeights weights;
    std::vector<std::unique_ptr<LootEntry>> entries;
};

struct LootTableBuilder {
    void pool(LootPool * pool) {
        this->pools.push_back(std::unique_ptr<LootPool>(pool));
    }
    LootTable * build() {return new LootTable(this->pools);}

private:
    std::vector<std::unique_ptr<LootPool>> pools;
};

std::shared_ptr<LootTable> getOrBuildDesertPyramidLootTable() {
    static std::shared_ptr<LootTable> DESERT_PYRAMID = nullptr;
    if (DESERT_PYRAMID) return DESERT_PYRAMID;

    LootPoolBuilder * p = new LootPoolBuilder(2, 4);
    p->entry(5, "minecraft:diamond", 1, 3);
    p->entry(15, "minecraft:iron_ingot", 1, 5);
    p->entry(15, "minecraft:gold_ingot", 2, 7);
    p->entry(15, "minecraft:emerald", 1, 3);
    p->entry(25, "minecraft:bone", 4, 6);
    p->entry(25, "minecraft:bone", 1, 3);
    p->entry(25, "minecraft:rotten_flesh", 3, 7);
    p->entry(20, "minecraft:saddle");
    p->entry(15, "minecraft:iron_horse_armor");
    p->entry(10, "minecraft:golden_horse_armor");
    p->entry(5, "minecraft:diamond_horse_armor");
    /// TODO: item modifiers (i.e. enchantments)
    p->entry(20, "minecraft:enchanted_book");
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

/*
LootTable DESERT_PYRAMID{
    std::initializer_list<LootPool>{
        // Loot pool
        pool(
            UniformLootNumberProvider(2, 4),
            // Vector of entries
            entry_list{
                // Weighted entry
                entry{
                    // Weight
                    5,
                    // Item entry
                    item(
                        // Item ID
                        "minecraft:diamond",
                        // Count
                        UniformLootNumberProvider(1, 3)
                    )
                },
                entry{
                    15,
                    item(
                        "minecraft:iron_ingot",
                        UniformLootNumberProvider(1, 5)
                    )
                },
                entry{
                    15,
                    item(
                        "minecraft:gold_ingot",
                        UniformLootNumberProvider(2, 7)
                    )
                },
                entry{
                    15,
                    item(
                        "minecraft:emerald",
                        UniformLootNumberProvider(1, 3)
                    )
                },
                entry{
                    25,
                    item(
                        "minecraft:bone",
                        UniformLootNumberProvider(4, 6)
                    )
                },
                entry{
                    25,
                    item(
                        "minecraft:bone",
                        UniformLootNumberProvider(1, 3)
                    )
                },
                entry{
                    25,
                    item(
                        "minecraft:rotten_flesh",
                        UniformLootNumberProvider(3, 7)
                    )
                },
                entry{
                    20,
                    item(
                        "minecraft:saddle"
                        // If no count is provided, defaults to 1 like in the base game
                    )
                },
                entry{
                    15,
                    item(
                        "minecraft:iron_horse_armor"
                    )
                },
                entry{
                    10,
                    item(
                        "minecraft:golden_horse_armor"
                    )
                },
                entry{
                    5,
                    item(
                        "minecraft:diamond_horse_armor"
                    )
                },
                entry{
                    20,
                    item(
                        "minecraft:enchanted_book"
                        ///FIXME: No item modifier support as of yet
                    )
                },
                entry{
                    20,
                    item(
                        "minecraft:golden_apple"
                    )
                },
                entry{
                    2,
                    item(
                        "minecraft:enchanted_golden_apple"
                    )
                },
                entry{
                    10,
                    item(
                        "minecraft:bone",
                        UniformLootNumberProvider(1, 8)
                    )
                },
                entry{
                    10,
                    item(
                        "minecraft:gunpowder",
                        UniformLootNumberProvider(1, 8)
                    )
                },
                entry{
                    10,
                    item(
                        "minecraft:rotten_flesh",
                        UniformLootNumberProvider(1, 8)
                    )
                },
                entry{
                    10,
                    item(
                        "minecraft:string",
                        UniformLootNumberProvider(1, 8)
                    )
                },
                entry{
                    10,
                    item(
                        "minecraft:sand",
                        UniformLootNumberProvider(1, 8)
                    )
                }
            }
        ),
        pool(
            ConstantLootNumberProvider(1),
            entry_list{
                entry{
                    6,
                    empty()
                },
                entry{
                    1,
                    item(
                        "minecraft:dune_armor_trim_smithing_template",
                        ConstantLootNumberProvider(2)
                    )
                }
            }
        )
    }
};
*/
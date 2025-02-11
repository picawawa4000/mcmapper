#ifndef LOOT_HPP
#define LOOT_HPP

#include <mcmapper/rng.hpp>
#include <array>
#include <functional>
#include <optional>
#include <string>

struct ItemStack {
    std::string id;
    std::optional<u8> count;

    ItemStack(std::string id) : id(id), count(std::optional<u8>()) {}
    ItemStack(std::string id, u8 count) : id(id), count(count) {}
};

struct LootNumberProvider {
    virtual i32 next(LootContext& context) = 0;
};

struct ConstantLootNumberProvider : public LootNumberProvider {
    i32 count;

    explicit ConstantLootNumberProvider(i32 count) : count(count) {}

    virtual i32 next(LootContext& context) override {
        return this->count;
    }
};

struct UniformLootNumberProvider : public LootNumberProvider {
    i32 min, max;

    UniformLootNumberProvider(i32 min, i32 max) : min(min), max(max) {};

    virtual i32 next(LootContext& context) override {
        return context.random->next_i32(max - min + 1) + min;
    }
};

struct LootWeights {
    std::vector<u32> weights;
    u32 sum = 0;

    explicit LootWeights(std::vector<u32> weights) : weights(weights) {
        for (u32 weight : weights) this->sum += weight;
    }

    u32 select(Random& random) {
        i32 idx = random.next_i32(this->sum);
    }
};

struct LootContext {
    Random * random;

    LootContext() = default;
    explicit LootContext(Random * random) : random(random) {}
    explicit LootContext(u64 seed) : random(new CheckedRandom(seed)) {}

    ~LootContext() {
        delete this->random;
    }

    u32 select(std::vector<u32> weights) {

    }
};

struct LootEntry {
    virtual i32 getWeight() = 0;
    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) = 0;
};

struct ItemEntry : public LootEntry {
    const std::string& stackID;
    LootNumberProvider& count;
    i32 weight;

    ItemEntry(i32 weight, const std::string& stack, LootNumberProvider& count) : weight(weight), stackID(stackID), count(count) {}

    virtual i32 getWeight() override {
        return this->weight;
    }

    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) override {
        i32 itemCount = this->count.next(context);
        enter(ItemStack(stackID, (u8)itemCount));
    }
};

struct LootPool {
    LootNumberProvider& rolls;

    explicit LootPool(LootNumberProvider& rolls) : rolls(rolls) {}

    void roll(LootContext& context, std::function<void(ItemStack)> enter) {
        for (i32 rollCount = rolls.next(context); rollCount > 0; --rollCount) {
            
        }
    }

private:
    void rollOnce(LootContext& context, std::function<void(ItemStack)> enter) {

    }
};

struct LootTable {
    std::vector<LootPool> lootPools;

    explicit LootTable(std::vector<LootPool> lootPools) : lootPools(lootPools) {}

    void roll(LootContext& context, std::function<void(ItemStack)> enter) {
        for (LootPool pool : this->lootPools) pool.roll(context, enter);
    }
};

XoroshiroRandom getLootRandom(i64 worldSeed, i64 blockX, i64 blockZ, i64 index, i64 step) {
    XoroshiroRandom temp(worldSeed);
    i64 m = temp.next_u64() | 1L;
    i64 n = temp.next_u64() | 1L;
    i64 o = (blockX * m + blockZ * n) ^ worldSeed;
    i64 p = o + index + 10000 * step;
    return XoroshiroRandom(p);
}

std::array<u64, 4> getDesertTempleLootSeeds(XoroshiroRandom random) {
    random.next_u64();
    return {random.next_u64(), random.next_u64(), random.next_u64(), random.next_u64()};
}

std::vector<ItemStack> rollDesertTempleLoot(std::array<u64, 4> lootSeeds) {
    for (u64 lootSeed : lootSeeds) {

    }
}

#endif
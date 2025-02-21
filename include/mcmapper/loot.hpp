#ifndef LOOT_HPP
#define LOOT_HPP

#include <mcmapper/rng.hpp>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <memory>

struct LootContext {
    CheckedRandom random;
    float luck;

    explicit LootContext(CheckedRandom random) : random(random), luck(0) {}
    LootContext(CheckedRandom random, float luck) : random(random), luck(luck) {}
    explicit LootContext(u64 seed) : random(CheckedRandom(seed)), luck(0) {}
    LootContext(u64 seed, float luck) : random(CheckedRandom(seed)), luck(luck) {}
};

// An undefined count should be interpreted as 1.
// I don't know why I don't do that here, but I don't.
struct ItemStack {
    std::string id;
    std::optional<u32> count;

    ItemStack(std::string id) : id(id), count(std::optional<u32>()) {}
    ItemStack(std::string id, u32 count) : id(id), count(count) {}
};

// Abstract
struct LootNumberProvider {
    virtual ~LootNumberProvider() {}
    virtual i32 next(LootContext& context) = 0;
};

struct ConstantLootNumberProvider : public LootNumberProvider {
    i32 count;

    explicit ConstantLootNumberProvider(i32 count) : count(count) {}

    virtual ~ConstantLootNumberProvider() {}

    virtual i32 next(LootContext& context) override {
        return this->count;
    }
};

struct UniformLootNumberProvider : public LootNumberProvider {
    i32 min, max;

    UniformLootNumberProvider(i32 min, i32 max) : min(min), max(max) {};

    virtual ~UniformLootNumberProvider() {}

    virtual i32 next(LootContext& context) override {
        return context.random.next_i32(max - min + 1) + min;
    }
};

// A glorified pair representing a single weight within a LootWeights.
struct LootWeight {
    u32 baseWeight;
    u32 bonusWeight;

    explicit LootWeight(u32 baseWeight) : baseWeight(baseWeight), bonusWeight(0) {}
    LootWeight(u32 baseWeight, u32 bonusWeight) : baseWeight(baseWeight), bonusWeight(bonusWeight) {}

    u32 get(float luck) const {
        return this->baseWeight + luck * this->bonusWeight;
    }
};

// A glorified vector representing a set of loot weights.
struct LootWeights {
    std::vector<LootWeight> weights;

    LootWeights() = default;
    explicit LootWeights(std::vector<LootWeight> weights) : weights(weights) {}

    u32 select(LootContext& context) {
        if (this->weights.size() == 1) return 0;
        u32 sum = 0;
        for (int i = 0; i < this->weights.size(); ++i) sum += this->weights[i].get(context.luck);
        u32 idx = (u32)context.random.next_i32(sum);
        u32 sumToNow = 0;
        for (u32 i = 0; i < this->weights.size(); ++i) {
            sumToNow += this->weights[i].get(context.luck);
            if (idx < sumToNow) return i;
        }
        throw std::runtime_error("Random number generator returned erroneous value (check next_i32 implementation)!");
    }
};

// Abstract
struct LootEntry {
    virtual ~LootEntry() {}
    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) = 0;
};

struct ItemEntry : public LootEntry {
    const std::string& stackID;
    std::unique_ptr<LootNumberProvider> count;

    ItemEntry(const std::string& stackID) : stackID(stackID), count(std::unique_ptr<LootNumberProvider>(new ConstantLootNumberProvider(1))) {}
    // Invalidates count
    ItemEntry(const std::string& stackID, std::unique_ptr<LootNumberProvider> count) : stackID(stackID) {
        this->count.swap(count);
    }

    // For builders
    template <class T> ItemEntry(const std::string& stackID, T count) : stackID(stackID) {
        static_assert(std::is_convertible_v<T*, LootNumberProvider*>);
        this->count.reset(&count);
    }

    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) override {
        i32 itemCount = this->count->next(context);
        enter(ItemStack(stackID, (u32)itemCount));
    }
};

struct NothingEntry : public LootEntry {
    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) override {}
};

struct LootPool {
    std::unique_ptr<LootNumberProvider> rolls;
    LootWeights weights;
    std::vector<std::unique_ptr<LootEntry>> entries;

    // Invalidates rolls and every pointer in entries
    LootPool(std::unique_ptr<LootNumberProvider>& rolls, LootWeights weights, std::vector<std::unique_ptr<LootEntry>>& entries) : weights(weights) {
        this->rolls.swap(rolls);

        this->entries.reserve(entries.size());
        for (int i = 0; i < entries.size(); ++i) {
            this->entries.push_back(std::unique_ptr<LootEntry>());
            this->entries[i].swap(entries[i]);
        }
    }

    // Builder constructor
    template <class T> LootPool(T rolls, std::vector<std::pair<u32, std::unique_ptr<LootEntry>>> entries) {
        static_assert(std::is_convertible_v<T*, LootNumberProvider*>);
        this->rolls.reset(&rolls);

        this->weights.weights.reserve(entries.size());
        this->entries.reserve(entries.size());
        for (std::pair<u32, std::unique_ptr<LootEntry>>& entry : entries) {
            this->weights.weights.push_back(LootWeight(entry.first));
            this->entries.push_back(std::unique_ptr<LootEntry>());
            this->entries.back().swap(entry.second);
        }
    }

    //"Copy" constructor (invalidates other)
    explicit LootPool(LootPool& other) {
        this->rolls.swap(other.rolls);
        this->weights = other.weights;

        this->entries.reserve(other.entries.size());
        for (int i = 0; i < other.entries.size(); ++i) {
            this->entries.push_back(std::unique_ptr<LootEntry>());
            this->entries[i].swap(other.entries[i]);
        }
    }

    void roll(LootContext& context, std::function<void(ItemStack)> enter) {
        for (i32 rollCount = rolls->next(context); rollCount > 0; --rollCount)
            this->rollOnce(context, enter);
    }

private:
    void rollOnce(LootContext& context, std::function<void(ItemStack)> enter) {
        u32 index = weights.select(context);
        entries[index]->generateLoot(context, enter);
    }
};

struct LootTable {
    std::vector<std::unique_ptr<LootPool>> lootPools;

    // Invalidates lootPools
    LootTable(std::vector<std::unique_ptr<LootPool>>& lootPools) : lootPools(std::move(lootPools)) {}

    void roll(LootContext& context, std::function<void(ItemStack)> enter) {
        for (std::unique_ptr<LootPool>& pool : this->lootPools) pool->roll(context, enter);
    }
};

#endif
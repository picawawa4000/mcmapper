#ifndef LOOT_HPP
#define LOOT_HPP

#include <mcmapper/rng.hpp>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <memory>

#include <iostream>

// TODO: give items their own file
struct Enchantment {
    const std::string id;
    u32 maxLevel = 1;

    explicit Enchantment(const std::string& id) : id(id) {}
    Enchantment(const std::string& id, u32 maxLevel) : id(id), maxLevel(maxLevel) {}
};

struct EnchantmentInstance {
    const std::string id;
    u32 level;
    
    EnchantmentInstance(Enchantment type, u32 level) : id(type.id), level(level) {}
};

const std::array<Enchantment, 39> vanillaRandomEnchants{
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
    u32 count = 1;
    std::optional<std::vector<EnchantmentInstance>> enchantments{};

    explicit ItemStack(std::string id) : id(id) {}
    ItemStack(std::string id, u32 count) : id(id), count(count) {}
    ItemStack(std::string id, std::vector<EnchantmentInstance> enchantments) : id(id), enchantments(enchantments) {}
    ItemStack(std::string id, u32 count, std::vector<EnchantmentInstance> enchantments) : id(id), count(count), enchantments(enchantments) {}
};

// Abstract
struct LootNumberProvider {
    virtual ~LootNumberProvider() = default;
    virtual i32 next(LootContext& context) const = 0;
};

struct ConstantLootNumberProvider : public LootNumberProvider {
    i32 count;

    explicit ConstantLootNumberProvider(i32 count) : count(count) {}

    virtual ~ConstantLootNumberProvider() = default;

    virtual i32 next(LootContext& context) const override {
        return this->count;
    }
};

struct UniformLootNumberProvider : public LootNumberProvider {
    i32 min, max;

    UniformLootNumberProvider(i32 min, i32 max) : min(min), max(max) {}

    virtual ~UniformLootNumberProvider() = default;

    virtual i32 next(LootContext& context) const override {
        return context.random.next_i32(max - min + 1) + min;
    }
};

//Abstract
struct LootFunction {
    virtual ~LootFunction() = default;
    virtual ItemStack apply(const ItemStack& input, LootContext& context) = 0;
};

struct EnchantRandomlyLootFunction : public LootFunction {
    virtual ~EnchantRandomlyLootFunction() = default;

    virtual ItemStack apply(const ItemStack& input, LootContext& context) {
        const UniformLootNumberProvider provider(0, vanillaRandomEnchants.size());

        i32 idx = provider.next(context);

        i32 level = 1;
        if (vanillaRandomEnchants[idx].maxLevel != 1) {
            UniformLootNumberProvider levelProvider(1, vanillaRandomEnchants[idx].maxLevel);
            level = levelProvider.next(context);
        }
        
        return ItemStack(input.id, input.count, {EnchantmentInstance(vanillaRandomEnchants[idx], level)});
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
    // should these be stored here? probably not. but they are.
    std::vector<std::unique_ptr<LootFunction>> functions{};

    virtual ~LootEntry() = default;
    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) = 0;
};

struct ItemEntry : public LootEntry {
    const std::string stackID;
    std::unique_ptr<LootNumberProvider> count;

    ItemEntry(const std::string& stackID, i32 count) : stackID(stackID), count(std::make_unique<ConstantLootNumberProvider>(count)) {}
    ItemEntry(const std::string& stackID, i32 min, i32 max) : stackID(stackID), count(std::make_unique<UniformLootNumberProvider>(min, max)) {}
    explicit ItemEntry(const std::string& stackID) : ItemEntry(stackID, 1) {}

    template <class T, typename std::enable_if<std::is_base_of<LootFunction, T>::value>::type* = nullptr>
    ItemEntry(const std::string& stackID, u32 count, T func) : ItemEntry(stackID, count) {
        this->functions.push_back(std::move(std::make_unique<T>(func)));
    }
    template <class T, typename std::enable_if<std::is_base_of<LootFunction, T>::value>::type* = nullptr>
    ItemEntry(const std::string& stackID, u32 min, u32 max, T func) : ItemEntry(stackID, min, max) {
        this->functions.push_back(std::move(std::make_unique<T>(func)));
    }
    template <class T, typename std::enable_if<std::is_base_of<LootFunction, T>::value>::type* = nullptr>
    ItemEntry(const std::string& stackID, T func) : ItemEntry(stackID) {
        this->functions.push_back(std::move(std::make_unique<T>(func)));
    }

    virtual ~ItemEntry() = default;

    virtual void generateLoot(LootContext& context, std::function<void(ItemStack)> enter) override {
        i32 itemCount = this->count->next(context);
        ItemStack stack(this->stackID, (u32)itemCount);

        for (std::unique_ptr<LootFunction>& func : this->functions)
            stack = func->apply(stack, context);

        enter(stack);
    }
};

struct NothingEntry : public LootEntry {
    virtual ~NothingEntry() = default;
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
        this->rolls = std::make_unique<T>(rolls);
        
        this->weights.weights.reserve(entries.size());
        this->entries.reserve(entries.size());
        for (std::pair<u32, std::unique_ptr<LootEntry>>& entry : entries) {
            this->weights.weights.push_back(LootWeight(entry.first));
            this->entries.push_back(std::unique_ptr<LootEntry>());
            this->entries.back().swap(entry.second);
        }
    }

    void roll(LootContext& context, std::function<void(ItemStack)> enter) {
        for (i32 rollCount = this->rolls->next(context); rollCount > 0; --rollCount) {
            std::cout << rollCount << " rolls left" << std::endl;
            this->rollOnce(context, enter);
        }
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
        for (std::unique_ptr<LootPool>& pool : this->lootPools)
            pool->roll(context, enter);
    }
};

// Helper to avoid u32s being cast to bools, which would cause overload resolution problems.
struct bool_t {
    bool val;
    explicit bool_t(bool val) : val(val) {}
    operator bool() {return this->val;}
};

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
    void entry(u32 weight, const std::string& stackID, bool_t enchant = bool_t(false)) {
        this->weights.weights.push_back(LootWeight(weight));
        if (enchant)
            this->entries.push_back(std::make_unique<ItemEntry>(stackID, EnchantRandomlyLootFunction()));
        else
            this->entries.push_back(std::make_unique<ItemEntry>(stackID));
    }
    void entry(u32 weight, const std::string& stackID, u32 count, bool_t enchant = bool_t(false)) {
        this->weights.weights.push_back(LootWeight(weight));
        if (enchant)
            this->entries.push_back(std::make_unique<ItemEntry>(stackID, count, EnchantRandomlyLootFunction()));
        else
            this->entries.push_back(std::make_unique<ItemEntry>(stackID, count));
    }
    void entry(u32 weight, const std::string& stackID, u32 min, u32 max, bool_t enchant = bool_t(false)) {
        this->weights.weights.push_back(LootWeight(weight));
        if (enchant)
            this->entries.push_back(std::make_unique<ItemEntry>(stackID, min, max, EnchantRandomlyLootFunction()));
        else
            this->entries.push_back(std::make_unique<ItemEntry>(stackID, min, max));
    }

    LootPool * build() {
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

#endif
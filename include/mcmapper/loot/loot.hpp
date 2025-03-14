#ifndef LOOT_HPP
#define LOOT_HPP

#include <mcmapper/rng/rng.hpp>
#include <mcmapper/data/item.hpp>

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
        return context.random.next_i32(this->max - this->min + 1) + this->min;
    }
};

//Abstract
struct LootFunction {
    virtual ~LootFunction() = default;
    virtual ItemStack apply(const ItemStack& input, LootContext& context) = 0;
};

// Currently, this does not work correctly
// (try loot seed 33333; expect enchanted_book with protection 4)
struct EnchantRandomlyLootFunction : public LootFunction {
    virtual ~EnchantRandomlyLootFunction() = default;

    virtual ItemStack apply(const ItemStack& input, LootContext& context) {
        const UniformLootNumberProvider provider(0, vanillaEnchants.size());
        i32 idx = provider.next(context);

        i32 level = 1;
        if (vanillaEnchants[idx].maxLevel != 1) {
            UniformLootNumberProvider levelProvider(1, vanillaEnchants[idx].maxLevel);
            level = levelProvider.next(context);
        }
        
        return ItemStack(input.id, input.count, {EnchantmentInstance(vanillaEnchants[idx], level)});
    }
};

struct EnchantWithLevelsLootFunction : public LootFunction {
    u32 levels;
    u32 enchantability;

    explicit EnchantWithLevelsLootFunction(u32 levels) : levels(levels), enchantability(1) {}
    // Enchantability defaults to 1.
    // Stone tools have 5.
    // Iron armour and the turtle shell have 9.
    // Diamond tools and armour have 10.
    // Chain armour has 12.
    // Iron tools have 14.
    // Wood tools, leather armour, netherite tools and armour, and the mace have 15.
    // Gold tools have 22.
    // Gold armour has 25.
    EnchantWithLevelsLootFunction(u32 levels, u32 enchantability) : levels(levels), enchantability(enchantability) {}

    virtual ~EnchantWithLevelsLootFunction() = default;

    virtual ItemStack apply(const ItemStack& input, LootContext& context) {
        u32 l = this->levels + 1 + context.random.next_i32(this->enchantability / 4 + 1) + context.random.next_i32(this->enchantability / 4 + 1);
        f32 f = (context.random.next_f32() + context.random.next_f32() - 1.f) * 0.15f;
        l = std::clamp((i32)std::round((f32)l + (f32)l * f), 1, std::numeric_limits<i32>::max());
        std::vector<EnchantmentInstance> possibleEnchants;
        for (EnchantmentWithPower enchantment : vanillaPowerEnchants)
            for (int i = enchantment.maxLevel; i >= 1; --i) {
                if (i < enchantment.minBaseCost + enchantment.minCostPerLevel * (i - 1) || i > enchantment.maxBaseCost + enchantment.maxCostPerLevel * (i - 1)) continue;
                possibleEnchants.emplace_back(enchantment, i);
                break;
            }
        std::vector<EnchantmentInstance> enchantments;
        if (!possibleEnchants.empty()) {
            
        }
        throw std::runtime_error("Unimplemented function EnchantWithLevelsLootFunction::apply()!");
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
        for (i32 rollCount = this->rolls->next(context); rollCount > 0; --rollCount)
            this->rollOnce(context, enter);
    }

private:
    void rollOnce(LootContext& context, std::function<void(ItemStack)> enter) {
        u32 index = this->weights.select(context);
        this->entries[index]->generateLoot(context, enter);
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

struct Modifier {
    enum Type {
        NONE,
        ENCHANT_RANDOMLY,
        ENCHANT_WITH_LEVELS,
    };

    Type type;
    // ENCHANT_WITH_LEVELS
    u32 levels = 1;
    u32 enchantability = 1;
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
    void entry(u32 weight, const std::string& stackID, Modifier modifier = {.type=Modifier::Type::NONE}) {
        this->weights.weights.push_back(LootWeight(weight));
        switch (modifier.type) {
            case Modifier::Type::ENCHANT_RANDOMLY:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, EnchantRandomlyLootFunction()));
                break;
            case Modifier::Type::ENCHANT_WITH_LEVELS:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, EnchantWithLevelsLootFunction(modifier.levels, modifier.enchantability)));
                break;
            default:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID));
        }
    }
    void entry(u32 weight, const std::string& stackID, u32 count, Modifier modifier = {.type=Modifier::Type::NONE}) {
        this->weights.weights.push_back(LootWeight(weight));
        switch (modifier.type) {
            case Modifier::Type::ENCHANT_RANDOMLY:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, count, EnchantRandomlyLootFunction()));
                break;
            case Modifier::Type::ENCHANT_WITH_LEVELS:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, count, EnchantWithLevelsLootFunction(modifier.levels, modifier.enchantability)));
                break;
            default:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, count));
        }
    }
    void entry(u32 weight, const std::string& stackID, u32 min, u32 max, Modifier modifier = {.type=Modifier::Type::NONE}) {
        this->weights.weights.push_back(LootWeight(weight));
        switch (modifier.type) {
            case Modifier::Type::ENCHANT_RANDOMLY:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, min, max, EnchantRandomlyLootFunction()));
                break;
            case Modifier::Type::ENCHANT_WITH_LEVELS:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, min, max, EnchantWithLevelsLootFunction(modifier.levels)));
                break;
            default:
                this->entries.push_back(std::make_unique<ItemEntry>(stackID, min, max));
        }
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
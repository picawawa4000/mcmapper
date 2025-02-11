#include <mcmapper/biometree.hpp>

#include <memory>
#include <type_traits>

std::string paramsToString(std::vector<ParameterRange> parameters) {
    //doing this the ugly way because it's a debug function and I don't care
    std::string out = "";
    for (int i = 0; i < 7; ++i) {
        out += parameters[i];
        if (i != 6) out += ", ";
    }
    return out;
}

i64 getSquaredDistance(std::vector<ParameterRange> parameters, std::vector<i64> point) {
    i64 out = 0;
    for (int i = 0; i < 7; ++i) {
        i64 n = parameters[i].getDistance(point[i]);
        out += n * n;
    }
    return out;
}

TreeNode TreeNode::getResultingNode(std::vector<i64> point, const TreeNode& alternative) {
    //this is how we define leaf nodes
    if (this->value) return *this;
    i64 retDistance = alternative.value ? getSquaredDistance(alternative.parameters, point) : std::numeric_limits<i64>::max();
    TreeNode ret = alternative;
    for (int i = 0; i < this->children.size(); ++i) {
        i64 distance = getSquaredDistance(this->children[i]->parameters, point);
        //std::cout << "ret = " << biomeRepr(ret.value) << ", retDistance = " << retDistance << ", value = " << biomeRepr(this->children[i]->value) << ", distance = " << distance << ", params = " << paramsToString(this->children[i]->parameters) << std::endl;
        if (retDistance < distance) continue;
        TreeNode endNode = this->children[i]->getResultingNode(point, alternative);
        if (!endNode.value) throw std::runtime_error("Expected leaf node, but found branch node!");
        i64 endDistance = (endNode.value == this->children[i]->value) ? distance : getSquaredDistance(endNode.parameters, point);
        if (retDistance < endDistance) continue;
        retDistance = endDistance;
        ret = endNode;
    }
    //std::cout << "returning " << biomeRepr(ret.value) << " with distance " << retDistance << std::endl;
    return ret;
}

std::vector<ParameterRange> getEnclosingParameters(std::vector<std::shared_ptr<TreeNode>> nodes) {
    if (nodes.size() == 0) throw std::runtime_error("expected nodes, but found none!");
    std::vector<std::optional<ParameterRange>> res = {{}, {}, {}, {}, {}, {}, {}};
    for (int i = 0; i < nodes.size(); ++i) for (int j = 0; j < 7; ++j) res[j] = nodes[i]->parameters[j].combine(res[j]);
    std::vector<ParameterRange> out(7);
    for (int i = 0; i < 7; ++i) out[i] = res[i].value();
    return out;
}

void sortTree(std::vector<std::shared_ptr<TreeNode>> children, i32 currentParameter, bool abs) {
    std::sort(children.begin(), children.end(), [currentParameter, abs](std::shared_ptr<TreeNode> a, std::shared_ptr<TreeNode> b){
        for (int i = 0; i < 7; ++i) {
            i64 aOut = (a->parameters[(currentParameter + i) % 7].min + a->parameters[(currentParameter + i) % 7].max) / 2;
            i64 bOut = (b->parameters[(currentParameter + i) % 7].min + b->parameters[(currentParameter + i) % 7].max) / 2;
            if ((abs ? std::abs(aOut) : aOut) > (abs ? std::abs(bOut) : bOut)) return true;
            if ((abs ? std::abs(aOut) : aOut) < (abs ? std::abs(bOut) : bOut)) return false;
        }
        return false;
    });
}

std::vector<std::shared_ptr<TreeNode>> batchTree(std::vector<std::shared_ptr<TreeNode>> children) {
    std::vector<std::shared_ptr<TreeNode>> ret, inner;
    int count = std::pow(6.0, std::floor(std::log(children.size() - 0.01) / std::log(6.0)));
    for (int i = 0; i < children.size(); ++i) {
        inner.push_back(children[i]);
        if (inner.size() < count) continue;
        ret.emplace_back(new TreeNode(getEnclosingParameters(inner), THE_VOID, inner));
        inner = {};
    }
    if (!inner.empty()) ret.emplace_back(new TreeNode(getEnclosingParameters(inner), THE_VOID, inner));
    return ret;
}

std::shared_ptr<TreeNode> createNode(std::vector<std::shared_ptr<TreeNode>> children) {
    if (children.empty()) throw std::runtime_error("can't create tree node without entries!");
    if (children.size() == 1) {
        return children[0];
    }
    if (children.size() <= 6) {
        std::sort(children.begin(), children.end(), [](std::shared_ptr<TreeNode> a, std::shared_ptr<TreeNode> b){
            i64 aOut = 0, bOut = 0;
            for (int i = 0; i < 7; ++i) {
                aOut += std::abs((a->parameters[i].min + a->parameters[i].max) / 2);
                bOut += std::abs((b->parameters[i].min + b->parameters[i].max) / 2);
            }
            return aOut > bOut;
        });
        return std::shared_ptr<TreeNode>(new TreeNode(getEnclosingParameters(children), THE_VOID, children));
    }
    i64 span = std::numeric_limits<i64>::max();
    i32 param = -1;
    std::vector<std::shared_ptr<TreeNode>> out;
    for (int i = 0; i < 7; ++i) {
        sortTree(children, i, false);
        std::vector<std::shared_ptr<TreeNode>> batched = batchTree(children);
        i64 innerSpan = 0;
        for (int j = 0; j < batched.size(); ++j) for (int k = 0; k < 7; ++k) innerSpan += batched[j]->parameters[k].max - batched[j]->parameters[k].min;
        if (innerSpan >= span) continue;
        span = innerSpan;
        param = i;
        out = batched;
    }
    sortTree(out, param, true);
    std::vector<std::shared_ptr<TreeNode>> ret(out.size());
    for (int i = 0; i < out.size(); ++i) ret[i] = std::shared_ptr<TreeNode>(createNode(out[i].get()->children));
    return std::shared_ptr<TreeNode>(new TreeNode(getEnclosingParameters(ret), THE_VOID, ret));
}

//doing it this way isn't as useful but it still helps knock out quite a couple of useless entries
static TreeNode lastResult({}, THE_VOID, {});

SearchTree::SearchTree(std::vector<std::pair<NoiseHypercube, Biome>> entries) {
    if (entries.empty()) throw std::runtime_error("can't build search tree without entries!");
    std::vector<std::shared_ptr<TreeNode>> convertedList(entries.size());
    for (int i = 0; i < entries.size(); ++i) convertedList[i] = std::shared_ptr<TreeNode>(new TreeNode(entries[i].first.toList(), entries[i].second, std::vector<std::shared_ptr<TreeNode>>(0)));
    this->root = createNode(convertedList);
}

Biome SearchTree::get(NoisePoint point) const {
    TreeNode ret = this->root->getResultingNode(point.toList(), lastResult);
    lastResult = ret;
    return ret.value;
}

static const ParameterRange * combine(const ParameterRange * a, const ParameterRange * b) {
    return new ParameterRange(std::min(a->min, b->min), std::max(a->max, b->max));
}

static constexpr ParameterRange defaultRange = ParameterRange(-1.f, 1.f);

static const std::array<const ParameterRange *, 5> temperatureParameters = {new ParameterRange(-1.f, -0.45f), new ParameterRange(-0.45f, -0.15f), new ParameterRange(-0.15f, 0.2f), new ParameterRange(0.2f, 0.55f), new ParameterRange(0.55f, 1.f)};
static const std::array<const ParameterRange *, 5> humidityParameters = {new ParameterRange(-1.f, -0.35f), new ParameterRange(-0.35f, -0.1f), new ParameterRange(-0.1f, 0.1f), new ParameterRange(0.1f, 0.3f), new ParameterRange(0.3f, 1.f)};
static const std::array<const ParameterRange *, 7> erosionParameters = {new ParameterRange(-1.f, -0.78f), new ParameterRange(-0.78f, -0.375f), new ParameterRange(-0.375f, -0.2225f), new ParameterRange(-0.2225f, -0.05f), new ParameterRange(0.05f, 0.45f), new ParameterRange(0.45f, 0.55f), new ParameterRange(0.55f, 1.f)};

static const ParameterRange * nonFrozenParameters = new ParameterRange(-0.45f, 1.f);

static const ParameterRange * coastContinentalness = new ParameterRange(-0.19f, -0.11f);
static const ParameterRange * riverContinentalness = new ParameterRange(-0.11f, 0.55f);
static const ParameterRange * nearInlandContinentalness = new ParameterRange(-0.11f, 0.03f);
static const ParameterRange * midInlandContinentalness = new ParameterRange(0.03f, 0.3f);
static const ParameterRange * farInlandContinentalness = new ParameterRange(0.3f, 1.0f);

static const std::vector<const std::array<Biome, 5>> oceanBiomes = {{DEEP_FROZEN_OCEAN, DEEP_COLD_OCEAN, DEEP_OCEAN, DEEP_LUKEWARM_OCEAN, WARM_OCEAN}, {FROZEN_OCEAN, COLD_OCEAN, OCEAN, LUKEWARM_OCEAN, WARM_OCEAN}};
static const std::vector<const std::array<Biome, 5>> commonBiomes = {{SNOWY_PLAINS, SNOWY_PLAINS, SNOWY_PLAINS, SNOWY_TAIGA, TAIGA}, {PLAINS, PLAINS, FOREST, TAIGA, OLD_GROWTH_SPRUCE_TAIGA}, {FLOWER_FOREST, PLAINS, FOREST, BIRCH_FOREST, DARK_FOREST}, {SAVANNA, SAVANNA, FOREST, JUNGLE, JUNGLE}, {DESERT, DESERT, DESERT, DESERT, DESERT}};
static const std::vector<const std::array<Biome, 5>> uncommonBiomes = {{ICE_SPIKES, THE_VOID, SNOWY_TAIGA, THE_VOID, THE_VOID}, {THE_VOID, THE_VOID, THE_VOID, THE_VOID, OLD_GROWTH_PINE_TAIGA}, {SUNFLOWER_PLAINS, THE_VOID, THE_VOID, OLD_GROWTH_BIRCH_FOREST, THE_VOID}, {THE_VOID, THE_VOID, PLAINS, SPARSE_JUNGLE, BAMBOO_JUNGLE}, {THE_VOID, THE_VOID, THE_VOID, THE_VOID, THE_VOID}};
static const std::vector<const std::array<Biome, 5>> nearMountainBiomes = {{SNOWY_PLAINS, SNOWY_PLAINS, SNOWY_PLAINS, SNOWY_TAIGA, SNOWY_TAIGA}, {MEADOW, MEADOW, FOREST, TAIGA, OLD_GROWTH_SPRUCE_TAIGA}, {MEADOW, MEADOW, MEADOW, MEADOW, DARK_FOREST}, {SAVANNA_PLATEAU, SAVANNA_PLATEAU, FOREST, FOREST, JUNGLE}, {BADLANDS, BADLANDS, BADLANDS, WOODED_BADLANDS, WOODED_BADLANDS}};
static const std::vector<const std::array<Biome, 5>> specialNearMountainBiomes = {{ICE_SPIKES, THE_VOID, THE_VOID, THE_VOID, THE_VOID}, {CHERRY_GROVE, THE_VOID, MEADOW, MEADOW, OLD_GROWTH_PINE_TAIGA}, {CHERRY_GROVE, CHERRY_GROVE, FOREST, BIRCH_FOREST, PALE_GARDEN}, {THE_VOID, THE_VOID, THE_VOID, THE_VOID, THE_VOID}, {ERODED_BADLANDS, ERODED_BADLANDS, THE_VOID, THE_VOID, THE_VOID}};
static const std::vector<const std::array<Biome, 5>> windsweptBiomes = {{WINDSWEPT_GRAVELLY_HILLS, WINDSWEPT_GRAVELLY_HILLS, WINDSWEPT_HILLS, WINDSWEPT_FOREST, WINDSWEPT_FOREST}, {WINDSWEPT_GRAVELLY_HILLS, WINDSWEPT_GRAVELLY_HILLS, WINDSWEPT_HILLS, WINDSWEPT_FOREST, WINDSWEPT_FOREST}, {WINDSWEPT_HILLS, WINDSWEPT_HILLS, WINDSWEPT_HILLS, WINDSWEPT_FOREST, WINDSWEPT_FOREST}, {THE_VOID, THE_VOID, THE_VOID, THE_VOID, THE_VOID}, {THE_VOID, THE_VOID, THE_VOID, THE_VOID, THE_VOID}};

static inline Biome getRegularBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    if (weirdness->max < 0) {
        return commonBiomes[temperature][humidity];
    }
    Biome uncommon = uncommonBiomes[temperature][humidity];
    return uncommon == THE_VOID ? commonBiomes[temperature][humidity] : uncommonBiomes[temperature][humidity];
}

static inline Biome getBadlandsBiome(int humidity, const ParameterRange * weirdness) {
    if (humidity < 2) return weirdness->max < 0 ? BADLANDS : ERODED_BADLANDS;
    if (humidity < 3) return BADLANDS;
    return WOODED_BADLANDS;
}

static inline Biome getBadlandsOrRegularBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    return temperature == 4 ? getBadlandsBiome(humidity, weirdness) : getRegularBiome(temperature, humidity, weirdness);
}

static inline Biome getNearMountainBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    if (weirdness->max > 0 and specialNearMountainBiomes[temperature][humidity] != THE_VOID) return specialNearMountainBiomes[temperature][humidity];
    return nearMountainBiomes[temperature][humidity];
}

static inline Biome getMountainSlopeBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    if (temperature >= 3) return getNearMountainBiome(temperature, humidity, weirdness);
    if (humidity <= 1) return SNOWY_SLOPES;
    return GROVE;
}

static inline Biome getMountainStartBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    return temperature == 0 ? getMountainSlopeBiome(temperature, humidity, weirdness) : getBadlandsOrRegularBiome(temperature, humidity, weirdness);
}

static inline Biome getShoreBiome(int temperature) {
    return temperature == 0 ? SNOWY_BEACH : (temperature == 4 ? DESERT : BEACH);
}

static inline Biome getBiomeOrWindsweptSavanna(int temperature, int humidity, const ParameterRange * weirdness, Biome alt) {
    return (temperature > 1 and humidity < 4 and weirdness->max >= 0) ? WINDSWEPT_SAVANNA : alt;
}

static inline Biome getErodedShoreBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    Biome alt = weirdness->max >= 0 ? getRegularBiome(temperature, humidity, weirdness) : getShoreBiome(temperature);
    return getBiomeOrWindsweptSavanna(temperature, humidity, weirdness, alt);
}

static inline Biome getWindsweptOrRegularBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    Biome alt = windsweptBiomes[temperature][humidity];
    return alt == THE_VOID ? getRegularBiome(temperature, humidity, weirdness) : alt;
}

static inline Biome getPeakBiome(int temperature, int humidity, const ParameterRange * weirdness) {
    if (temperature <= 2) return weirdness->max < 0 ? JAGGED_PEAKS : FROZEN_PEAKS;
    if (temperature == 3) return STONY_PEAKS;
    return getBadlandsBiome(humidity, weirdness);
}

static void enterValleyBiomes(std::function<void(const std::array<const ParameterRange *,5>&,i64,Biome)> enter, const ParameterRange * weirdness) {
    enter({temperatureParameters[0], &defaultRange, coastContinentalness, combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, weirdness->max < 0 ? STONY_SHORE : FROZEN_RIVER);
    enter({nonFrozenParameters, &defaultRange, coastContinentalness, combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, weirdness->max < 0 ? STONY_SHORE : RIVER);
    enter({temperatureParameters[0], &defaultRange, nearInlandContinentalness, combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, FROZEN_RIVER);
    enter({nonFrozenParameters, &defaultRange, nearInlandContinentalness, combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, RIVER);
    enter({temperatureParameters[0], &defaultRange, combine(nearInlandContinentalness, farInlandContinentalness), combine(erosionParameters[2], erosionParameters[5]), weirdness}, 0, FROZEN_RIVER);
    enter({nonFrozenParameters, &defaultRange, combine(nearInlandContinentalness, farInlandContinentalness), combine(erosionParameters[2], erosionParameters[5]), weirdness}, 0, RIVER);
    enter({temperatureParameters[0], &defaultRange, coastContinentalness, erosionParameters[6], weirdness}, 0, FROZEN_RIVER);
    enter({nonFrozenParameters, &defaultRange, coastContinentalness, erosionParameters[6], weirdness}, 0, RIVER);
    enter({combine(temperatureParameters[1], temperatureParameters[2]), &defaultRange, combine(riverContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, SWAMP);
    enter({combine(temperatureParameters[3], temperatureParameters[4]), &defaultRange, combine(riverContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, MANGROVE_SWAMP);
    enter({temperatureParameters[0], &defaultRange, combine(riverContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, FROZEN_RIVER);
    for (int i = 0; i < temperatureParameters.size(); ++i) {
        const ParameterRange * temperature = temperatureParameters[i];
        for (int j = 0; j < humidityParameters.size(); ++j) {
            const ParameterRange * humidity = humidityParameters[j];
            Biome biome = getBadlandsOrRegularBiome(i, j, weirdness);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, biome);
        }
    }
}

static void enterLowBiomes(std::function<void(const std::array<const ParameterRange *,5>&,i64,Biome)> enter, const ParameterRange * weirdness) {
    enter({temperatureParameters[0], &defaultRange, coastContinentalness, combine(erosionParameters[0], erosionParameters[2]), weirdness}, 0, STONY_SHORE);
    enter({combine(temperatureParameters[3], temperatureParameters[4]), &defaultRange, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, SWAMP);
    enter({combine(temperatureParameters[3], temperatureParameters[4]), &defaultRange, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, SWAMP);
    for (int i = 0; i < temperatureParameters.size(); ++i) {
        const ParameterRange * temperature = temperatureParameters[i];
        for (int j = 0; j < humidityParameters.size(); ++j) {
            const ParameterRange * humidity = humidityParameters[j];
            Biome regular = getRegularBiome(i, j, weirdness);
            Biome badlandsOrRegular = getBadlandsOrRegularBiome(i, j, weirdness);
            Biome mountainStart = getMountainStartBiome(i, j, weirdness);
            Biome shore = getShoreBiome(i);
            Biome regularOrWindsweptSavanna = getBiomeOrWindsweptSavanna(i, j, weirdness, regular);
            Biome erodedShore = getErodedShoreBiome(i, j, weirdness);
            enter({temperature, humidity, nearInlandContinentalness, combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, badlandsOrRegular);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, mountainStart);
            enter({temperature, humidity, nearInlandContinentalness, combine(erosionParameters[2], erosionParameters[3]), weirdness}, 0, regular);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), combine(erosionParameters[2], erosionParameters[3]), weirdness}, 0, badlandsOrRegular);
            enter({temperature, humidity, coastContinentalness, erosionParameters[4], weirdness}, 0, shore);
            enter({temperature, humidity, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[4], weirdness}, 0, regular);
            enter({temperature, humidity, coastContinentalness, erosionParameters[5], weirdness}, 0, erodedShore);
            enter({temperature, humidity, nearInlandContinentalness, erosionParameters[5], weirdness}, 0, regularOrWindsweptSavanna);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[5], weirdness}, 0, regular);
            enter({temperature, humidity, coastContinentalness, erosionParameters[6], weirdness}, 0, shore);
            if (i != 0) continue;
            enter({temperature, humidity, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, regular);
        }
    }
}

static void enterMidBiomes(std::function<void(const std::array<const ParameterRange *,5>&,i64,Biome)> enter, const ParameterRange * weirdness) {
    enter({&defaultRange, &defaultRange, coastContinentalness, combine(erosionParameters[0], erosionParameters[2]), weirdness}, 0, STONY_SHORE);
    enter({combine(temperatureParameters[1], temperatureParameters[2]), &defaultRange, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, SWAMP);
    enter({combine(temperatureParameters[3], temperatureParameters[4]), &defaultRange, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, MANGROVE_SWAMP);
    for (int i = 0; i < temperatureParameters.size(); ++i) {
        const ParameterRange * temperature = temperatureParameters[i];
        for (int j = 0; j < humidityParameters.size(); ++j) {
            const ParameterRange * humidity = humidityParameters[j];
            Biome regular = getRegularBiome(i, j, weirdness);
            Biome badlandsOrRegular = getBadlandsOrRegularBiome(i, j, weirdness);
            Biome mountainStart = getMountainStartBiome(i, j, weirdness);
            Biome windsweptOrRegular = getWindsweptOrRegularBiome(i, j, weirdness);
            Biome nearMountain = getNearMountainBiome(i, j, weirdness);
            Biome shore = getShoreBiome(i);
            Biome regularOrWindsweptSavanna = getBiomeOrWindsweptSavanna(i, j, weirdness, regular);
            Biome erodedShore = getErodedShoreBiome(i, j, weirdness);
            Biome mountainSlope = getMountainSlopeBiome(i, j, weirdness);
            enter({temperature, humidity, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[0], weirdness}, 0, mountainSlope);
            enter({temperature, humidity, combine(nearInlandContinentalness, midInlandContinentalness), erosionParameters[1], weirdness}, 0, mountainStart);
            enter({temperature, humidity, farInlandContinentalness, erosionParameters[1], weirdness}, 0, i == 0 ? mountainSlope : nearMountain);
            enter({temperature, humidity, nearInlandContinentalness, erosionParameters[2], weirdness}, 0, regular);
            enter({temperature, humidity, midInlandContinentalness, erosionParameters[2], weirdness}, 0, badlandsOrRegular);
            enter({temperature, humidity, farInlandContinentalness, erosionParameters[2], weirdness}, 0, nearMountain);
            enter({temperature, humidity, combine(coastContinentalness, nearInlandContinentalness), erosionParameters[3], weirdness}, 0, regular);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[3], weirdness}, 0, badlandsOrRegular);
            if (weirdness->max < 0) {
                enter({temperature, humidity, coastContinentalness, erosionParameters[4], weirdness}, 0, shore);
                enter({temperature, humidity, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[4], weirdness}, 0, regular);
            } else
                enter({temperature, humidity, combine(coastContinentalness, farInlandContinentalness), erosionParameters[4], weirdness}, 0, regular);
            enter({temperature, humidity, coastContinentalness, erosionParameters[5], weirdness}, 0, erodedShore);
            enter({temperature, humidity, nearInlandContinentalness, erosionParameters[5], weirdness}, 0, regularOrWindsweptSavanna);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[5], weirdness}, 0, windsweptOrRegular);
            if (weirdness->max < 0)
                enter({temperature, humidity, coastContinentalness, erosionParameters[6], weirdness}, 0, shore);
            else
                enter({temperature, humidity, coastContinentalness, erosionParameters[6], weirdness}, 0, regular);
            if (i != 0) continue;
            enter({temperature, humidity, combine(nearInlandContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, regular);
        }
    }
}

static void enterHighBiomes(std::function<void(const std::array<const ParameterRange *,5>&,i64,Biome)> enter, const ParameterRange * weirdness) {
    for (int i = 0; i < temperatureParameters.size(); ++i) {
        const ParameterRange * temperature = temperatureParameters[i];
        for (int j = 0; j < humidityParameters.size(); ++j) {
            const ParameterRange * humidity = humidityParameters[j];
            Biome regular = getRegularBiome(i, j, weirdness);
            Biome badlandsOrRegular = getBadlandsOrRegularBiome(i, j, weirdness);
            Biome mountainStart = getMountainStartBiome(i, j, weirdness);
            Biome nearMountainBiome = getNearMountainBiome(i, j, weirdness);
            Biome windsweptOrRegular = getWindsweptOrRegularBiome(i, j, weirdness);
            Biome regularOrWindsweptSavanna = getBiomeOrWindsweptSavanna(i, j, weirdness, regular);
            Biome mountainSlope = getMountainSlopeBiome(i, j, weirdness);
            Biome peak = getPeakBiome(i, j, weirdness);
            enter({temperature, humidity, coastContinentalness, combine(erosionParameters[0], erosionParameters[1]), weirdness}, 0, regular);
            enter({temperature, humidity, nearInlandContinentalness, erosionParameters[0], weirdness}, 0, mountainSlope);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[0], weirdness}, 0, peak);
            enter({temperature, humidity, nearInlandContinentalness, erosionParameters[1], weirdness}, 0, mountainStart);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[1], weirdness}, 0, mountainSlope);
            enter({temperature, humidity, combine(coastContinentalness, nearInlandContinentalness), combine(erosionParameters[2], erosionParameters[3]), weirdness}, 0, regular);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[2], weirdness}, 0, nearMountainBiome);
            enter({temperature, humidity, midInlandContinentalness, erosionParameters[3], weirdness}, 0, badlandsOrRegular);
            enter({temperature, humidity, farInlandContinentalness, erosionParameters[3], weirdness}, 0, nearMountainBiome);
            enter({temperature, humidity, combine(coastContinentalness, farInlandContinentalness), erosionParameters[4], weirdness}, 0, regular);
            enter({temperature, humidity, combine(coastContinentalness, nearInlandContinentalness), erosionParameters[5], weirdness}, 0, regularOrWindsweptSavanna);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[5], weirdness}, 0, windsweptOrRegular);
            enter({temperature, humidity, combine(coastContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, regular);
        }
    }
}

static void enterPeakBiomes(std::function<void(const std::array<const ParameterRange *,5>&,i64,Biome)> enter, const ParameterRange * weirdness) {
    for (int i = 0; i < temperatureParameters.size(); ++i) {
        const ParameterRange * temperature = temperatureParameters[i];
        for (int j = 0; j < humidityParameters.size(); ++j) {
            const ParameterRange * humidity = humidityParameters[j];
            Biome regular = getRegularBiome(i, j, weirdness);
            Biome badlandsOrRegular = getBadlandsOrRegularBiome(i, j, weirdness);
            Biome mountainStart = getMountainStartBiome(i, j, weirdness);
            Biome nearMountainBiome = getNearMountainBiome(i, j, weirdness);
            Biome windsweptOrRegular = getWindsweptOrRegularBiome(i, j, weirdness);
            Biome regularOrWindsweptSavanna = getBiomeOrWindsweptSavanna(i, j, weirdness, windsweptOrRegular);
            Biome peak = getPeakBiome(i, j, weirdness);
            enter({temperature, humidity, combine(coastContinentalness, farInlandContinentalness), erosionParameters[0], weirdness}, 0, peak);
            enter({temperature, humidity, combine(coastContinentalness, nearInlandContinentalness), erosionParameters[1], weirdness}, 0, mountainStart);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[1], weirdness}, 0, peak);
            enter({temperature, humidity, combine(coastContinentalness, nearInlandContinentalness), combine(erosionParameters[2], erosionParameters[3]), weirdness}, 0, regular);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[2], weirdness}, 0, nearMountainBiome);
            enter({temperature, humidity, midInlandContinentalness, erosionParameters[3], weirdness}, 0, badlandsOrRegular);
            enter({temperature, humidity, farInlandContinentalness, erosionParameters[3], weirdness}, 0, nearMountainBiome);
            enter({temperature, humidity, combine(coastContinentalness, farInlandContinentalness), erosionParameters[4], weirdness}, 0, regular);
            enter({temperature, humidity, combine(coastContinentalness, nearInlandContinentalness), erosionParameters[5], weirdness}, 0, regularOrWindsweptSavanna);
            enter({temperature, humidity, combine(midInlandContinentalness, farInlandContinentalness), erosionParameters[5], weirdness}, 0, windsweptOrRegular);
            enter({temperature, humidity, combine(coastContinentalness, farInlandContinentalness), erosionParameters[6], weirdness}, 0, regular);
        }
    }
}

static constexpr SearchTree * genSearchTree() {
    std::vector<std::pair<NoiseHypercube, Biome>> entries;
    auto enter = [&entries](const std::array<const ParameterRange *, 5>& parameters, i64 offset, Biome biome){
        entries.push_back(std::pair<NoiseHypercube, Biome>(NoiseHypercube(*parameters[0], *parameters[1], *parameters[2], *parameters[3], ParameterRange(0.f, 0.f), *parameters[4], offset), biome));
        entries.push_back(std::pair<NoiseHypercube, Biome>(NoiseHypercube(*parameters[0], *parameters[1], *parameters[2], *parameters[3], ParameterRange(1.f, 1.f), *parameters[4], offset), biome));
    };

    //Ocean biomes
    enter({&defaultRange, &defaultRange, new ParameterRange(-1.2f, -1.05f), &defaultRange, &defaultRange}, 0, MUSHROOM_FIELDS);
    for (int i = 0; i < temperatureParameters.size(); ++i) {
        const ParameterRange * temperature = temperatureParameters[i];
        enter({temperature, &defaultRange, new ParameterRange(-1.05f, -0.455f), &defaultRange, &defaultRange}, 0, oceanBiomes[0][i]);
        enter({temperature, &defaultRange, new ParameterRange(-0.455f, -0.19f), &defaultRange, &defaultRange}, 0, oceanBiomes[1][i]);
    }

    //Land biomes
    enterMidBiomes(enter, new ParameterRange(-1.0f, -0.93333334f));
    enterHighBiomes(enter, new ParameterRange(-0.93333334f, -0.7666667f));
    enterPeakBiomes(enter, new ParameterRange(-0.7666667f, -0.56666666f));
    enterHighBiomes(enter, new ParameterRange(-0.56666666f, -0.4f));
    enterMidBiomes(enter, new ParameterRange(-0.4f, -0.26666668f));
    enterLowBiomes(enter, new ParameterRange(-0.26666668f, -0.05f));
    enterValleyBiomes(enter, new ParameterRange(-0.05f, 0.05f));
    enterLowBiomes(enter, new ParameterRange(0.05f, 0.26666668f));
    enterMidBiomes(enter, new ParameterRange(0.26666668f, 0.4f));
    enterHighBiomes(enter, new ParameterRange(0.4f, 0.56666666f));
    enterPeakBiomes(enter, new ParameterRange(0.56666666f, 0.7666667f));
    enterHighBiomes(enter, new ParameterRange(0.7666667f, 0.93333334f));
    enterMidBiomes(enter, new ParameterRange(0.93333334f, 1.0f));

    //Cave biomes
    entries.push_back(std::pair<NoiseHypercube, Biome>(NoiseHypercube(defaultRange, ParameterRange(0.7f, 1.0f), defaultRange, defaultRange, ParameterRange(0.2f, 0.9f), defaultRange, 0), LUSH_CAVES));
    entries.push_back(std::pair<NoiseHypercube, Biome>(NoiseHypercube(defaultRange, defaultRange, ParameterRange(0.8f, 1.0f), defaultRange, ParameterRange(0.2f, 0.9f), defaultRange, 0), DRIPSTONE_CAVES));
    entries.push_back(std::pair<NoiseHypercube, Biome>(NoiseHypercube(defaultRange, defaultRange, defaultRange, *combine(erosionParameters[0], erosionParameters[1]), ParameterRange(1.1f, 1.1f), defaultRange, 0), DEEP_DARK));

    return new SearchTree(entries);
}

const SearchTree * getSearchTree() {
    static constexpr SearchTree * INSTANCE = genSearchTree();
    return INSTANCE;
}
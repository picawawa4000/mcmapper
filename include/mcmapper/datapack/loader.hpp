#ifndef DATAPACK_LOADER_HPP
#define DATAPACK_LOADER_HPP

#include <fstream>
#include <nlohmann/json.hpp>

#include <mcmapper/densityfunction/misc.hpp>
#include <mcmapper/densityfunction/cache.hpp>

#include <unordered_map>

struct DFuncGenInfoCache {
    std::map<std::string, std::shared_ptr<DensityFunction>> namedFuncs;
    // For invalidation
    std::vector<std::shared_ptr<Cache2DFunction>> cache2DFuncs{};
    std::vector<std::shared_ptr<CacheOnceFunction>> cacheOnceFuncs{};
    std::vector<std::shared_ptr<FlatCacheFunction>> flatCacheFuncs{};
    std::vector<std::shared_ptr<CellCacheFunction>> cellCacheFuncs{};
    // For changing the seed
    std::vector<std::shared_ptr<InterpolatedNoise>> interpolatedNoises{};
    std::map<std::string, std::shared_ptr<DoublePerlinNoise>> noisesById;
    // Whether to cache anything at all
    bool useCaches;
    // Whether to convert shifted noises with y_scale = 0 to flat scaled noises
    // Could potentially break some noises with a non-zero shift_y, but this shouldn't be a major problem
    bool useFlatShiftHack = true;
    // Cell size.
    i32 horizontalCellBlockCount, verticalCellBlockCount;
};

inline std::shared_ptr<DensityFunction> makeShiftedNoise(nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    std::shared_ptr<DensityFunction>
        shiftX = makeDensityFunction(jsondata["shift_x"], cache),
        shiftY = makeDensityFunction(jsondata["shift_y"], cache),
        shiftZ = makeDensityFunction(jsondata["shift_z"], cache);
    double xzScale = jsondata["xz_scale"].get<double>();
    double yScale = jsondata["y_scale"].get<double>();
    std::shared_ptr<DoublePerlinNoise> noise = cache.noisesById[jsondata["noise"].get<std::string>()];
    if (cache.useFlatShiftHack && yScale == 0.)
        return std::make_shared<FlatScaledNoiseFunction>(noise, shiftX, shiftZ, xzScale);
    return std::make_shared<ShiftedNoiseFunction>(noise, shiftX, shiftY, shiftZ, xzScale, yScale);
}

inline std::shared_ptr<DensityFunction> makeInterpolatedNoise(nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    double xzScale = jsondata["xz_scale"].get<double>();
    double yScale = jsondata["y_scale"].get<double>();
    double xzFactor = jsondata["xz_factor"].get<double>();
    double yFactor = jsondata["y_factor"].get<double>();
    double smearScaleMultiplier = jsondata["smear_scale_multiplier"].get<double>();
    std::shared_ptr<InterpolatedNoise> noise = std::make_shared<InterpolatedNoise>(XoroshiroRandom(0ULL), xzScale, yScale, xzFactor, yFactor, smearScaleMultiplier);
    cache.interpolatedNoises.push_back(noise);
    return std::make_shared<InterpolatedNoiseFunction>(noise);
}

inline std::shared_ptr<DensityFunction> makeSplineFunction(nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    throw std::runtime_error("unimplemented function makeSplineFunction!");
}

std::shared_ptr<DensityFunction> makeDensityFunction(nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    if (jsondata.is_number())
        return std::make_shared<ConstantFunction>(jsondata.get<double>());
    if (jsondata.is_string())
        return cache.namedFuncs[jsondata.get<std::string>()];

    if (!jsondata["type"].is_string()) throw std::runtime_error("type of density function isn't a string!");
    std::string type = jsondata["type"].get<std::string>();

    if (type == "abs") {
        return std::make_shared<AbsFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "square") {
        return std::make_shared<SquareFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "cube") {
        return std::make_shared<CubeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "half_negative") {
        return std::make_shared<HalfNegativeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "quarter_negative") {
        return std::make_shared<QuarterNegativeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "squeeze") {
        return std::make_shared<SqueezeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "add") {
        return std::make_shared<AddFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "mul") {
        return std::make_shared<MulFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "min") {
        return std::make_shared<MinFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "max") {
        return std::make_shared<MaxFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "blend_alpha") {
        return std::make_shared<ConstantFunction>(1.);
    } else if (type == "blend_offset") {
        return std::make_shared<ConstantFunction>(0.);
    } else if (type == "blend_density") {
        return makeDensityFunction(jsondata["argument"], cache);
    } else if (type == "beardifier") {
        throw std::runtime_error("beardifier should not be referenced in data packs!");
    } else if (type == "end_islands") {
        throw std::runtime_error("No end islands support yet!");
    } else if (type == "noise") {
        return std::make_shared<NoiseFunction>(cache.noisesById[jsondata["noise"].get<std::string>()], jsondata["xz_scale"].get<double>(), jsondata["y_scale"].get<double>());
    } else if (type == "shifted_noise") {
        return makeShiftedNoise(jsondata, cache);
    } else if (type == "old_blended_noise") {
        return makeInterpolatedNoise(jsondata, cache);
    } else if (type == "weird_scaled_sampler") {
        return std::make_shared<WeirdScaledFunction>(makeDensityFunction(jsondata["input"], cache), cache.noisesById[jsondata["noise"].get<std::string>()], jsondata["rarity_value_mapper"].get<std::string>() == "type_1");
    } else if (type == "range_choice") {
        return std::make_shared<RangeChoiceFunction>(makeDensityFunction(jsondata["input"], cache), makeDensityFunction(jsondata["when_in_range"], cache), makeDensityFunction(jsondata["when_out_of_range"], cache), jsondata["min_inclusive"].get<double>(), jsondata["max_exclusive"].get<double>());
    } else if (type == "shift_a") {
        return std::make_shared<ShiftAFunction>(cache.noisesById[jsondata["argument"].get<std::string>()]);
    } else if (type == "shift_b") {
        return std::make_shared<ShiftBFunction>(cache.noisesById[jsondata["argument"].get<std::string>()]);
    } else if (type == "shift") {
        return std::make_shared<ShiftFunction>(cache.noisesById[jsondata["argument"].get<std::string>()]);
    } else if (type == "clamp") {
        return std::make_shared<ClampFunction>(makeDensityFunction(jsondata["input"], cache), jsondata["min"].get<double>(), jsondata["max"].get<double>());
    } else if (type == "constant") {
        return std::make_shared<ConstantFunction>(jsondata["argument"].get<double>());
    } else if (type == "y_clamped_gradient") {
        return std::make_shared<YGradientFunction>(jsondata["from_y"].get<i32>(), jsondata["to_y"].get<i32>(), jsondata["from_value"].get<double>(), jsondata["to_value"].get<double>());
    } else if (type == "spline") {
        return makeSplineFunction(jsondata, cache);
    } else if (type == "interpolated") {
        return std::make_shared<InterpolatedFunction>(makeDensityFunction(jsondata["argument"], cache), cache.horizontalCellBlockCount, cache.verticalCellBlockCount, cache.useCaches);
    } else if (type == "flat_cache") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<FlatCacheFunction> ret = std::make_shared<FlatCacheFunction>(makeDensityFunction(jsondata["argument"], cache));
        cache.flatCacheFuncs.push_back(ret);
        return ret;
    } else if (type == "cache_2d") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<Cache2DFunction> ret = std::make_shared<Cache2DFunction>(makeDensityFunction(jsondata["argument"], cache));
        cache.cache2DFuncs.push_back(ret);
        return ret;
    } else if (type == "cache_all_in_cell") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<CellCacheFunction> ret = std::make_shared<CellCacheFunction>(makeDensityFunction(jsondata["argument"], cache), cache.horizontalCellBlockCount, cache.verticalCellBlockCount);
        cache.cellCacheFuncs.push_back(ret);
        return ret;
    } else if (type == "cache_once") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<CacheOnceFunction> ret = std::make_shared<CacheOnceFunction>(makeDensityFunction(jsondata["argument"], cache));
        cache.cacheOnceFuncs.push_back(ret);
        return ret;
    }

    throw std::runtime_error("unrecognised function type " + type);
}

std::shared_ptr<DensityFunction> loadDensityFunction(std::string filepath, std::string id, DFuncGenInfoCache& infoCache) {
    std::ifstream file(filepath);
    nlohmann::json jsondata = nlohmann::json::parse(file);
    
    std::shared_ptr<DensityFunction> function = makeDensityFunction(jsondata, infoCache);
    infoCache.namedFuncs[id] = function;
}

#endif
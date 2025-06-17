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
    std::vector<std::shared_ptr<Cache2DFunction>> cache2DFuncs;
    std::vector<std::shared_ptr<CacheOnceFunction>> cacheOnceFuncs;
    std::vector<std::shared_ptr<FlatCacheFunction>> flatCacheFuncs;
    std::vector<std::shared_ptr<CellCacheFunction>> cellCacheFuncs;
    // For changing the seed
    std::vector<std::shared_ptr<InterpolatedNoise>> interpolatedNoises;
    std::map<std::string, std::shared_ptr<DoublePerlinNoise>> noisesById;
    // Whether to cache anything at all
    bool useCaches;
};

std::shared_ptr<DensityFunction> makeDensityFunction(nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    if (jsondata.is_number())
        return std::make_shared<ConstantFunction>(jsondata.template get<double>());

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
    }
}

std::shared_ptr<DensityFunction> loadDensityFunction(std::string filepath, std::string id, DFuncGenInfoCache& infoCache) {
    std::ifstream file(filepath);
    nlohmann::json jsondata = nlohmann::json::parse(file);
    
    std::shared_ptr<DensityFunction> function = makeDensityFunction(jsondata, infoCache);
    infoCache.namedFuncs[id] = function;
}

#endif
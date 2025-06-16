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
    std::vector<std::shared_ptr<DoublePerlinNoise>> noises;
};

std::shared_ptr<DensityFunction> makeDensityFunction(nlohmann::json jsondata) {
    if (jsondata.is_number())
        return std::make_shared<ConstantFunction>(jsondata.template get<double>());

    if (!jsondata["type"].is_string()) throw std::runtime_error("type of density function isn't a string!");
    std::string type = jsondata["type"].template get<std::string>();

    
}

std::shared_ptr<DensityFunction> loadDensityFunction(std::string filepath, std::string id, DFuncGenInfoCache& infoCache) {
    std::ifstream file(filepath);
    nlohmann::json jsondata = nlohmann::json::parse(file);
    
    std::shared_ptr<DensityFunction> function = makeDensityFunction(jsondata);
    infoCache.namedFuncs[id] = function;
}

#endif
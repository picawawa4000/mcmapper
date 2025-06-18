#ifndef DATAPACK_LOADER_HPP
#define DATAPACK_LOADER_HPP

#include <fstream>
#include <nlohmann/json.hpp>

#include <mcmapper/densityfunction/misc.hpp>
#include <mcmapper/densityfunction/cache.hpp>

#include <unordered_map>

struct DataPack {
    std::map<std::string, std::shared_ptr<DoublePerlinNoise>> noises;
    std::map<std::string, std::shared_ptr<DensityFunction>> densityFunctions;
};

struct DFuncGenInfoCache {
    std::map<std::string, std::shared_ptr<DensityFunction>> * namedFuncs;
    // For invalidation
    std::vector<std::shared_ptr<Cache2DFunction>> cache2DFuncs;
    std::vector<std::shared_ptr<CacheOnceFunction>> cacheOnceFuncs;
    std::vector<std::shared_ptr<FlatCacheFunction>> flatCacheFuncs;
    std::vector<std::shared_ptr<CellCacheFunction>> cellCacheFuncs;
    // For changing the seed
    std::vector<std::shared_ptr<InterpolatedNoise>> interpolatedNoises;
    std::map<std::string, std::shared_ptr<DoublePerlinNoise>> * noisesById;
    // Whether to cache anything at all
    bool useCaches;
    // Whether to convert shifted noises with y_scale = 0 to flat scaled noises
    // Could potentially break some noises with a non-zero shift_y, but this shouldn't be a major problem
    bool useFlatShiftHack = true;
    // Cell size.
    i32 horizontalCellBlockCount, verticalCellBlockCount;
};

std::shared_ptr<DensityFunction> loadDensityFunction(std::istream& file, DFuncGenInfoCache& infoCache);

/// @brief Loads a data pack.
/// @param rootpath The root path of the data pack. Should be a path
/// to the folder containing the `pack.mcmeta` file.
/// @return A data pack object.
DataPack loadDataPack(std::string rootpath);

#endif
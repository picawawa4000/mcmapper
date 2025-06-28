#include <mcmapper/datapack/loader.hpp>

#include <filesystem>
#include <iostream>

static std::shared_ptr<DensityFunction> makeDensityFunction(const nlohmann::json& jsondata, DFuncGenInfoCache& cache);

static inline std::shared_ptr<DensityFunction> makeShiftedNoise(const nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    std::shared_ptr<DensityFunction>
        shiftX = makeDensityFunction(jsondata["shift_x"], cache),
        shiftY = makeDensityFunction(jsondata["shift_y"], cache),
        shiftZ = makeDensityFunction(jsondata["shift_z"], cache);
    double xzScale = jsondata["xz_scale"].get<double>();
    double yScale = jsondata["y_scale"].get<double>();
    std::shared_ptr<DoublePerlinNoise> noise = cache.noisesById->at(jsondata["noise"].get<std::string>());
    if (cache.useFlatShiftHack && yScale == 0.)
        return std::make_shared<FlatScaledNoiseFunction>(noise, shiftX, shiftZ, xzScale);
    return std::make_shared<ShiftedNoiseFunction>(noise, shiftX, shiftY, shiftZ, xzScale, yScale);
}

static inline std::shared_ptr<DensityFunction> makeInterpolatedNoise(const nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    double xzScale = jsondata["xz_scale"].get<double>();
    double yScale = jsondata["y_scale"].get<double>();
    double xzFactor = jsondata["xz_factor"].get<double>();
    double yFactor = jsondata["y_factor"].get<double>();
    double smearScaleMultiplier = jsondata["smear_scale_multiplier"].get<double>();
    XoroshiroRandom rng(0);
    std::shared_ptr<InterpolatedNoise> noise = std::make_shared<InterpolatedNoise>(rng, xzScale, yScale, xzFactor, yFactor, smearScaleMultiplier);
    cache.interpolatedNoises.push_back(noise);
    return std::make_shared<InterpolatedNoiseFunction>(noise);
}

static inline SplineFunction makeSplineFunctionImpl(const nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    std::shared_ptr<DensityFunction> coordinateFunction = makeDensityFunction(jsondata["coordinate"], cache);
    SplineFunction ret(coordinateFunction);

    nlohmann::json points = jsondata["points"];
    for (nlohmann::json& point : points) {
        nlohmann::json jsonValue = point["value"];
        std::variant<float, SplineFunction> value;
        if (jsonValue.is_number()) value = jsonValue.get<float>();
        else value = makeSplineFunctionImpl(jsonValue, cache);
        ret.addPoint(point["location"], value, point["derivative"]);
    }

    return ret;
}

static inline std::shared_ptr<DensityFunction> makeSplineFunction(const nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    nlohmann::json splinedata = jsondata["spline"];
    if (splinedata.is_number())
        return std::make_shared<ConstantFunction>(splinedata.get<float>());
    
    SplineFunction func = makeSplineFunctionImpl(jsondata["spline"], cache);
    /// TODO: slow hack
    return std::make_shared<SplineFunction>(func);
}

static std::shared_ptr<DensityFunction> makeDensityFunction(const nlohmann::json& jsondata, DFuncGenInfoCache& cache) {
    if (jsondata.is_number())
        return std::make_shared<ConstantFunction>(jsondata.get<double>());
    if (jsondata.is_string()) {
        std::string id = jsondata.get<std::string>();
        if (!cache.namedFuncs->contains(id))
            cache.namedFuncs->insert({id, std::make_shared<ErrorFunction>("Attempted call to uninitialised function " + id)});
        return cache.namedFuncs->at(id);
    }

    if (!jsondata["type"].is_string()) throw std::runtime_error("type of density function isn't a string!");
    std::string type = jsondata["type"].get<std::string>();

    if (type == "minecraft:abs") {
        return std::make_shared<AbsFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "minecraft:square") {
        return std::make_shared<SquareFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "minecraft:cube") {
        return std::make_shared<CubeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "minecraft:half_negative") {
        return std::make_shared<HalfNegativeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "minecraft:quarter_negative") {
        return std::make_shared<QuarterNegativeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "minecraft:squeeze") {
        return std::make_shared<SqueezeFunction>(makeDensityFunction(jsondata["argument"], cache));
    } else if (type == "minecraft:add") {
        return std::make_shared<AddFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "minecraft:mul") {
        return std::make_shared<MulFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "minecraft:min") {
        return std::make_shared<MinFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "minecraft:max") {
        return std::make_shared<MaxFunction>(makeDensityFunction(jsondata["argument1"], cache), makeDensityFunction(jsondata["argument2"], cache));
    } else if (type == "minecraft:blend_alpha") {
        return std::make_shared<ConstantFunction>(1.);
    } else if (type == "minecraft:blend_offset") {
        return std::make_shared<ConstantFunction>(0.);
    } else if (type == "minecraft:blend_density") {
        return makeDensityFunction(jsondata["argument"], cache);
    } else if (type == "minecraft:beardifier") {
        return std::make_shared<ErrorFunction>("beardifier should not be referenced in data packs!");
    } else if (type == "minecraft:end_islands") {
        return std::make_shared<ErrorFunction>("No end islands support yet!");
    } else if (type == "minecraft:noise") {
        return std::make_shared<NoiseFunction>(cache.noisesById->at(jsondata["noise"].get<std::string>()), jsondata["xz_scale"].get<double>(), jsondata["y_scale"].get<double>());
    } else if (type == "minecraft:shifted_noise") {
        return makeShiftedNoise(jsondata, cache);
    } else if (type == "minecraft:old_blended_noise") {
        return makeInterpolatedNoise(jsondata, cache);
    } else if (type == "minecraft:weird_scaled_sampler") {
        return std::make_shared<WeirdScaledFunction>(makeDensityFunction(jsondata["input"], cache), cache.noisesById->at(jsondata["noise"].get<std::string>()), jsondata["rarity_value_mapper"].get<std::string>() == "type_1");
    } else if (type == "minecraft:range_choice") {
        return std::make_shared<RangeChoiceFunction>(makeDensityFunction(jsondata["input"], cache), makeDensityFunction(jsondata["when_in_range"], cache), makeDensityFunction(jsondata["when_out_of_range"], cache), jsondata["min_inclusive"].get<double>(), jsondata["max_exclusive"].get<double>());
    } else if (type == "minecraft:shift_a") {
        return std::make_shared<ShiftAFunction>(cache.noisesById->at(jsondata["argument"].get<std::string>()));
    } else if (type == "minecraft:shift_b") {
        return std::make_shared<ShiftBFunction>(cache.noisesById->at(jsondata["argument"].get<std::string>()));
    } else if (type == "minecraft:shift") {
        return std::make_shared<ShiftFunction>(cache.noisesById->at(jsondata["argument"].get<std::string>()));
    } else if (type == "minecraft:clamp") {
        return std::make_shared<ClampFunction>(makeDensityFunction(jsondata["input"], cache), jsondata["min"].get<double>(), jsondata["max"].get<double>());
    } else if (type == "minecraft:constant") {
        return std::make_shared<ConstantFunction>(jsondata["argument"].get<double>());
    } else if (type == "minecraft:y_clamped_gradient") {
        return std::make_shared<YGradientFunction>(jsondata["from_y"].get<i32>(), jsondata["to_y"].get<i32>(), jsondata["from_value"].get<double>(), jsondata["to_value"].get<double>());
    } else if (type == "minecraft:spline") {
        return makeSplineFunction(jsondata, cache);
    } else if (type == "minecraft:interpolated") {
        return std::make_shared<InterpolatedFunction>(makeDensityFunction(jsondata["argument"], cache), cache.horizontalCellBlockCount, cache.verticalCellBlockCount, cache.useCaches);
    } else if (type == "minecraft:flat_cache") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<FlatCacheFunction> ret = std::make_shared<FlatCacheFunction>(makeDensityFunction(jsondata["argument"], cache));
        cache.flatCacheFuncs.push_back(ret);
        return ret;
    } else if (type == "minecraft:cache_2d") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<Cache2DFunction> ret = std::make_shared<Cache2DFunction>(makeDensityFunction(jsondata["argument"], cache));
        cache.cache2DFuncs.push_back(ret);
        return ret;
    } else if (type == "minecraft:cache_all_in_cell") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<CellCacheFunction> ret = std::make_shared<CellCacheFunction>(makeDensityFunction(jsondata["argument"], cache), cache.horizontalCellBlockCount, cache.verticalCellBlockCount);
        cache.cellCacheFuncs.push_back(ret);
        return ret;
    } else if (type == "minecraft:cache_once") {
        if (!cache.useCaches) return makeDensityFunction(jsondata["argument"], cache);
        std::shared_ptr<CacheOnceFunction> ret = std::make_shared<CacheOnceFunction>(makeDensityFunction(jsondata["argument"], cache));
        cache.cacheOnceFuncs.push_back(ret);
        return ret;
    }

    throw std::runtime_error("unrecognised function type " + type);
}

std::shared_ptr<DensityFunction> loadDensityFunction(std::istream& file, DFuncGenInfoCache& infoCache) {
    nlohmann::json jsondata = nlohmann::json::parse(file);
    std::shared_ptr<DensityFunction> function = makeDensityFunction(jsondata, infoCache);
    return function;
}

DoublePerlinNoise loadNoise(std::istream& file) {
    nlohmann::json jsondata = nlohmann::json::parse(file);
    i32 firstOctave = jsondata["firstOctave"].get<i32>();
    std::vector<double> amplitudes = jsondata["amplitudes"].get<std::vector<double>>();
    XoroshiroRandom nullRandom(0);
    return DoublePerlinNoise(nullRandom, amplitudes, firstOctave);
}

static std::filesystem::path getRelPath(const std::filesystem::path& path, const std::string& stop) {
    auto begin = path.end();
    while (*--begin != stop);
    return std::accumulate(++begin, path.end(), std::filesystem::path{}, std::divides{});
}

void loadNoises(const std::filesystem::path& subpackPath, DataPack& pack) {
    std::filesystem::path noisesPath = subpackPath / "worldgen/noise";
    if (!std::filesystem::exists(noisesPath)) return;
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{noisesPath}) {
        if (dir_entry.is_regular_file()) {
            std::string id = getRelPath(dir_entry.path(), "noise").replace_extension();
            std::string namespaced_id = (std::string)(*--subpackPath.end()) + ":" + id;
            std::ifstream file(dir_entry.path());
            *pack.noises[namespaced_id] = loadNoise(file);
        }
    }
}

void loadDensityFunctions(const std::filesystem::path& subpack_path, DataPack& pack) {
    std::filesystem::path dfpath = subpack_path / "worldgen/density_function";
    if (!std::filesystem::exists(dfpath)) return;
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{dfpath}) {
        if (dir_entry.is_regular_file()) {
            std::string id = getRelPath(dir_entry.path(), "density_function").replace_extension();
            std::string namespaced_id = (std::string)(*--subpack_path.end()) + ":" + id;
            std::ifstream file(dir_entry.path());

            DFuncGenInfoCache cache{
                .namedFuncs=&pack.density_functions,
                .noisesById=&pack.noises,
                .useCaches=false,
                .useFlatShiftHack=true,
                .horizontalCellBlockCount=4,
                .verticalCellBlockCount=8
            };
            /// TODO: Poor practice (loadDensityFunction should return a DensityFunction instead of a DFuncPtr)
            *pack.density_functions[namespaced_id] = *loadDensityFunction(file, cache);
            pack.legacy_noises = cache.interpolatedNoises;
        }
    }
}

DataPack loadDataPack(const std::string& rootpath) {
    DataPack pack;

    std::filesystem::path fsroot(rootpath);
    std::filesystem::path dataroot = fsroot.append("data");

    std::vector<std::filesystem::path> subpacks;

    for (const auto& dir_entry : std::filesystem::directory_iterator{dataroot}) {
        if (dir_entry.is_directory())
            subpacks.push_back(dir_entry.path());
    }

    for (const std::filesystem::path& subpack : subpacks) loadNoises(subpack, pack);
    for (const std::filesystem::path& subpack : subpacks) loadDensityFunctions(subpack, pack);

    return pack;
}

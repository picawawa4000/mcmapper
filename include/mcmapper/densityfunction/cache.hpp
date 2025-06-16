#ifndef DENSITYFUNCTION_CACHE_HPP
#define DENSITYFUNCTION_CACHE_HPP

#include <mcmapper/densityfunction/base.hpp>
#include <unordered_map>

/// @brief A function that only computes its input once per
/// horizontal position.
struct Cache2DFunction : public DensityFunction {
    Cache2DFunction(std::shared_ptr<DensityFunction> target);
    virtual double operator()(Pos3D pos);
    void invalidate();

private:
    std::shared_ptr<DensityFunction> target;
    // No map because it's unlikely that this function will
    // be called multiple times for the same column.
    Pos2D lastPos = {0x7FFFFFFF, 0x7FFFFFFF};
    double lastResult = NAN;
};

/// @brief A function that only computes its input once per 4 by 4
/// cell in the X and Z directions.
struct FlatCacheFunction : public DensityFunction {
    FlatCacheFunction(std::shared_ptr<DensityFunction> target);
    virtual double operator()(Pos3D pos);
    void invalidate();

private:
    std::shared_ptr<DensityFunction> target;
    std::unordered_map<Pos2D, std::array<double, 16>> chunkPosToCache;
};

/// @brief A function that caches its input at every position.
struct CacheOnceFunction : public DensityFunction {
    CacheOnceFunction(std::shared_ptr<DensityFunction> target);
    virtual double operator()(Pos3D pos);
    void invalidate();

private:
    std::shared_ptr<DensityFunction> target;
    std::unordered_map<Pos2D, std::array<double, 16*16*384>> chunkPosToCache;
};

/// @brief A function that only computes its input at the lowest coordinate of each cell.
struct CellCacheFunction : public DensityFunction {
    CellCacheFunction(std::shared_ptr<DensityFunction> target, u32 horizontalCellBlockCount, u32 verticalCellBlockCount);
    virtual double operator()(Pos3D pos);
    void invalidate();

private:
    std::shared_ptr<DensityFunction> target;
    std::unordered_map<Pos2D, std::vector<double>> chunkPosToCache;
    u32 horizontalCellBlockCount, verticalCellBlockCount;
    std::size_t chunkCacheSize;
};

/// @brief A function that only samples its input at the corners of
/// each cell, and interpolates within each cell.
/// @note It is recommended to insert a `CellCacheFunction` in front
/// of this function. The `cacheTarget` flag in the constructor will
/// do this automatically.
struct InterpolatedFunction : public DensityFunction {
    /// @brief Creates a new `InterpolatedFunction`.
    /// @param target The density function to interpolate.
    /// @param horizontalCellBlockCount The number of horizontal
    /// blocks in each cell. 4 for the Overworld.
    /// @param verticalCellBlockCount The number of vertical blocks
    /// in each cell. 8 for the Overworld.
    /// @param cacheTarget Whether to insert a `CellCacheFunction` in
    /// front of the target. Unless you are very strained for memory,
    /// it is recommended to leave this as `true`.
    InterpolatedFunction(std::shared_ptr<DensityFunction> target, u32 horizontalCellBlockCount, u32 verticalCellBlockCount, bool cacheTarget = true);
    virtual double operator()(Pos3D pos);

private:
    std::shared_ptr<DensityFunction> target;
    u32 horizontalCellBlockCount, verticalCellBlockCount;
};

#endif
#include <mcmapper/densityfunction/cache.hpp>

Cache2DFunction::Cache2DFunction(std::shared_ptr<DensityFunction> target) : target(target) {}

double Cache2DFunction::operator()(Pos3D pos) {
    Pos2D flatPos = {pos.x, pos.z};
    if (this->lastPos == flatPos) {
        return this->lastResult;
    }
    this->lastPos = flatPos;
    this->lastResult = this->target->operator()(pos);
    return this->lastResult;
}

FlatCacheFunction::FlatCacheFunction(std::shared_ptr<DensityFunction> target) : target(target) {}

double FlatCacheFunction::operator()(Pos3D pos) {
    Pos2D chunkPos{pos.x >> 4, pos.z >> 4};
    std::size_t posHash = (pos.x & 0b1100) | ((pos.z & 0b1100) >> 2);
    if (!this->chunkPosToCache.contains(chunkPos)) {
        this->chunkPosToCache[chunkPos] = std::array<double, 16>{};
        this->chunkPosToCache[chunkPos].fill(NAN);
    }
    double result = this->chunkPosToCache[chunkPos][posHash];
    if (std::isnan(result)) {
        // FlatCacheFunction only samples at y=0
        pos.y = 0;
        result = this->target->operator()(pos);
        this->chunkPosToCache[chunkPos][posHash] = result;
    }
    return result;
}

void FlatCacheFunction::invalidate() {
    this->chunkPosToCache.clear();
}

CacheOnceFunction::CacheOnceFunction(std::shared_ptr<DensityFunction> target) : target(target) {}

double CacheOnceFunction::operator()(Pos3D pos) {
    Pos2D chunkPos{pos.x >> 4, pos.z >> 4};
    std::size_t posHash = (pos.y << 8) | ((pos.x & 0xF) << 4) | (pos.z & 0xF);
    if (!this->chunkPosToCache.contains(chunkPos)) {
        this->chunkPosToCache[chunkPos] = std::array<double, 16*16*384>{};
        this->chunkPosToCache[chunkPos].fill(NAN);
    }
    double result = this->chunkPosToCache[chunkPos][posHash];
    if (std::isnan(result)) {
        result = this->target->operator()(pos);
        this->chunkPosToCache[chunkPos][posHash] = result;
    }
    return result;
}

void CacheOnceFunction::invalidate() {
    this->chunkPosToCache.clear();
}

CellCacheFunction::CellCacheFunction(std::shared_ptr<DensityFunction> target, u32 horizontalCellBlockCount, u32 verticalCellBlockCount) : target(target), horizontalCellBlockCount(horizontalCellBlockCount), verticalCellBlockCount(verticalCellBlockCount) {
    this->chunkCacheSize = std::floor(16 / horizontalCellBlockCount) * std::floor(16 / horizontalCellBlockCount) * std::floor(384 / verticalCellBlockCount);
}

double CellCacheFunction::operator()(Pos3D pos) {
    Pos2D chunkPos{pos.x >> 4, pos.z >> 4};
    std::size_t cellPosHash = std::floor((pos.x & 0xF) / this->horizontalCellBlockCount) * this->horizontalCellBlockCount * this->verticalCellBlockCount + std::floor(pos.y / this->verticalCellBlockCount) * this->horizontalCellBlockCount + std::floor((pos.z & 0xF) / this->horizontalCellBlockCount);
#ifndef NDEBUG
    if (cellPosHash > this->chunkCacheSize - 1)
        throw std::runtime_error("cellPosHash " + std::to_string(cellPosHash) + " of position " + pos.toString() + " doesn't fit within cache of size " + std::to_string(chunkCacheSize) + "!");
#endif
    // Generate cache for chunk, if absent.
    if (!this->chunkPosToCache.contains(chunkPos))
        this->chunkPosToCache[chunkPos] = std::vector<double>(this->chunkCacheSize, NAN);
    double result = this->chunkPosToCache[chunkPos][cellPosHash];
    if (std::isnan(result)) {
        result = this->target->operator()(pos);
        this->chunkPosToCache[chunkPos][cellPosHash] = result;
    }
    return result;
}

void CellCacheFunction::invalidate() {
    this->chunkPosToCache.clear();
}

InterpolatedFunction::InterpolatedFunction(std::shared_ptr<DensityFunction> target, i32 horizontalCellBlockCount, i32 verticalCellBlockCount, bool cacheTarget) : horizontalCellBlockCount(horizontalCellBlockCount), verticalCellBlockCount(verticalCellBlockCount) {
    if (cacheTarget)
        this->target = std::make_shared<CellCacheFunction>(target, horizontalCellBlockCount, verticalCellBlockCount);
    else
        this->target = target;
}

double InterpolatedFunction::operator()(Pos3D pos) {
    i32 lowerX = std::floor(pos.x / this->horizontalCellBlockCount);
    i32 lowerY = std::floor(pos.y / this->verticalCellBlockCount);
    i32 lowerZ = std::floor(pos.z / this->horizontalCellBlockCount);
    double dx = (pos.x - lowerX) / this->horizontalCellBlockCount;
    double dy = (pos.y - lowerY) / this->verticalCellBlockCount;
    double dz = (pos.z - lowerZ) / this->horizontalCellBlockCount;
    double
        v000 = this->target->operator()({lowerX, lowerY, lowerZ}),
        v001 = this->target->operator()({lowerX, lowerY, lowerZ + this->horizontalCellBlockCount}),
        v010 = this->target->operator()({lowerX, lowerY + this->verticalCellBlockCount, lowerZ}),
        v011 = this->target->operator()({lowerX, lowerY + this->verticalCellBlockCount, lowerZ + this->horizontalCellBlockCount}),
        v100 = this->target->operator()({lowerX + this->horizontalCellBlockCount, lowerY, lowerZ}),
        v101 = this->target->operator()({lowerX + this->horizontalCellBlockCount, lowerY, lowerZ + this->horizontalCellBlockCount}),
        v110 = this->target->operator()({lowerX + this->horizontalCellBlockCount, lowerY + this->verticalCellBlockCount, lowerZ}),
        v111 = this->target->operator()({lowerX + this->horizontalCellBlockCount, lowerY + this->verticalCellBlockCount, lowerZ + this->horizontalCellBlockCount});
    return lerp3(dx, dy, dz, v000, v100, v010, v110, v001, v101, v011, v111);
}
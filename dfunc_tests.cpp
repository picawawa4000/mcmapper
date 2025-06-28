#include <mcmapper/datapack/loader.hpp>
#include <hashlib2plus/hl_md5.h>

#include <iostream>

XoroshiroRandom hashRandom(u64 worldLo, u64 worldHi, const std::string& hash) {
    MD5 md5;
    HL_MD5_CTX ctx;
    md5.MD5Init(&ctx);

    std::vector<unsigned char> data(hash.begin(), hash.end());
    md5.MD5Update(&ctx, data.data(), data.size());

    unsigned char digest[16];
    md5.MD5Final(digest, &ctx);

    worldLo ^= *(u64*)(digest);
    worldHi ^= *(u64*)(digest + 8);

    return XoroshiroRandom(worldLo, worldHi);
}

int main() {
    DataPack pack = loadDataPack("/Users/owenk/Downloads/mcmeta-cc1b743e8f26957d1fa3fcf81c80c6665a29b68c");

    std::cout << "Noises" << std::endl;
    for (const auto& kv : pack.noises) std::cout << kv.first << std::endl;
    std::cout << "\nDensity Functions" << std::endl;
    for (const auto& kv : pack.density_functions) std::cout << kv.first << std::endl;

    XoroshiroRandom rng(3447ULL);
    u64 lo = rng.next_u64();
    u64 hi = rng.next_u64();
    for (const auto& kv : pack.noises) {
        XoroshiroRandom noise_rng = hashRandom(lo, hi, kv.first);
        kv.second->replace(noise_rng);
    }
    for (const auto& legacy_noise : pack.legacy_noises) {
        CheckedRandom noise_rng(3447ULL);
        legacy_noise->replace(noise_rng);
    }

    DFuncPtr final_density_function = pack.density_functions["minecraft:overworld/final_density"];
    for (int i = 320; i > -64; --i) {
        if (final_density_function->operator()({0, i, 0}) > 0.) {
            std::cout << "Ground at " << i << std::endl;
            break;
        }
    }

    return 0;
}
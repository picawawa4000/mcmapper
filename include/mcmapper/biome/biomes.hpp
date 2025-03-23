#ifndef BIOMES_HPP
#define BIOMES_HPP

#include <mcmapper/util.hpp>
#include <string>

enum Biome {
    //if the void is 0, checking for no biomes becomes quicker
    //since you can just write `if (!biome) {std::cout << "no biome found!"}`
    THE_VOID = 0,
    PLAINS,
    SUNFLOWER_PLAINS,
    SNOWY_PLAINS,
    ICE_SPIKES,
    DESERT,
    SWAMP,
    MANGROVE_SWAMP,
    FOREST,
    FLOWER_FOREST,
    BIRCH_FOREST,
    DARK_FOREST,
    PALE_GARDEN,
    OLD_GROWTH_BIRCH_FOREST,
    OLD_GROWTH_PINE_TAIGA,
    OLD_GROWTH_SPRUCE_TAIGA,
    TAIGA,
    SNOWY_TAIGA,
    SAVANNA,
    SAVANNA_PLATEAU,
    WINDSWEPT_HILLS,
    WINDSWEPT_GRAVELLY_HILLS,
    WINDSWEPT_FOREST,
    WINDSWEPT_SAVANNA,
    JUNGLE,
    SPARSE_JUNGLE,
    BAMBOO_JUNGLE,
    BADLANDS,
    ERODED_BADLANDS,
    WOODED_BADLANDS,
    MEADOW,
    CHERRY_GROVE,
    GROVE,
    SNOWY_SLOPES,
    FROZEN_PEAKS,
    JAGGED_PEAKS,
    STONY_PEAKS,
    RIVER,
    FROZEN_RIVER,
    BEACH,
    SNOWY_BEACH,
    STONY_SHORE,
    WARM_OCEAN,
    LUKEWARM_OCEAN,
    DEEP_LUKEWARM_OCEAN,
    OCEAN,
    DEEP_OCEAN,
    COLD_OCEAN,
    DEEP_COLD_OCEAN,
    FROZEN_OCEAN,
    DEEP_FROZEN_OCEAN,
    MUSHROOM_FIELDS,
    DRIPSTONE_CAVES,
    LUSH_CAVES,
    DEEP_DARK,
    NETHER_WASTES,
    WARPED_FOREST,
    CRIMSON_FOREST,
    SOUL_SAND_VALLEY,
    BASALT_DELTAS,
    THE_END,
    END_HIGHLANDS,
    END_MIDLANDS,
    SMALL_END_ISLANDS,
    END_BARRENS
};

//returns upper-camel-cased representation of biome name
//at some point, this should probably be converted to a map
//to allow it to be data-driven
std::string biomeRepr(Biome biome);

namespace biometags {
    const Tag<Biome> DEEP_OCEAN = Tag<Biome>({Biome::DEEP_FROZEN_OCEAN, Biome::DEEP_COLD_OCEAN, Biome::DEEP_OCEAN, Biome::DEEP_LUKEWARM_OCEAN});
    const Tag<Biome> OCEAN = Tag<Biome>({Biome::FROZEN_OCEAN, Biome::COLD_OCEAN, Biome::OCEAN, Biome::LUKEWARM_OCEAN, Biome::WARM_OCEAN}).add(DEEP_OCEAN);
    const Tag<Biome> BEACH = Tag<Biome>({Biome::BEACH, Biome::SNOWY_BEACH});
    const Tag<Biome> RIVER = Tag<Biome>({Biome::RIVER, Biome::FROZEN_RIVER});
    const Tag<Biome> MOUNTAIN = Tag<Biome>({Biome::MEADOW, Biome::FROZEN_PEAKS, Biome::JAGGED_PEAKS, Biome::STONY_PEAKS, Biome::SNOWY_SLOPES, Biome::CHERRY_GROVE});
    const Tag<Biome> HILL = Tag<Biome>({Biome::WINDSWEPT_HILLS, Biome::WINDSWEPT_FOREST, Biome::WINDSWEPT_GRAVELLY_HILLS});
    const Tag<Biome> TAIGA = Tag<Biome>({Biome::TAIGA, Biome::SNOWY_TAIGA, Biome::OLD_GROWTH_PINE_TAIGA, Biome::OLD_GROWTH_SPRUCE_TAIGA});
    const Tag<Biome> JUNGLE = Tag<Biome>({Biome::JUNGLE, Biome::BAMBOO_JUNGLE, Biome::SPARSE_JUNGLE});
    const Tag<Biome> FOREST = Tag<Biome>({Biome::FOREST, Biome::FLOWER_FOREST, Biome::BIRCH_FOREST, Biome::OLD_GROWTH_BIRCH_FOREST, Biome::DARK_FOREST, Biome::GROVE});
    const Tag<Biome> BADLANDS = Tag<Biome>({Biome::BADLANDS, Biome::WOODED_BADLANDS, Biome::ERODED_BADLANDS});

    const Tag<Biome> OVERWORLD = Tag<Biome>({Biome::PLAINS, Biome::SUNFLOWER_PLAINS, Biome::SNOWY_PLAINS, Biome::ICE_SPIKES, Biome::DESERT, Biome::SWAMP, Biome::MANGROVE_SWAMP, Biome::PALE_GARDEN, Biome::SAVANNA, Biome::SAVANNA_PLATEAU, Biome::WINDSWEPT_SAVANNA, Biome::STONY_SHORE, Biome::MUSHROOM_FIELDS, Biome::DRIPSTONE_CAVES, Biome::LUSH_CAVES, Biome::DEEP_DARK}).add(OCEAN).add(BEACH).add(RIVER).add(MOUNTAIN).add(HILL).add(TAIGA).add(JUNGLE).add(FOREST).add(BADLANDS);
    const Tag<Biome> NETHER = Tag<Biome>({Biome::NETHER_WASTES, Biome::WARPED_FOREST, Biome::CRIMSON_FOREST, Biome::SOUL_SAND_VALLEY, Biome::BASALT_DELTAS});
    const Tag<Biome> END = Tag<Biome>({Biome::THE_END, Biome::END_HIGHLANDS, Biome::END_MIDLANDS, Biome::SMALL_END_ISLANDS, Biome::END_BARRENS});

    const Tag<Biome> BURIED_TREASURE = Tag<Biome>().add(BEACH);
    const Tag<Biome> DESERT_PYRAMID = Tag<Biome>({Biome::DESERT});
    const Tag<Biome> IGLOO = Tag<Biome>({Biome::SNOWY_TAIGA, Biome::SNOWY_PLAINS, Biome::SNOWY_SLOPES});
    const Tag<Biome> JUNGLE_TEMPLE = Tag<Biome>({Biome::BAMBOO_JUNGLE, Biome::JUNGLE});
    const Tag<Biome> MINESHAFT = Tag<Biome>({Biome::STONY_SHORE, Biome::MUSHROOM_FIELDS, Biome::ICE_SPIKES, Biome::WINDSWEPT_SAVANNA, Biome::DESERT, Biome::SAVANNA, Biome::SNOWY_PLAINS, Biome::PLAINS, Biome::SUNFLOWER_PLAINS, Biome::SWAMP, Biome::MANGROVE_SWAMP, Biome::SAVANNA_PLATEAU, Biome::DRIPSTONE_CAVES, Biome::LUSH_CAVES}).add(OCEAN).add(RIVER).add(BEACH).add(MOUNTAIN).add(HILL).add(TAIGA).add(FOREST);
    const Tag<Biome> MINESHAFT_MESA = Tag<Biome>().add(BADLANDS);
    const Tag<Biome> MINESHAFT_BLOCKING = Tag<Biome>({Biome::DEEP_DARK});
    const Tag<Biome> OCEAN_MONUMENT = Tag<Biome>().add(DEEP_OCEAN);
    const Tag<Biome> SURROUND_OCEAN_MONUMENT = Tag<Biome>().add(OCEAN).add(RIVER);
    const Tag<Biome> COLD_OCEAN_RUINS = Tag<Biome>({Biome::FROZEN_OCEAN, Biome::DEEP_FROZEN_OCEAN, Biome::COLD_OCEAN, Biome::DEEP_COLD_OCEAN, Biome::OCEAN, Biome::DEEP_OCEAN});
    const Tag<Biome> WARM_OCEAN_RUINS = Tag<Biome>({Biome::LUKEWARM_OCEAN, Biome::DEEP_LUKEWARM_OCEAN, Biome::WARM_OCEAN});
    const Tag<Biome> PILLAGER_OUTPOST = Tag<Biome>({Biome::DESERT, Biome::PLAINS, Biome::SAVANNA, Biome::SNOWY_PLAINS, Biome::TAIGA, Biome::GROVE}).add(MOUNTAIN);
    const Tag<Biome> RUINED_PORTAL_DESERT = Tag<Biome>({Biome::DESERT});
    const Tag<Biome> RUINED_PORTAL_JUNGLE = Tag<Biome>().add(JUNGLE);
    const Tag<Biome> RUINED_PORTAL_OCEAN = Tag<Biome>().add(OCEAN);
    const Tag<Biome> RUINED_PORTAL_SWAMP = Tag<Biome>({Biome::SWAMP, Biome::MANGROVE_SWAMP});
    const Tag<Biome> RUINED_PORTAL_MOUNTAIN = Tag<Biome>({Biome::WINDSWEPT_SAVANNA, Biome::SAVANNA_PLATEAU, Biome::STONY_SHORE}).add(MOUNTAIN).add(BADLANDS).add(HILL);
    const Tag<Biome> RUINED_PORTAL_STANDARD = Tag<Biome>({Biome::MUSHROOM_FIELDS, Biome::DRIPSTONE_CAVES, Biome::LUSH_CAVES, Biome::ICE_SPIKES, Biome::SAVANNA, Biome::SNOWY_PLAINS, Biome::PLAINS, Biome::SUNFLOWER_PLAINS}).add(BEACH).add(RIVER).add(TAIGA).add(FOREST);
    const Tag<Biome> SHIPWRECK_BEACHED = Tag<Biome>().add(BEACH);
    const Tag<Biome> SHIPWRECK = Tag<Biome>().add(OCEAN);
    const Tag<Biome> SWAMP_HUT = Tag<Biome>({Biome::SWAMP});
    const Tag<Biome> VILLAGE_PLAINS = Tag<Biome>({Biome::PLAINS, Biome::MEADOW});
    const Tag<Biome> VILLAGE_DESERT = Tag<Biome>({Biome::DESERT});
    const Tag<Biome> VILLAGE_SNOWY = Tag<Biome>({Biome::SNOWY_PLAINS});
    const Tag<Biome> VILLAGE_SAVANNA = Tag<Biome>({Biome::SAVANNA});
    const Tag<Biome> VILLAGE_TAIGA = Tag<Biome>({Biome::TAIGA});
    const Tag<Biome> TRAIL_RUINS = Tag<Biome>({Biome::OLD_GROWTH_BIRCH_FOREST, Biome::JUNGLE}).add(TAIGA);
    const Tag<Biome> WOODLAND_MANSION = Tag<Biome>({Biome::DARK_FOREST});
    const Tag<Biome> TRIAL_CHAMBERS = Tag<Biome>().add(OVERWORLD).remove(Biome::DEEP_DARK);
    const Tag<Biome> ANCIENT_CITY = Tag<Biome>({Biome::DEEP_DARK});

    const Tag<Biome> NETHER_STRUCTURE = Tag<Biome>().add(NETHER);
    const Tag<Biome> NETHER_FOSSIL = Tag<Biome>({Biome::SOUL_SAND_VALLEY});
    const Tag<Biome> BASTION_REMNANT = Tag<Biome>({Biome::NETHER_WASTES, Biome::CRIMSON_FOREST, Biome::WARPED_FOREST, Biome::SOUL_SAND_VALLEY});

    const Tag<Biome> END_CITY = Tag<Biome>({Biome::END_HIGHLANDS, Biome::END_MIDLANDS});

    const Tag<Biome> STRONGHOLD_BIASED_TO = Tag<Biome>({Biome::PLAINS, Biome::SUNFLOWER_PLAINS, Biome::SNOWY_PLAINS, Biome::ICE_SPIKES, Biome::DESERT, Biome::SAVANNA, Biome::SAVANNA_PLATEAU, Biome::WINDSWEPT_SAVANNA, Biome::MUSHROOM_FIELDS, Biome::DRIPSTONE_CAVES, Biome::LUSH_CAVES}).add(TAIGA).add(JUNGLE).add(BADLANDS).add(FOREST).add(HILL).add(MOUNTAIN).remove(Biome::CHERRY_GROVE);
    const Tag<Biome> STRONGHOLD = Tag<Biome>().add(OVERWORLD);
}

#endif
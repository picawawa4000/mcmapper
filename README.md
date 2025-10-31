# ARCHIVE NOTICE

After some deliberation, I decided that hardcoding the world generation would not be worth it. Instead, I created a new library that can read world generation settings (among other things) from datapacks and act on them. This new library can be found at:

https://github.com/picawawa4000/dpreader

# MCMapper

A C++ repository containing several Minecraft seed regeneration utilities.

## State of the program (to-do list)

MCMapper is currently in an **experimental** state. APIs are subject to constant change, most notably if VSCode ever implements C++20 modules.

Headers with a fairly stable API include:

- `mcmapper/densityfunction/*.hpp`
- `mcmapper/biome/*.hpp`
- `mcmapper/data/*.hpp`

- `mcmapper/util.hpp`  
- `mcmapper/rng/rng.hpp`  
- `mcmapper/rng/noise.hpp`  
- `mcmapper/terrain/spline.hpp`  
- `mcmapper/terrain/internoise.hpp`  
- `mcmapper/densityfunction/base.hpp`

Headers that are currently lacking features, or that may be split or otherwise refactored, include:

- `mcmapper/rng/noises.hpp`  
- `mcmapper/loot/loot.hpp`  
- `mcmapper/loot/builtin_loot.hpp`  
- `mcmapper/misc/generator.hpp`  
- `mcmapper/structure/structure.hpp`  
- `mcmapper/structure/structures.hpp`  
- `mcmapper/structure/gen/mansion.hpp`  
- `mcmapper/terrain/terrain.hpp`  
- `mcmapper/terrain/terrainextra.hpp`  

Headers that are completely unfinished include:

- `mcmapper/mcmapper.hpp`  
- `mcmapper/structure/gen/spiece.hpp`  
- `mcmapper/datapack/loader.hpp`

See [TODO.md](./TODO.md) for a more precise list of places where the library needs fixing.

## Dependencies

Can be found in `thirdparty`.

- [JSON](https://github.com/nlohmann/json)
- [hashlib++](https://hashlib2plus.sourceforge.net)

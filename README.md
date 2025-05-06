# MCMapper

A C++ repository containing several Minecraft seed regeneration utilities.

## State of the program (to-do list)

MCMapper is currently in an **experimental** state. APIs are subject to constant change, most notably if VSCode ever implements C++20 modules.

Headers with a fairly stable API include:

`mcmapper/util.hpp`  
`mcmapper/rng/rng.hpp`  
`mcmapper/rng/noise.hpp`  
`mcmapper/biome/*.hpp`  
`mcmapper/terrain/spline.hpp`  
`mcmapper/terrain/internoise.hpp`  
`mcmapper/data/*.hpp`

Headers that are currently lacking features, or that may be split or otherwise refactored, include:

`mcmapper/rng/noises.hpp`  
`mcmapper/loot/loot.hpp`  
`mcmapper/loot/builtin_loot.hpp`  
`mcmapper/misc/generator.hpp`  
`mcmapper/structure/structure.hpp`  
`mcmapper/structure/structures.hpp`  
`mcmapper/structure/gen/mansion.hpp`  
`mcmapper/terrain/terrain.hpp`  
`mcmapper/terrain/terrainextra.hpp`  

Headers that are completely unfinished include:

`mcmapper/mcmapper.hpp`  
`mcmapper/structure/gen/spiece.hpp`  

See [TODO.md](./TODO.md) for a more precise list of places where the library needs fixing.

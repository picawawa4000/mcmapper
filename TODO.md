# TODO

This is mostly just a list of bugs that need to be squashed or features that need to be added.

## Bugs

- [ ] `DoublePerlinNoise` is incorrect (beyond two decimal places), but `OctavePerlinNoise` and `PerlinNosie` are correct.
  - Known issues that may be related:
    - [ ] End density is incorrect.
    - [ ] Overworld initial density suffers from potential scaling issues.
    - [ ] Overworld final density is incorrect in some places.
- [ ] Inputs (block coordinates) for all terrain functions need to be converted to longs.

## Features

- [ ] The whole biome and terrain generation system is ugly and should be reworked.
- [ ] Ore veins (see [vein.cpp](vein.cpp)).
- [ ] Structure pieces are currently half-baked and non-functional.
- [ ] A lot more structures need to be implemented.
- [ ] Loot tables are missing several important loot functions, most notably enchantWithLevels.
- [ ] Biome enums and trees for older versions (down to 1.18) should be generated (low priority).

Note to self: Woodland mansions did not generate the `1x1_b5` room until 25w17a.

# Wayfarer

Wayfarer is a new open-source space game.

**Pioneer** is the technical host: 3D procedural galaxy, systems, planets,
terrain, ships, flight, and rendering.

**Endless Sky** is a design reference for data-driven ships, outfits, cargo,
markets, missions, and factions — concepts are reimplemented, not merged wholesale.

The Stage 0 goal is a living simulation layer: economy, autonomous NPC traders,
contracts, and faction reputation that exist whether or not the player trades.

## Repository layout

```
docs/                 Fusion architecture + licensing/provenance
simulation/           Headless wayfarer::sim library, tests, Stage 0 demo
data/wayfarer/        Data-driven ships, outfits, commodities, factions, worlds
host/pioneer/         Pioneer bridge (C++/Lua) + integrate.sh
vendor/               Local Pioneer / Endless Sky checkouts (not required to commit)
```

## Quick start (headless Stage 0)

```bash
cmake -S simulation -B simulation/build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++
cmake --build simulation/build -j$(nproc)
cd simulation/build && ctest --output-on-failure
./wayfarer_stage0_demo
```

This proves markets, NPC trading with persistent economic consequences, contracts,
reputation, and determinism — without opening a renderer.

## Pioneer visual host

1. Clone Pioneer into `vendor/pioneer` (or pass another path).
2. Run the integrator:

```bash
./host/pioneer/integrate.sh /path/to/pioneer
```

3. Build Pioneer as usual (`./bootstrap` / CMake + Ninja), pointing
   `WAYFARER_SIM_DIR` at this repo’s `simulation/` directory (set automatically
   by `integrate.sh` when run from this tree).

4. Start a game (`pioneer -startat`). Stage 0 starts the simulation, shows the
   economy debug panel, and spawns an NPC freighter steered by Pioneer AI.

Player flight controls remain Pioneer’s. The UI displays simulation state; it
does not own it.

## Documentation

- [Fusion architecture](docs/fusion-architecture.md)
- [Licensing & provenance](docs/licensing.md)

## Licence

GPL-3.0-or-later. See `LICENSE` and `docs/licensing.md`.

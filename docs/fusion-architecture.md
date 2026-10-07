# Wayfarer Fusion Architecture (Stage 0)

Wayfarer is a new open-source game. **Pioneer** is the technical/runtime host
(3D procedural space, flight, rendering). **Endless Sky** is a donor/reference
for data-driven ships, outfits, cargo, economy, missions, and factions.

We do **not** merge the two codebases wholesale. We keep Pioneer’s physical
universe and recreate Endless Sky–style gameplay concepts as a render-free
**World Simulation** layer.

---

## Layer diagram

```
┌───────────────────────────────┐
│          Presentation          │
│  Renderer / Scenegraph / UI    │
│  (Pioneer Graphics, PiGui)     │
└───────────────┬───────────────┘
                │ consumes state
                ▼
┌───────────────────────────────┐
│        Gameplay Layer          │
│  Missions / Contracts / Player │
└───────────────┬───────────────┘
                │
                ▼
┌───────────────────────────────┐
│        World Simulation        │
│  Economy / Factions / Ships    │
│  Logistics / Markets / Jobs    │
│  (wayfarer::sim — headless)    │
└───────────────┬───────────────┘
                │ PioneerWorldAdapter
                ▼
┌───────────────────────────────┐
│          Pioneer World         │
│  Galaxy / Systems / Planets    │
│  Physics / Flight / Terrain    │
└───────────────────────────────┘
```

**Critical rule:** the simulation must not depend on rendering. A headless
tick must be possible:

```cpp
wayfarer::sim::WorldSimulation simulation;
simulation.Tick();
auto ships = simulation.GetShips();
auto markets = simulation.GetMarkets();
auto factions = simulation.GetFactions();
```

---

## Pioneer systems we keep

| System | Location (Pioneer) | Role in Wayfarer |
|--------|--------------------|------------------|
| 3D renderer | `src/graphics/` | Presentation only |
| Scenegraph / models | `src/scenegraph/` | Ships, stations, props |
| Galaxy generation | `src/galaxy/` | Physical star map |
| Star systems / SystemBody | `StarSystem`, `SystemBody` | Locations for economic metadata |
| Planets / terrain | `Planet`, `TerrainBody`, `terrain/` | Landing and atmosphere |
| Ships / flight | `Ship`, `Propulsion`, controllers | Player & NPC 3D motion |
| Physics / collision | `Space`, `collider/`, `Frame` | Host world tick |
| Camera / WorldView | `Camera`, `WorldView` | Presentation |
| Input | `Input`, `PlayerShipController` | Unchanged flight controls |
| Sound | OpenAL path | Unchanged |
| UI infrastructure | PiGui / `data/pigui/` | Host for economy debug panel |
| Save/load shell | `SaveGameManager`, JSON | Extended with Wayfarer sim state |
| Ship JSON defs | `data/ships/*.json` | Still used for 3D models/thrust |
| Existing Lua modules | `data/modules/`, `data/libs/` | Coexist; Stage 0 does not delete them |

---

## Pioneer systems we extend

| System | Extension |
|--------|-----------|
| Ship data | Parallel Wayfarer hull/outfit model (`data/wayfarer/ships/`) mapped onto Pioneer ships via adapter |
| System / station simulation | `EconomicProfile` attached to Pioneer locations |
| NPC ships | Autonomous traders driven by sim AI, rendered as Pioneer `Ship` bodies |
| Economic objects | Markets with supply/demand (not static price tables) |
| Factions | Continuous reputation graph (Endless Sky–inspired) beside Pioneer `Faction`/`Polit` |
| Missions | Sim-side `Contract` objects; UI only displays/accepts |
| Cargo | Sim `CargoHold` with capacity checks; sync to Pioneer cargo where needed |
| Simulation ticking | Explicit `SimulationClock` separate from render frame rate |
| Game session | `Game::TimeStep` (or bridge) advances WorldSimulation after/with `Space::TimeStep` |

---

## Pioneer systems we eventually replace (only with clear reason)

| System | Reason to replace later | Stage 0 action |
|--------|-------------------------|----------------|
| Lua commodity market pricing in isolation | Prices should emerge from living supply/demand + NPC logistics | Overlay Wayfarer markets; do not delete Pioneer economy yet |
| Ad-hoc Lua mission modules as the only mission source | Missions should eventually emerge from world conditions | Add Contract layer; leave existing modules intact |
| Player-centric “economy starts when you arrive” feel | World must run without the player | NPC traders tick regardless of player trade |

**Do not** rewrite galaxy generation, flight physics, renderer, or scenegraph
because Endless Sky does them differently.

---

## Endless Sky concepts worth recreating

| Concept | Pioneer equivalent | Endless Sky equivalent | New Wayfarer implementation | Reason |
|---------|--------------------|------------------------|-----------------------------|--------|
| Data-driven ship defs | `data/ships/*.json` + `ShipType` | `ship` blocks in `.txt` | `data/wayfarer/ships/*.json` hull + outfits | Moddable gameplay stats without hard-coding C++ |
| Hull / outfit separation | Equipment slots + Lua equip | Chassis attributes + `Outfit` bag | `ShipHull` + `Outfit` composition | Clear capacity/power budgets |
| Outfit statistics | Equipment Lua modules | Free-form attribute dictionary | Minimal outfits: engine, shield, weapon power | Extensible; Stage 0 subset only |
| Cargo capacity | Ship `cargo` field / CargoManager | `CargoHold` tons | `CargoHold` with hard capacity | Enforce trade realism |
| Commodities | `GalacticEconomy` JSON | `Trade::Commodity` | `Commodity` registry + roles | Shared language for markets/contracts |
| Market prices | Station Lua markets | Base price + supply erf curve | Supply/demand derived buy/sell | Living economy |
| Missions / contracts | Lua `Mission` modules | `Mission` / `job` templates | `Contract` in simulation | UI displays; sim owns truth |
| Faction relationships | `Faction` + `Polit` types | `Government` + `Politics` reputation | `Faction` + player reputation map | Gate access/missions later |
| Reputation | Limited / law focus | Continuous floats + attitudes | Per-faction float | Contract rewards adjust standing |
| Ship roles | JSON `roles` | Categories / fleets | Role tag on hull (`trader`, …) | Spawn appropriate NPCs |
| Equipment | EquipSet | Outfits | Outfits affecting hull stats | Composition |
| Supply / demand | Affinity / production graph | Per-system `supply` + daily step | Per-location market stocks | NPC + player both mutate state |

We **reimplement concepts**, we do not copy Endless Sky class hierarchies or
large source files.

---

## Compatibility boundary

```
Pioneer Body / SystemPath / SpaceStation
        ↓
PioneerWorldAdapter
        ↓
wayfarer::sim::WorldLocation  (body id + EconomicProfile)
wayfarer::sim::SimShip        (hull, outfits, cargo, faction, location)
wayfarer::sim::WorldSimulation
        ↓
Gameplay (contracts, player accept/deliver)
```

Example shapes:

```cpp
struct WorldLocation {
    LocationId id;
    std::string name;
    std::string pioneer_path; // SystemPath string form when bound
    EconomicProfile economy;
    Market market;
};

struct SimShip {
    ShipId id;
    ShipHull hull;
    std::vector<Outfit> outfits;
    CargoHold cargo;
    FactionId faction;
    LocationId location;
    std::optional<LocationId> destination;
    Credits credits;
};
```

Economic classes must not include Pioneer renderer or `SceneGraph::Model`
headers.

---

## Data layer

Prefer Pioneer’s existing approach: **human-readable JSON**, deterministic,
diff-friendly, moddable. No new scripting language in Stage 0.

```
data/wayfarer/
├── ships/
├── outfits/
├── commodities/
├── factions/
├── governments/
├── industries/
├── markets/          # optional seeds / profiles
├── missions/         # contract templates (later)
└── worlds/           # economic profiles bound to locations
```

Root Wayfarer repo also mirrors this under `/data/wayfarer` for the headless
library; Pioneer integration copies or points at the same files.

---

## Simulation clock

```cpp
struct SimulationClock {
    double current_time = 0;   // simulation seconds
    double tick_duration = 1;  // seconds advanced per Tick()
    bool paused = false;
    double time_scale = 1.0;   // 1 real second → N sim seconds (architecture only)
};
```

Stage 0 advances at a fixed sim step from the host (Pioneer `Game::TimeStep`
bridge or headless demo). Extreme acceleration (1000×) is supported by the
clock API but not required for acceptance.

---

## Stage 0 vertical slice

1. Load/generate a Pioneer-capable star system context (or headless stand-in locations bound to Pioneer paths).
2. Two economically distinct locations (Mining / Industrial) with `EconomicProfile`.
3. Player ship with Wayfarer hull + cargo.
4. Autonomous NPC trader: select opportunity → buy → travel → sell → mutate markets.
5. Generated trade contract; player accept → cargo → deliver → credits + reputation.
6. Debug economy panel showing markets, NPC traders, contracts.
7. Deterministic tests for market, cargo, trade, contract, determinism.

---

## Engineering principles (Stage 0)

- Prefer composition (`Ship` → Hull, CargoHold, Outfits, Faction, AI).
- Prefer data over hard-coded behaviour.
- Prefer deterministic simulation (seeded RNG).
- Keep rendering separate from simulation.
- Reuse Pioneer for physics/flight/galaxy.
- Reimplement Endless Sky ideas; do not glue two games together.
- Keep the tree buildable after every meaningful change.
- Avoid premature abstraction and massive rewrites.

---

## Repository layout (Wayfarer)

```
docs/                 Architecture & licensing
simulation/           Headless wayfarer::sim library + tests
data/wayfarer/        Data-driven content
host/pioneer/         Optional Pioneer bridge / PiGui hooks
vendor/pioneer/       Pioneer host (development checkout / submodule)
vendor/endless-sky/   Reference only — not linked into the binary
```

---

## Out of scope for Stage 0

- Importing Endless Sky source tree into the build
- Replacing Pioneer renderer or flight model
- Rewriting galaxy generation
- Full Endless Sky campaign / ship / outfit catalogue
- Multiplayer, procedural factions, complex AI
- New UI framework
- Copying Endless Sky artwork

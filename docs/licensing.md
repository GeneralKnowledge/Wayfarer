# Wayfarer Licensing & Provenance

This document keeps Wayfarer legally auditable. Source-code licences do **not**
automatically cover artwork, music, fonts, or other assets.

---

## Summary

| Component | Licence | Redistributable with Wayfarer? |
|-----------|---------|--------------------------------|
| Wayfarer original code (`simulation/`, `host/`, most of `docs/`, `data/wayfarer/`) | **GPL-3.0-or-later** (same as host) | Yes |
| Pioneer (host engine, data, assets) | **GPL-3.0** (+ third-party notices under `vendor/pioneer/licenses/`) | Yes, under GPL-3 terms and Pioneer attribution |
| Endless Sky **source code** (reference only) | **GPL-3.0-or-later** | Not shipped in Wayfarer binaries; concepts reimplemented |
| Endless Sky **data text** (`data/*.txt`) | **GPL-3.0-or-later** | Do **not** copy wholesale; Stage 0 uses original Wayfarer JSON |
| Endless Sky **images** | Mostly **CC-BY-SA-4.0** (land/scene often public domain; see ES `copyright`) | **Not imported** in Stage 0 |
| Endless Sky **sounds** | Mostly public domain; some CC-BY-SA | **Not imported** in Stage 0 |

Wayfarer Stage 0 prefers **reimplementation of mechanics** over copying large
amounts of Endless Sky source or assets.

---

## Pioneer

- **Upstream:** https://github.com/pioneerspacesim/pioneer  
- **Licence:** GNU General Public License v3 (`vendor/pioneer/licenses/GPL-3.txt`)  
- **Copyright:** Pioneer Developers (`vendor/pioneer/AUTHORS.txt`)  
- **Third-party:** See `vendor/pioneer/licenses/` (Lua, GLEW, imgui, lz4, Assimp usage, fonts, NASA imagery policy, etc.)

When distributing a Wayfarer build that includes Pioneer, you must comply with
GPL-3 (source offer, licence texts, attribution).

---

## Endless Sky

- **Upstream:** https://github.com/endless-sky/endless-sky  
- **Code / default data text:** GPL-3.0-or-later (`vendor/endless-sky/license.txt`, Debian-format `copyright`)  
- **Images:** CC-BY-SA-4.0 by default; landscapes/scenes often public domain — **read `copyright` before any asset reuse**  
- **Role in Wayfarer:** Design reference and inspection only for Stage 0  

### What we take from Endless Sky

| Kind | Status |
|------|--------|
| Design concepts (hull/outfit split, supply/demand trade, contracts/jobs, reputation) | Independently reimplemented in `simulation/` |
| C++ classes / large source files | **Not copied** |
| Ship/outfit/mission data files | **Not copied** in Stage 0 |
| Artwork / music / fonts | **Not copied** in Stage 0 |

If future stages import ES data or CC-BY-SA art, update this file and add
per-file provenance under `docs/provenance/`.

---

## Wayfarer original work

All of the following are original Wayfarer work unless a file header says
otherwise:

- `docs/fusion-architecture.md`, `docs/licensing.md`
- `simulation/**` (WorldSimulation, markets, ships, outfits, factions, contracts, clock, adapters)
- `data/wayfarer/**` (Stage 0 JSON content)
- `host/pioneer/**` (bridge / debug UI hooks)
- Headless demo and unit tests

Licence: **GPL-3.0-or-later**, to remain compatible with Pioneer as host.

---

## Adapted vs independently reimplemented

| Item | Classification |
|------|----------------|
| Pioneer build/run as host | Upstream Pioneer (unchanged licence) |
| `PioneerWorldAdapter` aggregating SystemPath-like location ids | Original; inspired by Pioneer’s location model |
| Supply/demand price curve | Independently reimplemented; *inspired by* Endless Sky’s dynamic supply pricing, **not** a line-by-line port of `System::Price` / `GameData::StepEconomy` |
| Hull + outfit composition | Independently reimplemented; *inspired by* Endless Sky ship/outfit separation |
| Contract / job board fields | Independently reimplemented; *inspired by* Endless Sky mission/job templates |
| Faction reputation floats | Independently reimplemented; *inspired by* Endless Sky `Politics` / `Government` |
| Debug economy panel layout | Original Wayfarer development UI |

---

## Attribution requirements

When distributing Wayfarer:

1. Include GPL-3 licence text.  
2. Credit Pioneer and retain Pioneer licence/AUTHORS/third-party notices.  
3. If Endless Sky code or CC-BY-SA assets are ever added, credit Endless Sky
   contributors and satisfy GPL and/or CC-BY-SA (share-alike, attribution).  
4. Do not claim endorsement by either upstream project.

---

## Distribution checklist

- [ ] `COPYING` / `LICENSE` present (GPL-3)  
- [ ] Pioneer notices preserved when shipping the host  
- [ ] No Endless Sky artwork in Stage 0 tree  
- [ ] No unverified third-party assets  
- [ ] `docs/licensing.md` updated when imports change  
- [ ] File headers on original C++ sources  

---

## Stage 0 import inventory

| Path | Provenance |
|------|------------|
| `vendor/pioneer/**` | Pioneer upstream (dev checkout / submodule) — not authored by Wayfarer |
| `vendor/endless-sky/**` | Endless Sky upstream — **reference only**, not linked |
| `simulation/**` | Original Wayfarer |
| `data/wayfarer/**` | Original Wayfarer |
| `docs/**` | Original Wayfarer |
| `host/**` | Original Wayfarer |

Commodity names such as Food, Ore, Metal, Fuel, Machinery, Electronics, Luxury
Goods are generic terms, not copied from Endless Sky data files.

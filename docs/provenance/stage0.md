# Stage 0 provenance notes

| Path | Origin |
|------|--------|
| `simulation/**` | Original Wayfarer |
| `data/wayfarer/**` | Original Wayfarer (generic commodity names) |
| `host/pioneer/**` | Original Wayfarer Pioneer bridge |
| `docs/fusion-architecture.md` | Original Wayfarer (after inspecting Pioneer + Endless Sky) |
| `docs/licensing.md` | Original Wayfarer |
| `simulation/third_party/nlohmann/json.hpp` | nlohmann/json v3.11.3 (MIT) |
| Pioneer upstream | GPLv3 — used as host; not copied into this commit tree by default |
| Endless Sky upstream | Reference only — no source or assets imported into Stage 0 binaries |

Endless Sky design concepts independently reimplemented in Stage 0:

- hull / outfit separation
- supply–demand influenced prices
- contract/job fields
- faction reputation floats

#!/usr/bin/env bash
# Copy Wayfarer host integration into a Pioneer source tree and patch CMake/Lua/Game.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PIONEER="${1:-$ROOT/vendor/pioneer}"
HOST="$ROOT/host/pioneer"
SIM="$ROOT/simulation"

if [[ ! -d "$PIONEER/src" ]]; then
  echo "Pioneer tree not found at $PIONEER" >&2
  exit 1
fi

mkdir -p "$PIONEER/src/wayfarer" "$PIONEER/data/modules/Wayfarer" "$PIONEER/data/pigui/modules"

cp -f "$HOST/src/wayfarer/WayfarerHost.h" "$PIONEER/src/wayfarer/"
cp -f "$HOST/src/wayfarer/WayfarerHost.cpp" "$PIONEER/src/wayfarer/"
cp -f "$HOST/src/lua/LuaWayfarer.h" "$PIONEER/src/lua/"
cp -f "$HOST/src/lua/LuaWayfarer.cpp" "$PIONEER/src/lua/"
cp -f "$HOST/data/modules/Wayfarer/Stage0.lua" "$PIONEER/data/modules/Wayfarer/"
cp -f "$HOST/data/pigui/modules/wayfarer-economy-panel.lua" "$PIONEER/data/pigui/modules/"

python3 - "$PIONEER" "$SIM" <<'PY'
import sys
from pathlib import Path

pioneer = Path(sys.argv[1])
sim = Path(sys.argv[2]).resolve()
cmake = pioneer / "CMakeLists.txt"
text = cmake.read_text()

if "src/wayfarer" not in text:
    text = text.replace(
        "list(APPEND SRC_FOLDERS\n\tsrc/",
        "list(APPEND SRC_FOLDERS\n\tsrc/\n\tsrc/wayfarer",
        1,
    )

block = f'''
# --- Wayfarer Stage 0 simulation (headless economy / traders / contracts) ---
set(WAYFARER_SIM_DIR "{sim}" CACHE PATH "Wayfarer simulation library")
if (EXISTS "${{WAYFARER_SIM_DIR}}/CMakeLists.txt")
\tadd_subdirectory(${{WAYFARER_SIM_DIR}} ${{CMAKE_BINARY_DIR}}/wayfarer_sim_ext EXCLUDE_FROM_ALL)
\ttarget_link_libraries(pioneer-lib PUBLIC wayfarer_sim)
\tmessage(STATUS "Wayfarer simulation enabled: ${{WAYFARER_SIM_DIR}}")
else()
\tmessage(WARNING "Wayfarer simulation not found at ${{WAYFARER_SIM_DIR}}")
endif()
'''

if "WAYFARER_SIM_DIR" not in text:
    anchor = "define_pioneer_library(pioneer-lib PIONEER_CXX_FILES PIONEER_HXX_FILES)\ntarget_link_libraries(pioneer-lib PUBLIC pioneer-core)"
    if anchor not in text:
        raise SystemExit("CMake anchor not found")
    text = text.replace(anchor, anchor + "\n" + block, 1)

cmake.write_text(text)
print("Patched", cmake)

lua_cpp = pioneer / "src/lua/Lua.cpp"
lt = lua_cpp.read_text()
if "LuaWayfarer" not in lt:
    if '#include "LuaEconomy.h"' in lt:
        lt = lt.replace('#include "LuaEconomy.h"', '#include "LuaEconomy.h"\n#include "LuaWayfarer.h"', 1)
    else:
        lt = lt.replace('#include "Lua.h"', '#include "Lua.h"\n#include "LuaWayfarer.h"', 1)
    if "LuaEconomy::Register();" in lt:
        lt = lt.replace("LuaEconomy::Register();", "LuaEconomy::Register();\n\t\tLuaWayfarer::Register();", 1)
    else:
        lt = lt.replace("LuaGame::Register();", "LuaWayfarer::Register();\n\t\tLuaGame::Register();", 1)
    lua_cpp.write_text(lt)
    print("Patched", lua_cpp)

game_cpp = pioneer / "src/Game.cpp"
gt = game_cpp.read_text()
if "WayfarerHost" not in gt:
    if '#include "WorldView.h"' in gt:
        gt = gt.replace('#include "WorldView.h"', '#include "WorldView.h"\n#include "wayfarer/WayfarerHost.h"', 1)
    else:
        gt = gt.replace('#include "Game.h"', '#include "Game.h"\n#include "wayfarer/WayfarerHost.h"', 1)
    old = "\tm_space->TimeStep(step);\n\n\tSfxManager::TimeStepAll(step, m_space->GetRootFrame());"
    new = "\tm_space->TimeStep(step);\n\n\tif (wayfarer::WayfarerHost::Get().IsActive()) {\n\t\twayfarer::WayfarerHost::Get().OnGameTimeStep(step);\n\t}\n\n\tSfxManager::TimeStepAll(step, m_space->GetRootFrame());"
    if old not in gt:
        raise SystemExit("Game.cpp TimeStep pattern not found")
    gt = gt.replace(old, new, 1)
    game_cpp.write_text(gt)
    print("Patched", game_cpp)
PY

# Optional soft-GL / llvmpipe workarounds (cities skip + skip broken DDS uploads)
SOFT_GL_PATCH="$HOST/patches/soft-gl-llvmpipe.patch"
if [[ -f "$SOFT_GL_PATCH" ]]; then
  if ! grep -q 'WAYFARER_SKIP_CITIES' "$PIONEER/src/CityOnPlanet.cpp" 2>/dev/null; then
    (cd "$PIONEER" && patch -p1 --forward < "$SOFT_GL_PATCH") || echo "Note: soft-GL patch may already be applied"
  fi
fi

echo "Integration complete into $PIONEER"
echo "For software GL (llvmpipe) CI/cloud environments, run Pioneer with:"
echo "  WAYFARER_SKIP_CITIES=1 WAYFARER_SOFT_GL=1 DisableSound=1 ./pioneer -startat"

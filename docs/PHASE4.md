# Phase 4 — content pipeline (note, not started)

One-file placeholder so the plan is visible without a full phase. Sized in `docs/PLAN.md`.

| Item | What | Blocker |
|---|---|---|
| `content/` | git-tracked sources: `maps/*.vmf`, `models/*.qc` + SMD/DMX, `materials/*.psd|png` + `.vtf` sources | none |
| `content/README.md` | exact `vbsp` / `vvis` / `vrad` / `studiomdl` / `vtex` commands (Windows or Proton on the SDK Base `bin/` tools), outputs into `game/cascade/{maps,models,materials}` | C3: no Linux compilers |
| `tools/pack_vpk.sh` | `cascade_misc.vpk` + `cascade_textures.vpk` from `game/cascade/` via `<SDK Base MP>/bin/linux64/vpk` (needs `LD_LIBRARY_PATH=<SDK>/bin/linux64` for `libmimalloc.so`) | none |
| First map `hx_c1_01` | `cfg/chapter1.cfg` already points at it; needs `info_player_start`, one `hx_logic_checkpoint`, one `hx_logic_run` wired to a `trigger_once`; `game/cascade/cascade.fgd` defines both | needs a Windows/Proton Hammer session |
| Menu background | `maps/hx_background01.bsp` + `scripts/ChapterBackgrounds.txt` update; until then the static `materials/console/background01` shows | same |
| `docs/ASSET_INDEPENDENCE.md` | already has entry 1; add a row per new `content/` asset that replaces an SDK Base mount | — |

# Cascade — repo guide for Claude Code

Cascade is a singleplayer game built on the **HL2 branch** of Valve's `source-sdk-2013` (2025 64-bit tree).
It runs on **Source SDK Base 2013 Multiplayer (app 243750)**. `docs/ARCHITECTURE.md` is the source of truth for
why; `docs/PLAN.md` for what is next.

## Hard constraints

| # | Rule |
|---|---|
| C1 | `src/buildallprojects` (release **and** debug) must pass after every commit |
| C2 | Never commit build outputs: `src/_vpc_/`, `src/compile_commands.json`, `game/*/bin/`, `game/*_linux64`, `*.so`, `*.o`, `*.a` built by ninja (see `.gitignore`) |
| C3 | Do not run map/model/material compilers on Linux (vbsp/vvis/vrad/studiomdl/vtex are Windows-only). Document commands in `content/README.md` |
| C4 | Everything under `src/` stays under the SOURCE 1 SDK LICENSE. Owner tooling outside `src/` (`tools/`) may be AGPL-3.0. No SPDX headers in `src/` |
| C5 | Never delete upstream systems in place. Disable via `CHxGameRules` / `hx_` ConVars first; remove only after build + smoke test |
| C6 | Never guess an engine API. `grep` the tree and cite `file:line` in the commit message |
| C7 | Ask before any rename touching >50 files or any `git rm` |
| C8 | New ConVars, entities, classes and files are prefixed `hx_` / `CHx` / `hx_` |

## Commands

```bash
tools/dev.sh build [debug|release]     # podman + Steam Runtime sniper; regenerates VPC if scripts changed
tools/dev.sh run --testmaps +map test_hardware   # sniper runtime, windowed, -dev -console -novid -condebug
tools/dev.sh run --hl2maps +map d1_trainstation_01   # dev-only mount of Half-Life 2 (app 220) content
tools/dev.sh log                       # tail game/cascade/console.log
tools/dev.sh attach-gdb                # attach to hl2_linux64 (use a debug build)
```

Build targets: `/hl2` (Cascade), `/hl2mp`, `/tf` — all three must keep building (`src/buildallprojects:27`).
Outputs: `game/cascade/bin/linux64/{client,server}.so`, `game/cascade_linux64`.

## Layout

| Path | What |
|---|---|
| `src/game/{client,server,shared}/hl2/` | HL2 game code Cascade compiles. Upstream files — minimal diffs only |
| `src/game/{client,server,shared}/hx/` | Cascade-owned code (Phase 3+) |
| `src/game/client/client_hl2.vpc`, `src/game/server/server_hl2.vpc` | Project files. `$GAMENAME "cascade"` under `$SOURCESDK` |
| `src/launcher_main/launcher_main_cascade.vpc` | Launcher; binary name `cascade_linux64` → mod dir `game/cascade` |
| `game/cascade/` | Mod dir: `gameinfo.txt`, `cfg/`, `resource/`, `scripts/`, `maps/` |
| `game/mod_tf/`, `game/mod_hl2mp/` | Upstream templates. Do not edit |
| `tools/` | Owner tooling (AGPL-3.0) |
| `docs/` | `ARCHITECTURE.md`, `PLAN.md`, later `GAMEPLAY.md`, `RELEASE.md`, `ASSET_INDEPENDENCE.md` |

## Known engine issues (not fixable here)

- `cl_localnetworkbackdoor` must stay `0` (`game/cascade/cfg/hx_defaults.cfg`) — upstream #610.
- Save/restore is not 64-bit clean; vphysics restore crashes — upstream #629. Design around checkpoints.

## Working method

- One phase = one branch (`phase-N/<name>`) = one PR against `origin` (haptixxx-dev fork). `upstream` = ValveSoftware.
- Small commits, each buildable. Commit messages cite `file:line` for engine facts.
- Phase ends with a status table (done · deferred · assumptions · how verified). Do not wait for "go" between phases.
- Subsystem verdicts: `docs/ARCHITECTURE.md` §5.

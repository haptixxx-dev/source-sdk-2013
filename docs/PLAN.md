# Cascade — Plan

Brief (2026-09-09):

```
Working title:      Cascade
One-line pitch:     TBD (singleplayer, HL2-branch campaign)
Mode(s) at launch:  singleplayer only
Player count:       1
Art direction:      HL2 assets from SDK Base 2013 MP (hl2 + hl2_complete) for now; TF2 not mounted
Distribution goal:  Steam (free mod under SOURCE 1 SDK LICENSE, Valve approval) long-term; zip meanwhile
Non-goals v1:       multiplayer, econ/items, matchmaking, replay, VR, quicksave parity with HL2 (checkpoints first)
```

Constraints C1–C8 from the bootstrap prompt apply. `docs/ARCHITECTURE.md` holds the decision record.

One phase = one branch = one PR. Every commit builds with `src/buildallprojects` release **and** debug.

---

## Phase overview

| Phase | Branch | Goal | Est. commits | Files touched | Risk | Blocker |
|---|---|---|---|---|---|---|
| 0 | `phase-0/audit` | Audit, decision record, this plan | 2 | `docs/` | low | — |
| 1 | `phase-1/identity` | HL2 SP target builds as Cascade; repo tooling | 6–8 | ~25 | **med** — first-ever `/hl2` linux64 link | none |
| 2 | `phase-2/branding` | Player-facing identity: menu, schemes, strings, icons | 5–7 | ~20 | low | Phase 1 boots to menu |
| 3 | `phase-3/gameplay` | `CHxGameRules`, one mode, `hx_` ConVars, checkpoint spike | 8–12 | ~15 new | **med** — save/restore 64-bit | Phase 1 |
| 4 | `phase-4/content` | `content/`, `tools/pack_vpk.sh`, asset index | 3–4 | ~10 | low | vpk tool in SDK Base MP |
| 5 | `phase-5/release` | CI, versioning, packaging, `docs/RELEASE.md` | 4–6 | ~10 | low | Phase 1 build in CI container |
| 6 | `phase-6/hardening` | srcds, crash dumps, backlog | 3–5 | ~8 | low | v0.1 boots end-to-end |

---

## Phase 1 — Repo identity (detailed)

Deviation from the bootstrap prompt: **no rename of `game/mod_tf`**. Cascade is an HL2-branch mod; `game/cascade/` is
created new from the `game/mod_hl2mp/` template (46 tracked files, 29 of them `scripts/` weapon/game scripts). `mod_tf`
and `mod_hl2mp` stay untouched so upstream merges stay clean. No C7 trigger (>50-file rename) expected.

| # | Task | Files | Risk | Commits |
|---|---|---|---|---|
| 1.1 | Wire HL2 target into VPC: add `[$HL2]` lines for `client_hl2.vpc`, `server_hl2.vpc`, new `launcher_main_cascade.vpc` | `src/vpc_scripts/projects.vgc:16-20,51-55,57-61`, `src/launcher_main/launcher_main_cascade.vpc` (+`.rc`) | med | 1 |
| 1.2 | `$GAMENAME "cascade"` under `$SOURCESDK` so outputs land in `game/cascade/bin/linux64/` | `src/game/client/client_hl2.vpc:8-9`, `src/game/server/server_hl2.vpc:8-9` | low | (with 1.1) |
| 1.3 | Add `/hl2` to VPC invocation; keep `/hl2mp /tf` so upstream targets still build (C1 baseline) | `src/buildallprojects:27` | low | 1 |
| 1.4 | First `/hl2` linux64 build. Fix 64-bit / sniper compile errors in `server/hl2`, `client/hl2` if any (upstream #1145 reports it compiles) | unknown | **med** | 1–3 |
| 1.5 | `game/cascade/`: `gameinfo.txt` (`type singleplayer_only`, `SteamAppId 243750`, `nodegraph 1`, `GameData halflife2.fgd`), `cfg/`, `resource/`, `scripts/` copied from `mod_hl2mp` template | `game/cascade/**` (~45 files) | low | 1 |
| 1.6 | Force `cl_localnetworkbackdoor 0`; `-novid -dev -console` dev cfg | `game/cascade/cfg/config_default.cfg`, `cfg/valve.rc` | low | (with 1.5) |
| 1.7 | Boot smoke test: `./cascade_linux64 -dev -console -windowed +map test_hardware`. SDK Base MP ships only `hl2/maps/{background01,test_hardware}.bsp`; HL2 campaign maps need app 220 (`\|appid_220\|hl2`, dev-only mount behind `hx_dev_mount_hl2` gameinfo variant) | `console.log` | med | 0 |
| 1.8 | Remotes: `origin` → Haptixxx fork, `upstream` → ValveSoftware | git config | low | 0 |
| 1.9 | Root `CLAUDE.md` + 4 scoped stubs, `.gitignore` additions (`src/.ninja_*`, `compile_commands.json`, `src/lib/public/linux64/*.a`, `game/*_linux64`, `game/*/bin/`, runtime junk: `stats.txt`, `voice_ban.dt`, `videoconfig_linux.cfg`, `GameState.txt`, `trainingprogress.txt`, `cfg/config.cfg`), `.clangd` | root | low | 1 |
| 1.10 | `tools/dev.sh` (`build`, `run`, `attach-gdb`, `log`), `README.md` | `tools/`, `README.md` | low | 1 |

Exit criteria: release + debug build green for `tf`, `hl2mp`, `hl2`; `./game/cascade_linux64` reaches the main menu;
`console.log` shows `[Source Mod Launcher] Launching default game: cascade`.

---

## Phase 2 — Player-facing identity

| # | Task | Files |
|---|---|---|
| 2.1 | `resource/GameMenu.res` (engine GameUI main menu): New Game, Load, Options, Quit only | `game/cascade/resource/GameMenu.res` |
| 2.2 | `resource/cascade_english.txt`; `gameinfo.txt` `title`/`title2`; `steam.inf` `ProductName=cascade` | `game/cascade/resource/`, `steam.inf` |
| 2.3 | `ClientScheme.res`, `SourceScheme.res` from `docs/BRAND.md` placeholder | `game/cascade/resource/` |
| 2.4 | Background map / `materials/console/background01*.vtf` (VTF authored on Windows/Proton, C3), `resource/game.ico`, `icon.*` | `game/cascade/` |
| 2.5 | Chapter/loading screen: `cfg/chapter1.cfg`, `scripts/titles.txt` | `game/cascade/` |

Exit: no "Half-Life" string reachable from the main menu.

---

## Phase 3 — Gameplay foundation

| # | Task | Files |
|---|---|---|
| 3.1 | `CHxGameRules : CHalfLife2` in `src/game/shared/hx/hx_gamerules.{h,cpp}`; `InstallGameRules()` creates it under `HX_DLL` define | `src/game/server/hl2/hl2_client.cpp:162`, new `shared/hx/` |
| 3.2 | `shared/hx/hx_convars.cpp`: `hx_checkpoint_*`, `hx_weapon_roster`, `hx_disable_gamestats` | new |
| 3.3 | Checkpoint spike: `hx_logic_checkpoint` entity → transition-style restart (`changelevel`/`restart` + persisted player state via `hx_` KeyValues), **not** engine `save` | new `server/hx/` |
| 3.4 | Save/restore 64-bit audit: run `save`/`load` on a physics-heavy test map in debug build; list crashing datadescs; decide fix-vs-avoid | `docs/GAMEPLAY.md` |
| 3.5 | Weapon/NPC roster enforced in `CHxGameRules` (`FShouldSwitchWeapon`, `CanHavePlayerItem`, precache lists) | `hx_gamerules.cpp` |
| 3.6 | HUD element `CHxHudCheckpoint` | `client/hx/` |
| 3.7 | `docs/GAMEPLAY.md` with round/chapter state diagram | docs |

---

## Phase 4 — Content pipeline

| # | Task |
|---|---|
| 4.1 | `content/{maps,models,materials}/` sources + `content/README.md` with vbsp/vvis/vrad/studiomdl/vtex commands (Windows or Proton, C3) |
| 4.2 | `tools/pack_vpk.sh` using `<SDK Base MP>/bin/linux64/vpk` inside the sniper container |
| 4.3 | `docs/ASSET_INDEPENDENCE.md`: entry 1 = list of `|appid_243750|` mounts (hl2, hl2_complete, platform) |

## Phase 5 — Release engineering

| # | Task |
|---|---|
| 5.1 | `.github/workflows/build.yml`: sniper container, release + debug, warning baseline diff, upload `client.so`/`server.so`/`cascade_linux64` (stripped) |
| 5.2 | `tools/version.sh` → `steam.inf` `PatchVersion/ClientVersion/ServerVersion`, git tag |
| 5.3 | `tools/package.sh` → `dist/cascade-<ver>-linux64.tar.zst` + Windows layout dir |
| 5.4 | `docs/RELEASE.md`: player install (SDK Base 2013 MP required), `sourcemods/` path, Steam free-mod approval path |

## Phase 6 — Hardening

| # | Task |
|---|---|
| 6.1 | Dedicated server not applicable for SP; document `-listen` only. Drop srcds from scope |
| 6.2 | Breakpad/`core` dump location, `tools/dev.sh attach-gdb` against debug build |
| 6.3 | Backlog: Windows build, Steamworks (achievements, cloud saves), episodic content, TF2 asset mounting (if art direction changes) |

---

## Backlog / open questions

| Item | Owner | Needed by |
|---|---|---|
| One-line pitch, target length, core loop | Sarah | Phase 3 |
| Episodic (`HL2_EPISODIC`) in or out | Sarah | Phase 1.1 (affects vpc choice) |
| `docs/BRAND.md` (fonts, palette, logo) | Sarah / placeholder | Phase 2 |
| Dev-only mount of Half-Life 2 (app 220) for campaign test maps — yes/no | Sarah | Phase 1.7 |
| Save/restore fix-vs-avoid | Phase 3.4 result | Phase 3 |

---

## Build baseline (2026-09-09, `b8cfb12c`, sniper container, 16 cores + ccache)

| Config | Targets | Result | Warnings (pre-existing, upstream) |
|---|---|---|---|
| release | tf, hl2mp, launchers, tools | pass | not captured (built before audit) |
| debug | same, 3196 ninja steps | pass, `EXIT=0` | 2: `game/server/hl2/npc_monk.cpp:274` `-Wsequence-point`; `game/server/hl2/npc_strider.cpp:432` `-Wdelete-non-virtual-dtor` |

C1 rule "no new warnings" is measured against this list. Both warnings live in HL2 code that Cascade will compile, so they
will appear in the `/hl2` build too — leave them (upstream code, C5) unless they turn into runtime bugs.

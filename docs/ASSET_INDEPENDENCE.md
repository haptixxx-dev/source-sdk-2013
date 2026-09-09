# Asset independence roadmap

Cascade currently plays entirely with Valve content mounted from the player's **Source SDK Base 2013
Multiplayer** install (app 243750). Nothing from Half-Life 2 (app 220) or Team Fortress 2 (app 440) is
mounted. This file tracks every external mount in `game/cascade/gameinfo.txt` and what replaces it.

## Entry 1 — 2026-09-09, Phase 2 state

| Search path (`gameinfo.txt`) | Provides | Cascade still needs it for | Replacement plan |
|---|---|---|---|
| `\|appid_243750\|hl2/hl2_textures.vpk`, `hl2_misc.vpk` | HL2 world/model materials, models, scripts (`scripts/*.txt`, `resource/hl2_english.txt`, `resource/*.res`) | everything: player, weapons, NPC models, UI schemes we did not override | Long-term. Replace per-asset as `content/` grows; keep a `hx_asset_manifest.txt` of what a shipped map references |
| `\|appid_243750\|hl2/hl2_sound_misc.vpk`, `hl2_sound_vo_english.vpk` | weapon/world sounds, HL2 NPC voice lines | weapons, NPCs | Own SFX + VO. NPC voice lines only matter if HL2 NPCs stay in the roster |
| `\|appid_243750\|hl2_complete/*.vpk` | extra HL2 content packaged for HL2DM mods (models/materials not in `hl2/`) | unknown until a map audit | Audit with `mat_texture_list` / `sv_pure` style listing per map; drop if unused |
| `\|appid_243750\|platform/platform_misc.vpk`, `platform` | VGUI base schemes, fonts, `gameui_english.txt`, engine dialogs | all menus/dialogs | Never replaced — engine-owned, ships with every Source game |
| ~~`\|appid_243750\|sourcetest`~~ | `maps/background01.bsp` | Tried as main-menu background map in Phase 2; it is a Lost Coast map with missing materials. **Removed** — the menu uses the static `materials/console/background01` instead | `content/maps/hx_background01` when art exists |

Already independent: `resource/GameMenu.res`, `resource/ClientScheme.res`, `resource/cascade_english.txt`,
`materials/console/background01*`, `materials/vgui/chapters/chapter1*`, `resource/game.ico`, `cfg/*`,
`scripts/HudLayout.res`, `scripts/titles.txt`, `scripts/hud_textures.txt`.

## Rules

- Every new line under `SearchPaths` gets a row here in the same commit.
- Dev-only content (Half-Life 2 app 220, `sourcetest` once dropped) is mounted through `tools/dev.sh run --hl2maps/--testmaps`, never in `gameinfo.txt`.
- Redistributing Valve `.bsp`/`.vpk` content inside this repo is not allowed; mounts are the only way to use it.

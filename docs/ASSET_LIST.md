# Cascade — asset replacement brief

What Valve content Cascade uses today and what an artist needs to deliver to replace it. Regenerate the
raw lists with `tools/list_assets.py` (outputs in `docs/assets/`); this file is the curated version.

Scope today: **no Cascade map exists yet**, so "used" means what the compiled HL2 code, the weapon
scripts and the UI reference by name. World textures, props and skyboxes are level-design driven and are
listed as categories with counts, not files. Everything below is mounted from Source SDK Base 2013 MP
(`docs/ASSET_INDEPENDENCE.md`).

## Delivery formats (engine constraints)

| Type | Source to deliver | Shipped as | Notes |
|---|---|---|---|
| Models | FBX or SMD/DMX + textures, rigged where animated; QC written by us | `.mdl` via `studiomdl` (Windows/Proton) | HL2 skeleton/animation compatibility matters for weapons (`v_*` use the HL2 hands rig) and NPCs (HL2 activity set) |
| Textures / UI | PNG or TGA, power-of-two (128…2048), alpha where needed | `.vtf` via `vtex`; UI placeholders can go through `tools/gen_brand_assets.py` | sRGB; sprite sheets keep the existing sub-rect layout so `hud_textures.txt` still works |
| Fonts | TTF (title + UI) | referenced by `resource/ClientScheme.res` | Replaces the HL2 symbol font that draws weapon/crosshair glyphs; see P0 |
| Audio | WAV 44.1 kHz 16-bit mono (SFX), stereo for music | `.wav`/`.mp3`, soundscripts in `scripts/game_sounds_*.txt` | Names must match the soundscript entries or the scripts get re-pointed |

Palette and typography: `docs/BRAND.md`.

## P0 — always on screen (commission first)

### UI and HUD

| Asset | Valve file(s) today | Used by | Deliverable |
|---|---|---|---|
| Main-menu / console background | `materials/console/background01[_widescreen]` (Cascade placeholder already) | engine GameUI | 1024×1024 (4:3) + 1024×1024 with 16:9 content, left third quiet |
| Chapter tiles | `materials/vgui/chapters/chapter1` (placeholder) | New Game dialog | 152×86 per chapter, padded to 256×128 |
| Game icon | `resource/game.ico`, `src/launcher_main/res/cascade.ico` (placeholder) | window/launcher | 256 px master |
| HUD sprite sheet | `sprites/640_hud` | health/armor/ammo panels, damage indicators | one 256×256 sheet, keep sub-rect layout in `scripts/hud_textures.txt` |
| Crosshairs | `sprites/crosshairs`, `sprites/hud/v_crosshair1`, `v_crosshair2`, `sprites/qi_center` | crosshair, quick-info | 64–128 px sprites |
| Weapon selection & ammo icons | glyphs in the `HalfLife2` symbol font (`resource/halflife2.ttf`) via `WeaponIcons*` fonts in `ClientScheme.res` | weapon wheel, ammo HUD | either a new icon font (TTF, one glyph per weapon + ammo type) or a sprite sheet + `ClientScheme.res` rework |
| HUD/UI fonts | `Trebuchet MS`, `HalfLife2`, `HL2EP2.ttf` | all HUD text, chapter titles | 1 UI family (regular/bold), 1 display family for titles |
| Flashlight, suit-power, zoom, squad, geiger overlays | `sprites/640_hud` sub-rects, `vgui/hud/*` (17 files) | `hud_*.cpp` in `src/game/client/hl2/` | same sheet, plus `materials/vgui/hud/` replacements |
| Loading / dialog chrome | `materials/vgui/resource/*` (31), `vgui/cursors/*` (12) | engine dialogs | optional; engine-owned look, low value |

### First person

| Asset | Valve file(s) today | Deliverable |
|---|---|---|
| Hands | `models/weapons/v_hands.mdl` | HL2 hands rig, new mesh/textures — every viewmodel below shares it |
| Weapons in the default roster (`hx_weapon_roster` empty = all) | see table | view model (`v_`) + world model (`w_`) each, HL2 animation set |

| Weapon | View model | World model | Ammo pickup models (`models/items/`) |
|---|---|---|---|
| `weapon_crowbar` | `v_crowbar.mdl` | `w_crowbar.mdl` | — |
| `weapon_pistol` | `v_pistol.mdl` | `w_pistol.mdl` | `boxsrounds.mdl` |
| `weapon_357` | `v_357.mdl` | `w_357.mdl` | `357ammo.mdl`, `357ammobox.mdl` |
| `weapon_smg1` | `v_smg1.mdl` | `w_smg1.mdl` | `boxmrounds.mdl`, `ammocrate_smg1.mdl`, grenade `ar2_grenade.mdl` |
| `weapon_ar2` | `v_irifle.mdl` | `w_irifle.mdl` | `combine_rifle_ammo01.mdl`, `combine_rifle_cartridge01.mdl` |
| `weapon_shotgun` | `v_shotgun.mdl` | `w_shotgun.mdl` | `boxbuckshot.mdl` |
| `weapon_crossbow` | `v_crossbow.mdl` | `w_crossbow.mdl` | `crossbowrounds.mdl`, bolt `models/crossbow_bolt.mdl` |
| `weapon_rpg` | `v_rpg.mdl` | `w_rocket_launcher.mdl` | `models/weapons/w_missile*.mdl` (3) |
| `weapon_frag` | `v_grenade.mdl` | `models/items/grenadeammo.mdl` | — |
| `weapon_physcannon` | `v_physcannon.mdl` (+ `v_superphyscannon.mdl`) | `w_physics.mdl` | — |
| `weapon_bugbait` | `v_bugbait.mdl` | `w_bugbait.mdl` | — (drop if antlions are cut) |
| `weapon_stunstick` | `v_stunbaton.mdl` | `w_stunbaton.mdl` | NPC-only unless rostered |

Full script data: `docs/assets/weapons.csv`. Pickups: `models/items/healthkit.mdl`, `healthvial.mdl`,
`battery.mdl`, `hevsuit.mdl`, ammo crates `ammocrate_*.mdl` (7), `item_item_crate.mdl`.

## P1 — enemies and allies (commission per roster decision)

The code compiles 55 NPC types; `hx_npc_roster` decides which ship. Models the code hard-references
(`docs/assets/code_references.txt`):

| Group | Models | Extra parts |
|---|---|---|
| Combine infantry | `combine_soldier.mdl`, `combine_super_soldier.mdl`, `police.mdl`, `police_cheaple.mdl` | gibs in `models/gibs/` (35 files referenced), `w_stunbaton`, shields |
| Combine synths / machines | `combine_strider.mdl`, `gunship.mdl`, `attack_helicopter.mdl`, `combine_helicopter*.mdl`, `combine_dropship*.mdl`, `combine_scanner.mdl`, `shield_scanner.mdl`, `manhack.mdl`, `roller.mdl`, `combine_turrets/{floor,ceiling,ground,citizen}_turret.mdl`, `combine_camera/combine_camera.mdl`, `combine_apc_destroyed_gib0[1-6].mdl` | projectiles: `models/weapons/w_missile*`, `helicopter_bomb01.mdl`, `w_energy_grenade.mdl` |
| Zombies / headcrabs | `zombie/{classic,fast,poison,zombie_soldier}.mdl` (+ `_torso`, `_legs`), `headcrab.mdl`, `headcrabclassic.mdl`, `headcrabblack.mdl`, `baby_headcrab.mdl`, `barnacle.mdl` | |
| Antlions | `antlion.mdl`, `antlion_worker.mdl`, `antlion_guard.mdl`, `antlion_grub*.mdl`, `grub_nugget_*.mdl`, `spitball_*.mdl` | only if bugbait stays |
| Wildlife | `crow.mdl`, `pigeon.mdl`, `ichthyosaur.mdl`, `leech.mdl` | |
| Allies / story cast | `alyx.mdl`, `barney.mdl`, `eli.mdl`, `kleiner.mdl`, `mossman.mdl`, `breen.mdl`, `gman.mdl`, `monk.mdl`, `dog.mdl`, `vortigaunt.mdl`, `stalker.mdl`, `humans/male_cheaple.mdl` + citizen set (`npc_citizen17` picks from `models/humans/group0*`) | cut list in Phase 3 roster |
| Player | `models/player.mdl` (ragdoll/third person only) | |

Cheapest viable enemy set for a v0.1 vertical slice: 1 humanoid (soldier), 1 melee (zombie), 1 fast
(headcrab or manhack), 1 turret. Everything else can stay on `hx_npc_roster` = off.

## P2 — effects and world (level-design driven)

| Category | Valve files today | Notes |
|---|---|---|
| Muzzle flashes, tracers, beams, glows | `effects/*` (39) and `sprites/*` (40) referenced by weapon and NPC code — `effects/combinemuzzle*`, `effects/muzzleflash1`, `effects/laser1`, `sprites/glow01`, `sprites/bluelaser1`, `sprites/redglow*`… | Small VTFs; batch of ~80 |
| Particles | `particles/*.pcf` via `particles_manifest.txt` (blood, fire, smoke, water) | Editable in-engine with the particle editor (Windows) |
| Decals | `decals_subrect.txt` + `materials/decals/*` | Bullet holes, blood, scorch |
| World textures | `hl2_textures.vpk`: `materials/models` (1377 files), plus `materials/{concrete,metal,brick,wood,nature,building_template,…}` | Per-map material list once maps exist; start from the level art bible, not from HL2 |
| Props | `models/props_*` — `props_wasteland` 1657, `props_c17` 1433, `props_debris` 1397, `props_combine` 1009, `props_junk` 750, `props_lab` 709, `props_pipes` 570, `gibs` 682 | Replace what the maps place; audit per map with `mat_texture_list` |
| Skyboxes | `materials/skybox/sky_*` | 1 per map |

## P3 — audio

| Category | Files today (`hl2_sound_misc.vpk`) | Notes |
|---|---|---|
| Weapons | 144 | one set per rostered weapon: fire, reload, empty, draw |
| Player | 91 | footsteps (per material), damage, suit voice (HEV lines) |
| Physics / impacts | 408 | material impact set |
| Ambient | 607 | map-driven |
| Doors / plats / buttons / items | 47 / 35 / 32 / 12 | prop-driven |
| Music | 56 | menu + stingers |
| NPC SFX | 1283 | per rostered NPC |
| Voice-over (English) | 2553 (`hl2_sound_vo_english.vpk`) | only needed for HL2 NPC dialogue; a Cascade cast replaces it wholesale |

Soundscripts to re-point: `scripts/game_sounds_{weapons,player,physics,items,world,ui,vehicles,ambient_generic}.txt`.

## Order of work

1. P0 UI + fonts (menu already Cascade-branded; HUD sheet + icon font make it fully ours).
2. Hands + the 5-weapon starter set (`crowbar, pistol, smg1, shotgun, ar2`) + their pickups.
3. P1 vertical-slice enemy set (4 types) with gibs.
4. Effects batch (muzzle/tracer/glow) — small but everywhere.
5. Audio in the same order: weapons, player, impacts.
6. World/props/skybox per map as level design lands (Phase 4).

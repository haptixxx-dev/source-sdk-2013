# game/cascade — mod directory

- `gameinfo.txt`: `SteamAppId 243750`, `type singleplayer_only`, mounts `|appid_243750|hl2` + `hl2_complete` only. No TF2 (440), no Half-Life 2 (220) — dev mounts go through `tools/dev.sh run --hl2maps`.
- `cfg/valve.rc` execs `cfg/hx_defaults.cfg` (engine workarounds) before `autoexec.cfg`. Keep player-editable settings out of `hx_defaults.cfg`.
- `bin/` is a build output (ignored). `console.log`, `save/`, `screenshots/` are runtime junk (ignored).
- Binary assets (`.bsp`, `.vtf`, `.mdl`) are compiled on Windows/Proton from `content/` (C3). Only commit outputs that are actually shipped.
- Localization tokens live in `resource/cascade_english.txt` (UTF-16 LE). Strings still say Half-Life until Phase 2.

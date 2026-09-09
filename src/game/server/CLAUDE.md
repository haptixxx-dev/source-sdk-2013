# src/game/server — Cascade notes

- Cascade compiles `server_hl2.vpc` (`HL2_DLL;USES_SAVERESTORE`). No `NEXT_BOT`, no `nav_mesh.vpc`; HL2 AI uses `.ain` node graphs.
- Gamerules are created in `hl2/hl2_client.cpp` `InstallGameRules()`; Cascade overrides via `CHxGameRules` (Phase 3) under an `HX_DLL` define.
- Player limits come from `base_gameinterface.cpp` (`minplayers = 1`). Do not copy the hl2mp/tf gameinterface.
- Cascade-owned server code goes in `hx/`. Entities are `hx_*`, classes `CHx*`.
- Save/restore: every new entity needs a `BEGIN_DATADESC` block sized for 64-bit (`public/datamap.h`). Upstream #629 tracks the remaining breakage.

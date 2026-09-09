# src/game/shared — Cascade notes

- Shared HL2 code: `hl2/` (gamerules, movement, usermessages). `tf/`, `econ/`, `hl2mp/` are not compiled for Cascade.
- `hx/` holds `hx_gamerules.{h,cpp}` and `hx_convars.cpp` (Phase 3). ConVars are `hx_*`; replicate with `FCVAR_REPLICATED` only when the client needs them.
- `hl2/hl2_gamerules.cpp:1817` carries the one Cascade patch to upstream HL2 code so far (`AddAmmoType` overload ambiguity under GCC). Keep such patches one-line and commented.

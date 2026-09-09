# src/game/client — Cascade notes

- Cascade compiles `client_hl2.vpc` only. `tf/`, `hl2mp/`, `econ/`, `replay/` are not in the link — leave them alone.
- Cascade-owned client code goes in `hx/` with `CHx` classes and `hx_` files; hook it in via a `$Include` at the end of `client_hl2.vpc`, not by editing the upstream file list.
- HUD elements live in `hl2/hud_*.cpp`; client mode is `hl2/clientmode_hlnormal.cpp`. Subclass, do not edit, where possible.
- Cite `file:line` for any engine interface you call (C6).

# Cascade

A singleplayer game built on Valve's [Source SDK 2013](https://github.com/ValveSoftware/source-sdk-2013)
(2025 64-bit tree, HL2 branch). Runs on **Source SDK Base 2013 Multiplayer**. Linux-first.

## Requirements

- Steam with **Source SDK Base 2013 Multiplayer** (app 243750) installed
- `podman` (the build runs inside the Steam Runtime "sniper" SDK container)
- ~10 GB disk for the container image and build outputs

## Quickstart

```bash
git clone https://github.com/haptixxx-dev/source-sdk-2013 cascade && cd cascade
tools/dev.sh build                    # release build, first run pulls the sniper image
tools/dev.sh run --testmaps +map test_hardware   # runs inside the Steam Linux Runtime, windowed, console open
tools/dev.sh log                      # tail game/cascade/console.log in another terminal
```

`tools/dev.sh build debug` builds the debug configuration; `tools/dev.sh attach-gdb` attaches to it.
`tools/gen_brand_assets.py` regenerates the placeholder menu background, chapter tile and icons (needs Pillow).

## Layout

| Path | Contents |
|---|---|
| `src/` | Engine SDK + game code (SOURCE 1 SDK LICENSE, see `LICENSE`) |
| `game/cascade/` | Mod directory: `gameinfo.txt`, configs, resources, maps |
| `tools/` | Developer scripts (AGPL-3.0) |
| `docs/` | Architecture, plan, gameplay and release notes |

## Upstream

`upstream` remote = ValveSoftware/source-sdk-2013. Valve's original README is preserved in git history
(`git show upstream/master:README.md`). Windows builds are not supported yet — see `docs/PLAN.md`.

## License

Code under `src/` is licensed under the [SOURCE 1 SDK LICENSE](LICENSE). Tooling under `tools/` is
AGPL-3.0-or-later. Cascade is not affiliated with Valve Corporation.

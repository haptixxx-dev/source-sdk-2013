#!/usr/bin/env python3
# List the Valve assets Cascade currently depends on, for the art replacement brief.
#
# Copyright (C) 2026 Haptixxx
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Sources scanned:
#   1. asset paths hard-coded in the HL2 game code Cascade compiles
#      (src/game/{server,client,shared}/hl2)
#   2. weapon scripts shipped with Source SDK Base 2013 MP (hl2/scripts/weapon_*.txt)
#   3. HUD texture atlas list (game/cascade/scripts/hud_textures.txt)
#   4. per-folder counts of the HL2 VPKs (needs the SDK Base install; skipped if absent)
# Outputs land in docs/assets/. docs/ASSET_LIST.md is the hand-curated brief built on them.

import os
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
OUT = REPO / "docs" / "assets"
STEAM_COMMON = Path(os.environ.get("STEAM_COMMON", Path.home() / ".local/share/Steam/steamapps/common"))
SDK = STEAM_COMMON / "Source SDK Base 2013 Multiplayer"

CODE_DIRS = [REPO / "src/game/server/hl2", REPO / "src/game/client/hl2", REPO / "src/game/shared/hl2"]
REF_RE = re.compile(r'"((?:models|sprites|effects|particle|particles|materials|vgui|sound)/[A-Za-z0-9_./ -]+)', re.I)


def code_references():
    refs = set()
    for d in CODE_DIRS:
        for p in d.rglob("*.cpp"):
            refs.update(m.lower() for m in REF_RE.findall(p.read_text(errors="ignore")))
        for p in d.rglob("*.h"):
            refs.update(m.lower() for m in REF_RE.findall(p.read_text(errors="ignore")))
    return sorted(refs)


def weapon_scripts():
    rows = []
    for p in sorted((SDK / "hl2/scripts").glob("weapon_*.txt")):
        txt = p.read_text(errors="ignore")
        def kv(key):
            m = re.search(r'"%s"\s*"([^"]*)"' % key, txt, re.I)
            return m.group(1) if m else ""
        rows.append((p.stem, kv("viewmodel"), kv("playermodel"), kv("primary_ammo"), kv("secondary_ammo"), kv("bucket")))
    return rows


def hud_textures():
    p = REPO / "game/cascade/scripts/hud_textures.txt"
    return sorted(set(m.lower() for m in re.findall(r'"file"\s*"([^"]+)"', p.read_text(errors="ignore"), re.I)))


def vpk_counts():
    vpk = SDK / "bin/linux64/vpk"
    if not vpk.exists():
        return {}
    env = dict(os.environ, LD_LIBRARY_PATH=str(SDK / "bin/linux64"))
    out = {}
    for name in ("hl2/hl2_misc_dir.vpk", "hl2/hl2_textures_dir.vpk", "hl2/hl2_sound_misc_dir.vpk", "hl2/hl2_sound_vo_english_dir.vpk"):
        try:
            listing = subprocess.run([str(vpk), "l", str(SDK / name)], capture_output=True, text=True, env=env, check=True).stdout
        except subprocess.CalledProcessError:
            continue
        c = Counter("/".join(line.split("/")[:2]) for line in listing.splitlines() if "/" in line)
        out[name] = c
    return out


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    refs = code_references()
    (OUT / "code_references.txt").write_text(
        "# Asset paths hard-coded in src/game/{server,client,shared}/hl2 (lower-cased, deduplicated)\n" + "\n".join(refs) + "\n")
    rows = weapon_scripts()
    (OUT / "weapons.csv").write_text(
        "weapon,viewmodel,playermodel,primary_ammo,secondary_ammo,bucket\n" + "\n".join(",".join(r) for r in rows) + "\n")
    (OUT / "hud_textures.txt").write_text("# Texture atlases referenced by game/cascade/scripts/hud_textures.txt\n" + "\n".join(hud_textures()) + "\n")
    counts = vpk_counts()
    with (OUT / "vpk_counts.txt").open("w") as f:
        f.write("# Files per top-level folder in the SDK Base HL2 VPKs Cascade mounts\n")
        for name, c in counts.items():
            f.write(f"\n[{name}]\n")
            for folder, n in c.most_common(25):
                f.write(f"{n:6d}  {folder}\n")
    print(f"code references: {len(refs)}  weapons: {len(rows)}  vpk listings: {len(counts)}  -> {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

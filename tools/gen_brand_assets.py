#!/usr/bin/env python3
# Generate Cascade placeholder brand assets (VTF backgrounds, chapter thumbnail, icons) with Pillow.
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
# Writes uncompressed VTF 7.2 (RGBA8888, single mip) — enough for UI textures and
# authorable on Linux without Valve's vtex (constraint C3). Real art should still be
# authored with proper tools; this only produces placeholders.

import struct
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

REPO = Path(__file__).resolve().parent.parent
MOD = REPO / "game" / "cascade"

INK = (11, 15, 20)
PAPER = (232, 238, 242)
ACCENT = (56, 182, 255)
MUTED = (107, 122, 136)

IMAGE_FORMAT_RGBA8888 = 0
IMAGE_FORMAT_NONE = 0xFFFFFFFF
TEXTUREFLAGS_CLAMPS = 0x4
TEXTUREFLAGS_CLAMPT = 0x8
TEXTUREFLAGS_NOMIP = 0x100
TEXTUREFLAGS_NOLOD = 0x200
TEXTUREFLAGS_EIGHTBITALPHA = 0x2000


def write_vtf(path: Path, img: Image.Image) -> None:
    """VTF 7.2 header (80 bytes, padded) + one RGBA8888 frame, no mipmaps, no low-res copy."""
    img = img.convert("RGBA")
    w, h = img.size
    if w & (w - 1) or h & (h - 1):
        raise SystemExit(f"{path}: dimensions must be powers of two, got {w}x{h}")
    flags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_EIGHTBITALPHA
    # VTF 7.2 header layout (offsets): 0 sig, 4 version[2], 12 headerSize, 16 w, 18 h, 20 flags,
    # 24 frames, 26 firstFrame, 28 pad4, 32 reflectivity[3], 44 pad4, 48 bumpScale, 52 hiResFormat,
    # 56 mipCount, 57 loResFormat, 61 loResW, 62 loResH, 63 depth -> 65 bytes, padded to headerSize 80.
    header = struct.pack(
        "<4sIIIHHIHH4xfff4xfIBIBBH",
        b"VTF\0", 7, 2, 80, w, h, flags, 1, 0,
        1.0, 1.0, 1.0, 1.0,
        IMAGE_FORMAT_RGBA8888, 1,
        IMAGE_FORMAT_NONE, 0, 0,
        1,
    ).ljust(80, b"\0")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + img.tobytes("raw", "RGBA"))


def font(size: int) -> ImageFont.FreeTypeFont:
    for name in ("DejaVuSans-Bold.ttf", "LiberationSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def gradient(size, top, bottom):
    img = Image.new("RGBA", size)
    px = img.load()
    w, h = size
    for y in range(h):
        t = y / max(h - 1, 1)
        c = tuple(int(top[i] * (1 - t) + bottom[i] * t) for i in range(3)) + (255,)
        for x in range(w):
            px[x, y] = c
    return img


def background(size, content_aspect):
    """Console/main-menu background. The engine stretches the texture over the whole screen, so
    draw at the display aspect and squash to the power-of-two size; it un-squashes on screen.
    The left third stays quiet because the engine draws the menu there."""
    w, h = size
    cw, ch = w, int(round(w / content_aspect))
    img = gradient((cw, ch), INK, (18, 26, 36))
    d = ImageDraw.Draw(img)
    # cascade of accent bars falling from the top right, fading as they go
    for i in range(16):
        x = int(cw * 0.50 + i * cw * 0.03)
        y0 = int(ch * 0.04 + i * ch * 0.05)
        y1 = y0 + int(ch * 0.40 - i * ch * 0.018)
        d.rectangle([x, y0, x + int(cw * 0.010), y1], fill=ACCENT + (max(24, 190 - i * 11),))
    foot = font(int(ch * 0.022))
    d.text((int(cw * 0.60), int(ch * 0.93)), "placeholder background — docs/BRAND.md", font=foot, fill=MUTED)
    return img.resize(size, Image.LANCZOS)


def chapter_thumb():
    img = gradient((256, 128), (18, 26, 36), INK)
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, 255, 127], outline=ACCENT, width=2)
    d.text((14, 40), "CH 1", font=font(40), fill=PAPER)
    return img


def icon():
    img = Image.new("RGBA", (256, 256), INK + (255,))
    d = ImageDraw.Draw(img)
    for i in range(6):
        x = 60 + i * 24
        d.rectangle([x, 40 + i * 14, x + 10, 200 - i * 6], fill=ACCENT + (255,))
    d.text((70, 196), "C", font=font(48), fill=PAPER)
    return img


def main() -> int:
    write_vtf(MOD / "materials/console/background01.vtf", background((1024, 1024), 4 / 3))
    write_vtf(MOD / "materials/console/background01_widescreen.vtf", background((1024, 1024), 16 / 9))
    write_vtf(MOD / "materials/vgui/chapters/chapter1.vtf", chapter_thumb())
    ico = icon()
    sizes = [(256, 256), (48, 48), (32, 32), (16, 16)]
    ico.save(MOD / "resource/game.ico", format="ICO", sizes=sizes)
    ico.save(REPO / "src/launcher_main/res/cascade.ico", format="ICO", sizes=sizes)
    for p in ("materials/console/background01.vtf", "materials/console/background01_widescreen.vtf",
              "materials/vgui/chapters/chapter1.vtf", "resource/game.ico"):
        print(f"wrote {MOD / p} ({(MOD / p).stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Render a full-bleed Ghibli-style team slide from an illustration + text overlays."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = ROOT / "docs" / "misc" / "team_ghibli_source.png"
DEFAULT_LOGO = ROOT / "docs" / "misc" / "lg_electronics_logo.png"
DEFAULT_OUT_PNG = ROOT / "docs" / "misc" / "team_ghibli.png"
DEFAULT_OUT_JPG = ROOT / "docs" / "misc" / "team_ghibli.jpg"

TARGET_W, TARGET_H = 1920, 1080
MARGIN_X, MARGIN_Y = 96, 80


def _load_font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    candidates = []
    if bold:
        candidates.extend(
            [
                "C:/Windows/Fonts/malgunbd.ttf",
                "C:/Windows/Fonts/segoeuib.ttf",
                "C:/Windows/Fonts/arialbd.ttf",
            ]
        )
    else:
        candidates.extend(
            [
                "C:/Windows/Fonts/malgun.ttf",
                "C:/Windows/Fonts/segoeui.ttf",
                "C:/Windows/Fonts/arial.ttf",
            ]
        )
    for path in candidates:
        if Path(path).is_file():
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def cover_resize(img: Image.Image, width: int, height: int) -> Image.Image:
    """Scale and center-crop to fill the target size with no letterboxing."""
    src_w, src_h = img.size
    scale = max(width / src_w, height / src_h)
    new_w = int(src_w * scale)
    new_h = int(src_h * scale)
    resized = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
    left = (new_w - width) // 2
    top = (new_h - height) // 2
    return resized.crop((left, top, left + width, top + height))


def draw_text_with_shadow(
    draw: ImageDraw.ImageDraw,
    xy: tuple[int, int],
    text: str,
    font: ImageFont.ImageFont,
    fill: tuple[int, int, int, int],
    shadow: tuple[int, int, int, int] = (0, 0, 0, 140),
    offset: tuple[int, int] = (2, 2),
    anchor: str = "la",
) -> None:
    x, y = xy
    draw.text((x + offset[0], y + offset[1]), text, font=font, fill=shadow, anchor=anchor)
    draw.text(xy, text, font=font, fill=fill, anchor=anchor)


def draw_text_readable_on_light_bg(
    draw: ImageDraw.ImageDraw,
    xy: tuple[int, int],
    text: str,
    font: ImageFont.ImageFont,
    fill: tuple[int, int, int, int] = (255, 255, 255, 255),
    anchor: str = "la",
) -> None:
    """Soft halo + drop shadow so white text stays legible on bright clouds."""
    x, y = xy
    halo = (20, 30, 50, 155)
    for dx, dy in (
        (-2, 0),
        (2, 0),
        (0, -2),
        (0, 2),
        (-2, -2),
        (2, 2),
        (-2, 2),
        (2, -2),
    ):
        draw.text((x + dx, y + dy), text, font=font, fill=halo, anchor=anchor)
    draw.text((x + 3, y + 3), text, font=font, fill=(0, 0, 0, 145), anchor=anchor)
    draw.text(
        xy,
        text,
        font=font,
        fill=fill,
        anchor=anchor,
        stroke_width=3,
        stroke_fill=(15, 25, 45, 220),
    )


def _prepare_lg_logo(logo: Image.Image) -> Image.Image:
    """Drop black matte and brighten wordmark for overlay on photo art."""
    logo = logo.convert("RGBA")
    px = logo.load()
    w, h = logo.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if r < 45 and g < 45 and b < 45:
                px[x, y] = (0, 0, 0, 0)
                continue
            if 70 <= r <= 190 and abs(r - g) < 25 and abs(g - b) < 25:
                px[x, y] = (255, 255, 255, 255)
    return logo


def _paste_logo_bottom_right(canvas: Image.Image, logo_path: Path) -> None:
    logo = _prepare_lg_logo(Image.open(logo_path))
    target_h = 96
    scale = target_h / logo.height
    target_w = int(logo.width * scale)
    logo = logo.resize((target_w, target_h), Image.Resampling.LANCZOS)

    x = TARGET_W - MARGIN_X - target_w
    y = TARGET_H - MARGIN_Y - target_h
    canvas.alpha_composite(logo, (x, y))


def _draw_team_title_top_left(canvas: Image.Image) -> None:
    team_font = _load_font(94, bold=True)
    draw = ImageDraw.Draw(canvas)
    draw_text_readable_on_light_bg(
        draw,
        (MARGIN_X, MARGIN_Y),
        "Team 3 : Blue Sky",
        team_font,
    )


def render(source: Path, logo: Path, out_png: Path, out_jpg: Path) -> None:
    img = Image.open(source).convert("RGBA")
    canvas = cover_resize(img, TARGET_W, TARGET_H)

    _draw_team_title_top_left(canvas)

    if logo.is_file():
        _paste_logo_bottom_right(canvas, logo)
    else:
        draw = ImageDraw.Draw(canvas)
        lg_font = _load_font(40, bold=True)
        white = (255, 255, 255, 255)
        draw_text_with_shadow(
            draw,
            (TARGET_W - MARGIN_X, TARGET_H - MARGIN_Y),
            "LG Electronics",
            lg_font,
            white,
            anchor="ra",
        )

    out_png.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(out_png, format="PNG", optimize=True)
    canvas.convert("RGB").save(out_jpg, format="JPEG", quality=92, optimize=True, progressive=True)
    print(f"Wrote {out_png}")
    print(f"Wrote {out_jpg}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Add text overlays to Ghibli team illustration.")
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE, help="Ghibli illustration PNG")
    parser.add_argument("--logo", type=Path, default=DEFAULT_LOGO, help="LG Electronics logo PNG")
    parser.add_argument("--out-png", type=Path, default=DEFAULT_OUT_PNG)
    parser.add_argument("--out-jpg", type=Path, default=DEFAULT_OUT_JPG)
    args = parser.parse_args()

    if not args.source.is_file():
        raise SystemExit(f"Source image not found: {args.source}")

    render(args.source, args.logo, args.out_png, args.out_jpg)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate the Open Graph preview image (1200x630) for the playground and docs site.

The image is committed as web/public/og.png, so this script only needs to run when
the design changes. It needs Pillow and the Noto Sans KR variable font (SIL OFL):

    curl -L -o NotoSansKR.ttf "https://github.com/google/fonts/raw/main/ofl/notosanskr/NotoSansKR%5Bwght%5D.ttf"
    python3 scripts/generate_og_image.py --font NotoSansKR.ttf --out web/public/og.png
"""
import argparse

from PIL import Image, ImageDraw, ImageFont

W, H = 1200, 630
BRAND = (52, 81, 178)        # #3451b2, same as the favicon
CODE_BG = (31, 49, 110)
WHITE = (255, 255, 255)
MUTED = (200, 210, 245)
ACCENT = (255, 214, 102)


def font(path: str, size: int, weight: int) -> ImageFont.FreeTypeFont:
    f = ImageFont.truetype(path, size)
    try:
        f.set_variation_by_axes([weight])  # variable font: wght axis
    except (OSError, AttributeError):
        pass
    return f


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--font", required=True, help="path to NotoSansKR[wght].ttf")
    ap.add_argument("--out", default="web/public/og.png")
    args = ap.parse_args()

    img = Image.new("RGB", (W, H), BRAND)
    d = ImageDraw.Draw(img)

    # Logo: the consonant ㅅ drawn as in the favicon (64x64 viewBox scaled to 160 px)
    x0, y0, s = 80, 80, 160 / 64
    d.rounded_rectangle([x0, y0, x0 + 160, y0 + 160], radius=int(14 * s), fill=WHITE)
    apex = (x0 + 32 * s, y0 + 13 * s)
    for foot in ((x0 + 13 * s, y0 + 52 * s), (x0 + 51 * s, y0 + 52 * s)):
        d.line([apex, foot], fill=BRAND, width=int(8 * s))
    for p in (apex, (x0 + 13 * s, y0 + 52 * s), (x0 + 51 * s, y0 + 52 * s)):
        r = 4 * s
        d.ellipse([p[0] - r, p[1] - r, p[0] + r, p[1] + r], fill=BRAND)

    d.text((280, 92), "KingSejong", font=font(args.font, 96, 800), fill=WHITE)
    d.text((284, 232), "한글 문법으로 쓰는 프로그래밍 언어", font=font(args.font, 40, 500), fill=MUTED)

    # A real line of josa method chaining (with 짝수 and 제곱 defined, it prints [4, 16, 36])
    box = [80, 330, W - 80, 500]
    d.rounded_rectangle(box, radius=24, fill=CODE_BG)
    code = font(args.font, 34, 500)
    d.text((120, 362), "[1, 2, 3, 4, 5, 6]을 걸러낸다(짝수)을 변환한다(제곱)", font=code, fill=WHITE)
    d.text((120, 425), "# [4, 16, 36]", font=code, fill=ACCENT)

    d.text((80, 545), "플레이그라운드 · 튜토리얼 · 레퍼런스", font=font(args.font, 30, 500), fill=MUTED)
    d.text((W - 80, 545), "kingsejong-lang.github.io", font=font(args.font, 30, 500), fill=MUTED, anchor="ra")

    img.save(args.out, optimize=True)
    print(f"wrote {args.out} ({W}x{H})")


if __name__ == "__main__":
    main()

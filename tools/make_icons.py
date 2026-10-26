# Generates EasyKey app, updater and tray icons from one geometry so they match.
# Usage: python tools/make_icons.py <out_dir>  (needs Pillow); copy the .ico files into Sources/EasyKey/win32/EasyKey.
import sys
from PIL import Image, ImageDraw
OUT = sys.argv[1]
SS = 8  # supersampling

RECT = (0.06, 0.17, 0.94, 0.83)
RADIUS = 0.17
V = [(0.335, 0.34), (0.5, 0.665), (0.665, 0.34)]
VW = 0.095

def px(v, n): return v * n

def draw_v(d, n, color, w=VW):
    pts = [(px(x, n), px(y, n)) for x, y in V]
    d.line(pts, fill=color, width=int(px(w, n)), joint='curve')
    r = px(w, n) / 2
    for x, y in pts:
        d.ellipse((x - r, y - r, x + r, y + r), fill=color)

def draw_e(d, n, color, w=0.085):
    x0, x1, y0, y1 = 0.36, 0.64, 0.33, 0.67
    W = px(w, n)
    d.rectangle((px(x0, n), px(y0, n), px(x0, n) + W, px(y1, n)), fill=color)
    for y in (y0, (y0 + y1) / 2 - w / 2, y1 - w):
        xe = x1 if y != (y0 + y1) / 2 - w / 2 else x1 - 0.03
        d.rectangle((px(x0, n), px(y, n), px(xe, n), px(y, n) + W), fill=color)

def rect_box(n): return tuple(px(v, n) for v in RECT)

def gradient(n, top, bottom):
    g = Image.new('RGBA', (n, n))
    d = ImageDraw.Draw(g)
    for y in range(n):
        t = y / (n - 1)
        d.line([(0, y), (n, y)], fill=tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (255,))
    return g

def app(n, badge=False):
    N = n * SS
    mask = Image.new('L', (N, N), 0)
    ImageDraw.Draw(mask).rounded_rectangle(rect_box(N), radius=px(RADIUS, N), fill=255)
    img = Image.new('RGBA', (N, N), (0, 0, 0, 0))
    img.paste(gradient(N, (0x3A, 0x96, 0xF3), (0x00, 0x5A, 0xBE)), (0, 0), mask)
    d = ImageDraw.Draw(img)
    draw_v(d, N, (255, 255, 255, 255))
    if badge:  # updater: white circle with a down arrow at the bottom right
        cx, cy, r = px(0.76, N), px(0.76, N), px(0.22, N)
        d.ellipse((cx - r, cy - r, cx + r, cy + r), fill=(255, 255, 255, 255))
        r2 = r * 0.82
        d.ellipse((cx - r2, cy - r2, cx + r2, cy + r2), fill=(0x10, 0x8A, 0x3E, 255))
        a = r2 * 0.55
        d.line([(cx, cy - a), (cx, cy + a * 0.6)], fill=(255, 255, 255, 255), width=int(r2 * 0.28))
        d.polygon([(cx - a * 0.75, cy + a * 0.05), (cx + a * 0.75, cy + a * 0.05), (cx, cy + a)], fill=(255, 255, 255, 255))
    return img.resize((n, n), Image.LANCZOS)

def tray_viet(n, color):
    N = n * SS
    img = Image.new('RGBA', (N, N), (0, 0, 0, 0))
    ImageDraw.Draw(img).rounded_rectangle(rect_box(N), radius=px(RADIUS, N), fill=color)
    hole = Image.new('L', (N, N), 0)
    draw_v(ImageDraw.Draw(hole), N, 255)
    img.putalpha(Image.composite(Image.new('L', (N, N), 0), img.getchannel('A'), hole))
    return img.resize((n, n), Image.LANCZOS)

def tray_eng(n, color):
    N = n * SS
    img = Image.new('RGBA', (N, N), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle(rect_box(N), radius=px(RADIUS, N), outline=color, width=int(px(0.075, N)))
    draw_e(d, N, color)
    return img.resize((n, n), Image.LANCZOS)

def save_ico(path, frames):
    frames = sorted(frames, key=lambda f: -f.size[0])
    frames[0].save(path, format='ICO', sizes=[f.size for f in frames], append_images=frames[1:])

APP = [16, 20, 24, 32, 40, 48, 64, 128, 256]
TRAY = [16, 20, 24, 32, 40, 48, 64]
WHITE, DARK = (255, 255, 255, 255), (0x1B, 0x1B, 0x1B, 255)
save_ico(f'{OUT}/icon.ico', [app(n) for n in APP])
save_ico(f'{OUT}/EasyKeyUpdate.ico', [app(n, True) for n in APP])
save_ico(f'{OUT}/small.ico', [app(n, True) for n in [16, 20, 24, 32, 48]])
save_ico(f'{OUT}/StatusVietDark.ico', [tray_viet(n, WHITE) for n in TRAY])
save_ico(f'{OUT}/StatusVietLight.ico', [tray_viet(n, DARK) for n in TRAY])
save_ico(f'{OUT}/StatusEngDark.ico', [tray_eng(n, WHITE) for n in TRAY])
save_ico(f'{OUT}/StatusEngLight.ico', [tray_eng(n, DARK) for n in TRAY])

# preview sheet
sheet = Image.new('RGBA', (900, 380), (32, 32, 32, 255))
light = Image.new('RGBA', (450, 120), (243, 243, 243, 255)); sheet.paste(light, (450, 260))
x = 10
for n in [256, 64, 32, 16]:
    sheet.paste(app(n), (x, 10), app(n)); x += n + 10
x = 10
for n in [128, 32, 16]:
    u = app(n, True); sheet.paste(u, (x, 270 - (n - 32) // 2 if n < 100 else 250), u); x += n + 10
x = 470
for f in (tray_viet, tray_eng):
    for n in (48, 24, 16):
        i = f(n, WHITE); sheet.paste(i, (x, 200), i)
        j = f(n, DARK); sheet.paste(j, (x, 290), j); x += n + 12
sheet.save(f'{OUT}/preview.png')

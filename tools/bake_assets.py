#!/usr/bin/env python3
"""Bake surfaces and icons from resources/theme/tokens.json into PNG.

Run after changing a token: tools/bake_assets.py
Needs pycairo and Pillow. Output is committed, the build does not run this.
"""

import json
import math
import re
from pathlib import Path

import cairo
from PIL import Image, ImageFilter

ROOT = Path(__file__).resolve().parent.parent
RES = ROOT / "resources"
TOKENS = json.loads((RES / "theme" / "tokens.json").read_text())
SIZE = TOKENS["size"]

PAD_L, PAD_T, PAD_R, PAD_B = (
    SIZE["shadow-left"], SIZE["shadow-top"], SIZE["shadow-right"], SIZE["shadow-bottom"])
TILE_W = 16  # bội của chu kỳ Bayer để phần giữa lặp ngang không lộ nối
EDGE_FADE = 4

BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def rgba(name):
    h = TOKENS["color"][name].lstrip("#")
    a = 255
    if len(h) == 8:
        a, h = int(h[:2], 16), h[2:]
    return (int(h[:2], 16), int(h[2:4], 16), int(h[4:6], 16), a / 255)


class Layer:
    """Straight-alpha float RGBA buffer."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [(0.0, 0.0, 0.0, 0.0)] * (w * h)

    def over(self, mask, color):
        """color is (r, g, b, a) or a function (x, y) -> (r, g, b, a)."""
        for y in range(self.h):
            for x in range(self.w):
                m = mask[y * self.w + x] / 255
                if m == 0:
                    continue
                r, g, b, a = color(x, y) if callable(color) else color
                sa = a * m
                dr, dg, db, da = self.px[y * self.w + x]
                oa = sa + da * (1 - sa)
                self.px[y * self.w + x] = (
                    (r * sa + dr * da * (1 - sa)) / oa,
                    (g * sa + dg * da * (1 - sa)) / oa,
                    (b * sa + db * da * (1 - sa)) / oa,
                    oa)

    def save(self, path):
        """Opaque pixels are quantised to RGB565 with ordered dither, because Qt only
        truncates when it blits onto a 16-bit framebuffer."""
        img = Image.new("RGBA", (self.w, self.h))
        out = []
        for y in range(self.h):
            for x in range(self.w):
                r, g, b, a = self.px[y * self.w + x]
                if a >= 0.999:
                    t = (BAYER[y % 4][x % 4] + 0.5) / 16
                    r5 = min(31, int(r / 255 * 31 + t))
                    g6 = min(63, int(g / 255 * 63 + t))
                    b5 = min(31, int(b / 255 * 31 + t))
                    out.append(((r5 << 3) | (r5 >> 2), (g6 << 2) | (g6 >> 4), (b5 << 3) | (b5 >> 2), 255))
                else:
                    out.append((round(r), round(g), round(b), round(a * 255)))
        img.putdata(out)
        path.parent.mkdir(parents=True, exist_ok=True)
        img.save(path, optimize=True)


def rrect_path(c, x, y, w, h, tl, tr=None, br=None, bl=None):
    tr = tl if tr is None else tr
    br = tl if br is None else br
    bl = tl if bl is None else bl
    c.new_sub_path()
    c.arc(x + w - tr, y + tr, tr, -math.pi / 2, 0)
    c.arc(x + w - br, y + h - br, br, 0, math.pi / 2)
    c.arc(x + bl, y + h - bl, bl, math.pi / 2, math.pi)
    c.arc(x + tl, y + tl, tl, math.pi, 3 * math.pi / 2)
    c.close_path()


def mask_of(w, h, draw):
    s = cairo.ImageSurface(cairo.FORMAT_A8, w, h)
    c = cairo.Context(s)
    draw(c)
    s.flush()
    stride, data = s.get_stride(), bytes(s.get_data())
    return [data[y * stride + x] for y in range(h) for x in range(w)]


def rmask(W, H, x, y, w, h, radii):
    def draw(c):
        rrect_path(c, x, y, w, h, *radii)
        c.fill()
    return mask_of(W, H, draw)


def blur(mask, w, h, sigma):
    if sigma == 0:
        return mask
    # Nới biên để blur không bị cắt tại mép ảnh, nhất là với ảnh inset.
    m = int(sigma * 4) + 2
    im = Image.new("L", (w + 2 * m, h + 2 * m))
    im.paste(Image.frombytes("L", (w, h), bytes(mask)), (m, m))
    im = im.filter(ImageFilter.GaussianBlur(sigma)).crop((m, m, m + w, m + h))
    return list(im.tobytes())


def shifted(mask, w, h, dy, fill=0):
    return [mask[(y - dy) * w + x] if 0 <= y - dy < h else fill for y in range(h) for x in range(w)]


def fade_edges(mask, w, h, rect):
    """Bóng bị cắt ở mép PNG sẽ lộ một vạch cứng, nên giảm dần về 0 ở các mép có vùng bóng."""
    x0, y0, rw, rh = rect

    def ramp(dist):
        return min(1, (dist + 0.5) / EDGE_FADE)

    out = []
    for y in range(h):
        for x in range(w):
            k = 1.0
            if x0 > 0:
                k *= ramp(x)
            if x0 + rw < w:
                k *= ramp(w - 1 - x)
            if y0 > 0:
                k *= ramp(y)
            if y0 + rh < h:
                k *= ramp(h - 1 - y)
            out.append(int(mask[y * w + x] * k))
    return out


def vertical(top, bottom, y0, h):
    a, b = rgba(top), rgba(bottom)

    def color(_x, y):
        t = min(1, max(0, (y + 0.5 - y0) / h))
        return tuple(a[i] + (b[i] - a[i]) * t for i in range(3)) + (1.0,)
    return color


def capped(top, bottom, y0, h, cap):
    """Gradient chỉ nằm ở hai đầu, phần giữa phẳng để thẻ cao bất kỳ khi kéo giãn không lộ sọc dither."""
    grad = vertical(top, bottom, y0, 2 * cap)
    mid = grad(0, y0 + cap)

    def color(x, y):
        if y < y0 + cap:
            return grad(x, y)
        if y >= y0 + h - cap:
            return grad(x, y - (h - 2 * cap))
        return mid
    return color


def diagonal135(start, end, x0, y0, w, h):
    """CSS linear-gradient(135deg, ...)."""
    a, b = rgba(start), rgba(end)
    s = math.sqrt(0.5)
    length = (w + h) * s

    def color(x, y):
        t = ((x + 0.5 - x0 - w / 2) * s + (y + 0.5 - y0 - h / 2) * s) / length + 0.5
        return tuple(a[i] + (b[i] - a[i]) * t for i in range(3)) + (1.0,)
    return color


def surface(name, W, H, rect, radii, fill, shadow=None, highlight=None, inset=None):
    """shadow: (dy, css_blur, token); inset: (dy, css_blur, token)."""
    x, y, w, h = rect
    mask = rmask(W, H, x, y, w, h, radii)
    layer = Layer(W, H)
    if shadow:
        dy, b, token = shadow
        soft = blur(shifted(mask, W, H, dy), W, H, b / 2)
        layer.over(fade_edges(soft, W, H, rect), rgba(token))
    layer.over(mask, fill)
    if inset:
        dy, b, token = inset
        outside = shifted([255 - v for v in mask], W, H, dy, 255)
        soft = blur(outside, W, H, b / 2)
        layer.over([s * m // 255 for s, m in zip(soft, mask)], rgba(token))
    if highlight:
        below = shifted(mask, W, H, 1)
        layer.over([max(0, m - b) for m, b in zip(mask, below)], rgba(highlight))
    layer.save(RES / "surfaces" / f"{name}.png")


def padded(w, h):
    return w + PAD_L + PAD_R, h + PAD_T + PAD_B, (PAD_L, PAD_T, w, h)


def bake_surfaces():
    bg = Layer(240, 320)
    bg.over([255] * (240 * 320), vertical("bg-top", "bg-bottom", 0, 320))
    bg.save(RES / "surfaces" / "background.png")

    R = TOKENS["radius"]
    card_shadow = (3, 9, "shadow-card")
    small_shadow = (2, 5, "shadow-small")

    # Thẻ chung: cao bất kỳ, co giãn phần giữa.
    W, H, rect = padded(2 * R["card"] + TILE_W, 2 * R["card"] + TILE_W)
    surface("card", W, H, rect, (R["card"],), capped("card-top", "card-bottom", PAD_T, rect[3], R["card"]),
            card_shadow, "highlight")

    for name, w, h, start, end, shadow in (
            ("tile_temp", 224, 128, "temp-start", "temp-end", "temp-shadow"),
            ("tile_humi", 108, 99, "humi-start", "humi-end", "humi-shadow"),
            ("tile_light", 108, 99, "light-start", "light-end", "light-shadow")):
        W, H, rect = padded(w, h)
        surface(name, W, H, rect, (R["card"],), diagonal135(start, end, PAD_L, PAD_T, w, h),
                (4, 9, shadow), "highlight")

    # Nút cao 42 px, chỉ lặp phần giữa theo chiều ngang.
    bw = 2 * R["button"] + TILE_W
    W, H, rect = padded(bw, 42)
    grad = lambda t, b: vertical(t, b, PAD_T, 42)
    press = (2, 4, "shadow-inset")
    surface("btn_primary", W, H, rect, (R["button"],), grad("pri-top", "pri-bottom"),
            (3, 8, "pri-shadow"), "pri-highlight")
    surface("btn_primary_pressed", W, H, rect, (R["button"],), grad("pri-pressed-top", "pri-pressed-bottom"), inset=press)
    surface("btn_secondary", W, H, rect, (R["button"],), grad("sec-top", "sec-bottom"), small_shadow, "highlight")
    surface("btn_secondary_pressed", W, H, rect, (R["button"],), grad("sec-pressed-top", "sec-pressed-bottom"), inset=press)
    surface("btn_danger", W, H, rect, (R["button"],), grad("dng-top", "dng-bottom"),
            (3, 8, "dng-shadow"), "dng-highlight")
    surface("btn_danger_pressed", W, H, rect, (R["button"],), grad("dng-pressed-top", "dng-pressed-bottom"), inset=press)
    surface("btn_disabled", W, H, rect, (R["button"],), rgba("track"))

    # Nút lớn ở S5c: cao 92 px, có PNG riêng vì kéo giãn sẽ làm hỏng gradient và dither.
    W, H, rect = padded(bw, 92)
    tall = lambda t, b: vertical(t, b, PAD_T, 92)
    surface("btn_big", W, H, rect, (R["button"],), tall("pri-top", "pri-bottom"), (3, 8, "pri-shadow"), "pri-highlight")
    surface("btn_big_pressed", W, H, rect, (R["button"],), tall("pri-pressed-top", "pri-pressed-bottom"), inset=press)

    # Nút quay lại: vòng 30 px, vùng chạm 44×40.
    back = (8, 4, 30, 30)
    backgrad = lambda t, b: vertical(t, b, 4, 30)
    surface("back", 44, 40, back, (15,), backgrad("sec-top", "sec-bottom"), small_shadow, "highlight")
    surface("back_pressed", 44, 40, back, (15,), backgrad("sec-pressed-top", "sec-pressed-bottom"), inset=press)

    # Bottom sheet: chỉ bo hai góc trên, bóng hắt lên trên.
    r = R["sheet"]
    sw, sh = 2 * r + TILE_W, 2 * r + TILE_W
    surface("sheet", sw, sh + 12, (0, 12, sw, sh + 8), (r, r, 0, 0),
            capped("card-top", "card-bottom", 12, sh, r), (-4, 12, "shadow-sheet"), "highlight")

    nav_h = SIZE["nav-height"]
    surface("nav", 240, nav_h, (-4, 0, 248, nav_h + 8), (0,), vertical("card-top", "card-bottom", 0, nav_h),
            highlight="highlight")

    # Track lõm: segment, °C/°F, slider.
    inset = (1, 3, "shadow-inset")
    seg_r = R["button"]
    surface("seg_track", 2 * seg_r + TILE_W, 36, (0, 0, 2 * seg_r + TILE_W, 36), (seg_r,), rgba("track"), inset=inset)
    on_shadow = (1, 2, "shadow-seg")
    seg_on = R["seg"]
    surface("seg_on", 2 + 2 * seg_on + TILE_W, 36, (1, 3, 2 * seg_on + TILE_W, 30), (seg_on,),
            vertical("pri-top", "pri-bottom", 3, 30), on_shadow, "seg-highlight")
    mini = R["seg"]
    surface("mini_track", 2 * mini + TILE_W, 32, (0, 0, 2 * mini + TILE_W, 32), (mini,), rgba("track"), inset=inset)
    mini_on = R["mini"]
    surface("mini_on", 2 * mini_on + TILE_W, 28, (0, 0, 2 * mini_on + TILE_W, 28), (mini_on,),
            vertical("pri-top", "pri-bottom", 0, 28), highlight="seg-highlight")

    for name in ("default", "yellow", "aqua", "red", "blue", "green"):
        surface(f"icon_tile_{name}", 30, 30, (2, 1, 26, 26), (R["seg"],),
                vertical(f"icon-tile-{name}-top", f"icon-tile-{name}-bottom", 1, 26),
                (1, 3, "icon-tile-shadow"), "icon-tile-highlight")

    surface("halo_primary", 76, 75, (6, 4, 64, 64), (32,), vertical("pri-top", "pri-bottom", 4, 64),
            (3, 8, "pri-shadow"))
    surface("halo_danger", 76, 75, (6, 4, 64, 64), (32,), vertical("dng-top", "dng-bottom", 4, 64),
            (3, 8, "dng-shadow"))
    surface("version_track", 2 * seg_r + TILE_W, 39, (0, 0, 2 * seg_r + TILE_W, 39), (seg_r,), rgba("track"),
            inset=inset)

    inset_small = (1, 2, "shadow-inset")
    surface("slider_track", 24, 8, (0, 0, 24, 8), (4,), rgba("track"), inset=inset_small)
    surface("slider_fill", 24, 8, (0, 0, 24, 8), (4,), vertical("pri-top", "pri-bottom", 0, 8))
    surface("slider_knob", 32, 32, (4, 3, 24, 24), (12,), vertical("knob-top", "knob-bottom", 3, 24),
            (2, 5, "shadow-knob"))

    for name, track, knob_x in (("toggle_off", rgba("track"), 3),
                                ("toggle_on", vertical("pri-top", "pri-bottom", 0, 26), 21)):
        mask = rmask(44, 26, 0, 0, 44, 26, (13,))
        layer = Layer(44, 26)
        layer.over(mask, track)
        if name == "toggle_off":
            soft = blur(shifted([255 - v for v in mask], 44, 26, 1, 255), 44, 26, 1.5)
            layer.over([s * m // 255 for s, m in zip(soft, mask)], rgba("shadow-inset"))
        knob = rmask(44, 26, knob_x, 3, 20, 20, (10,))
        layer.over(blur(shifted(knob, 44, 26, 1), 44, 26, 1.5), rgba("shadow-knob"))
        layer.over(knob, vertical("knob-top", "knob-bottom", 3, 20))
        layer.save(RES / "surfaces" / f"{name}.png")


WIFI_ARCS = '<path d="M2.5 9a14 14 0 0 1 19 0"/><path d="M6 13a9 9 0 0 1 12 0"/>'
WIFI_INNER = '<path d="M9.5 16.6a4 4 0 0 1 5 0"/><path d="M12 20h.01"/>'

# Hình vector từ mockup, viewBox 24×24, nét 2.
ICONS = {
    "network": ('<rect x="9" y="3" width="6" height="5"/><rect x="3" y="16" width="6" height="5"/>'
                '<rect x="15" y="16" width="6" height="5"/><path d="M12 8v4M6 16v-4h12v4"/>', (15, 16)),
    "network_off": ('<rect x="9" y="3" width="6" height="5"/><rect x="3" y="16" width="6" height="5"/>'
                    '<rect x="15" y="16" width="6" height="5"/><path d="M12 8v4M6 16v-4h12v4"/>'
                    '<path d="M3 3l18 18"/>', (15,)),
    "wifi": (WIFI_ARCS + WIFI_INNER, (15, 16)),
    "wifi_weak": (WIFI_ARCS.replace('<path ', '<path opacity=".3" ') + WIFI_INNER, (15,)),
    "wifi_off": (WIFI_ARCS + WIFI_INNER + '<path d="M3 3l18 18"/>', (15,)),
    "thermometer": ('<path d="M10 4a2 2 0 0 1 4 0v10a4 4 0 1 1-4 0z"/>', (16,)),
    "droplet": ('<path d="M12 3s7 7.5 7 12a7 7 0 0 1-14 0c0-4.5 7-12 7-12z"/>', (16,)),
    "sun": ('<circle cx="12" cy="12" r="4"/>'
            '<path d="M12 2v3M12 19v3M2 12h3M19 12h3M5 5l2 2M17 17l2 2M19 5l-2 2M7 17l-2 2"/>', (16,)),
    "home": ('<path d="M3 11 12 4l9 7"/><path d="M6 10v10h12V10"/>', (22,)),
    "chart": ('<path d="M4 20V4"/><path d="M4 20h16"/><path d="m7 15 4-5 3 3 5-7"/>', (22,)),
    "settings": ('<path d="M12.22 2h-.44a2 2 0 0 0-2 2v.18a2 2 0 0 1-1 1.73l-.43.25a2 2 0 0 1-2 0l-.15-.08a2 2 0 0 0-2.73.7'
                 '3l-.22.38a2 2 0 0 0 .73 2.73l.15.1a2 2 0 0 1 1 1.72v.51a2 2 0 0 1-1 1.74l-.15.09a2 2 0 0 0-.73 2'
                 '.73l.22.38a2 2 0 0 0 2.73.73l.15-.08a2 2 0 0 1 2 0l.43.25a2 2 0 0 1 1 1.73V20a2 2 0 0 0 2 2h.44a'
                 '2 2 0 0 0 2-2v-.18a2 2 0 0 1 1-1.73l.43-.25a2 2 0 0 1 2 0l.15.08a2 2 0 0 0 2.73-.73l.22-.39a2 2 '
                 '0 0 0-.73-2.73l-.15-.08a2 2 0 0 1-1-1.74v-.5a2 2 0 0 1 1-1.74l.15-.09a2 2 0 0 0 .73-2.73l-.22-.3'
                 '8a2 2 0 0 0-2.73-.73l-.15.08a2 2 0 0 1-2 0l-.43-.25a2 2 0 0 1-1-1.73V4a2 2 0 0 0-2-2z"/><circle cx="12" cy="12" r="3"/>', (22,)),
    "chevron": ('<path d="m9 6 6 6-6 6"/>', (14,)),
    "back": ('<path d="m15 5-7 7 7 7"/>', (18,)),
    "clock": ('<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/>', (16, 34)),
    "globe": ('<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3c5 5 5 13 0 18M12 3c-5 5-5 13 0 18"/>', (16,)),
    "download": ('<path d="M12 4v11"/><path d="m7 11 5 5 5-5"/><path d="M4 20h16"/>', (16,)),
    "wrench": ('<path d="M14 6a4 4 0 0 0 5 5l-9 9a2.8 2.8 0 0 1-4-4l9-9a4 4 0 0 1 4-5l-3 3 1 3z"/>', (16,)),
    "check": ('<path d="m5 12 5 5 9-10"/>', (12, 18, 34)),
    "pin": ('<path d="M12 21s7-6.2 7-11.5A7 7 0 0 0 5 9.5C5 14.8 12 21 12 21z"/>'
            '<circle cx="12" cy="9.5" r="2.5"/>', (16,)),
    "cpu": ('<rect x="6" y="6" width="12" height="12" rx="1"/>'
            '<path d="M9 3v3M15 3v3M9 18v3M15 18v3M3 9h3M3 15h3M18 9h3M18 15h3"/>', (16,)),
    "sensor": ('<circle cx="12" cy="12" r="2"/><path d="M7 7a7 7 0 0 0 0 10M17 7a7 7 0 0 1 0 10"/>', (16,)),
    "crosshair": ('<path d="M12 3v18M3 12h18"/><circle cx="12" cy="12" r="5"/>', (16, 34)),
    "log": ('<path d="M6 3h12v18H6z"/><path d="M9 8h6M9 12h6M9 16h4"/>', (16,)),
    "close": ('<path d="M6 6l12 12M18 6 6 18"/>', (12, 34)),
    "device": ('<rect x="4" y="5" width="16" height="11" rx="1.5"/><path d="M9 20h6M12 16v4"/>', (16,)),
    "system": ('<rect x="3" y="7" width="18" height="10" rx="1"/><path d="M7 7v10M12 7v10M17 7v10"/>', (16,)),
    "code": ('<path d="m8 7-5 5 5 5M16 7l5 5-5 5"/>', (16,)),
    "restore": ('<path d="M4 5v5h5"/><path d="M4.6 10A8 8 0 1 1 4 13"/>', (16,)),
}


def svg_arc(c, x1, y1, rx, ry, phi, large, sweep, x2, y2):
    """Endpoint to centre parameterisation, SVG 1.1 F.6.5. Only rx == ry occurs here."""
    assert rx == ry and phi == 0
    r = rx
    dx, dy = (x1 - x2) / 2, (y1 - y2) / 2
    lam = (dx * dx + dy * dy) / (r * r)
    if lam > 1:
        r *= math.sqrt(lam)
    k = math.sqrt(max(0, (r ** 4 - r * r * (dx * dx + dy * dy)) / (r * r * (dx * dx + dy * dy))))
    if large == sweep:
        k = -k
    cx = k * dy + (x1 + x2) / 2
    cy = -k * dx + (y1 + y2) / 2
    a1 = math.atan2(y1 - cy, x1 - cx)
    a2 = math.atan2(y2 - cy, x2 - cx)
    if sweep:
        c.arc(cx, cy, r, a1, a2)
    else:
        c.arc_negative(cx, cy, r, a1, a2)


def svg_path(c, d):
    tokens = re.findall(r"[MmLlHhVvCcSsAaZz]|-?\d*\.?\d+", d)
    i, cmd = 0, None
    x = y = sx = sy = 0.0
    last_ctrl = None

    def num():
        nonlocal i
        v = float(tokens[i])
        i += 1
        return v

    while i < len(tokens):
        if tokens[i].isalpha():
            cmd = tokens[i]
            i += 1
            if cmd in "Zz":
                c.close_path()
                x, y = sx, sy
                continue
        rel = cmd.islower()
        op = cmd.upper()
        ox, oy = (x, y) if rel else (0.0, 0.0)
        ctrl = None
        if op == "M":
            x, y = ox + num(), oy + num()
            sx, sy = x, y
            c.move_to(x, y)
            cmd = "l" if rel else "L"
        elif op == "L":
            x, y = ox + num(), oy + num()
            c.line_to(x, y)
        elif op == "H":
            x = ox + num()
            c.line_to(x, y)
        elif op == "V":
            y = oy + num()
            c.line_to(x, y)
        elif op == "C":
            p = [ox + num(), oy + num(), ox + num(), oy + num(), ox + num(), oy + num()]
            c.curve_to(*p)
            ctrl, (x, y) = (p[2], p[3]), (p[4], p[5])
        elif op == "S":
            cx1, cy1 = (2 * x - last_ctrl[0], 2 * y - last_ctrl[1]) if last_ctrl else (x, y)
            p = [ox + num(), oy + num(), ox + num(), oy + num()]
            c.curve_to(cx1, cy1, *p)
            ctrl, (x, y) = (p[0], p[1]), (p[2], p[3])
        elif op == "A":
            rx, ry, phi, large, sweep = num(), num(), num(), num(), num()
            nx, ny = ox + num(), oy + num()
            svg_arc(c, x, y, rx, ry, phi, large, sweep, nx, ny)
            x, y = nx, ny
        last_ctrl = ctrl


def bake_icon(name, svg, size):
    s = cairo.ImageSurface(cairo.FORMAT_ARGB32, size, size)
    c = cairo.Context(s)
    c.scale(size / 24, size / 24)
    c.set_line_width(2)
    c.set_line_cap(cairo.LINE_CAP_ROUND)
    c.set_line_join(cairo.LINE_JOIN_ROUND)
    for tag, body in re.findall(r"<(rect|circle|path) ([^>]*)/>", svg):
        a = dict(re.findall(r'(\w+)="([^"]*)"', body))
        c.new_path()
        if tag == "rect":
            x, y, w, h = (float(a[k]) for k in ("x", "y", "width", "height"))
            rx = float(a.get("rx", 0))
            if rx:
                rrect_path(c, x, y, w, h, rx)
            else:
                c.rectangle(x, y, w, h)
        elif tag == "circle":
            c.arc(float(a["cx"]), float(a["cy"]), float(a["r"]), 0, 2 * math.pi)
            c.close_path()
        else:
            svg_path(c, a["d"])
        c.set_source_rgba(0, 0, 0, float(a.get("opacity", 1)))
        c.stroke()
    s.flush()
    alpha = bytes(s.get_data())[3::4]
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    img.putalpha(Image.frombytes("L", (size, size), alpha))
    path = RES / "icons" / f"{name}_{size}.png"
    img.save(path, optimize=True)


def bake_icons():
    for name, (svg, sizes) in ICONS.items():
        for size in sizes:
            bake_icon(name, svg, size)


def write_qrc():
    files = sorted(p.relative_to(RES).as_posix() for p in (RES / "surfaces").glob("*.png"))
    files += sorted(f"icons/{n}_{s}.png" for n, (_, sizes) in ICONS.items() for s in sizes)
    entries = "\n".join(f"        <file>{f}</file>" for f in files)
    (RES / "assets.qrc").write_text(
        f'<!DOCTYPE RCC>\n<RCC version="1.0">\n    <qresource prefix="/">\n{entries}\n    </qresource>\n</RCC>\n')


if __name__ == "__main__":
    bake_surfaces()
    bake_icons()
    write_qrc()

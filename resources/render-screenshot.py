#!/usr/bin/env python3
"""Render bitwise's own output to a PNG for the README.

The screenshots in this directory used to be grabbed by hand with scrot (see
snip.txt), which meant they went stale every time the output grew a field.
This does the same job reproducibly: it runs the real binary under a pty so
bitwise emits its actual colors, parses the ANSI escapes, and draws them on
the character grid of a terminal.

The palette is Konsole's Breeze theme, which is what the original hand-taken
screenshots used -- keeping it means regenerated images still sit next to the
older ones without looking out of place.

Requires python3-pil. Run it from the top of the build tree, after "make":

    ./resources/render-screenshot.py 'bitwise 0xdeadbeef' resources/cmdline.png
    ./resources/render-screenshot.py 'bitwise "(1 << 31) | 0xff"' \
        resources/conversion.png

Those two commands regenerate the pair the README's quick tour uses. Both
arguments are chosen so the high bit is set, which is what makes the signed
and unsigned decimal lines differ.
"""
import argparse
import re
import subprocess
import sys

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit("This needs Pillow: apt-get install python3-pil")

BG = (0x23, 0x26, 0x27)
FG = (0xFC, 0xFC, 0xFC)

# Konsole Breeze, normal and intense.
NORMAL = {0: (0x23, 0x26, 0x27), 1: (0xED, 0x15, 0x15), 2: (0x11, 0xD1, 0x16),
          3: (0xF6, 0x74, 0x00), 4: (0x1D, 0x99, 0xF3), 5: (0x9B, 0x59, 0xB6),
          6: (0x1A, 0xBC, 0x9C), 7: (0xFC, 0xFC, 0xFC)}
BRIGHT = {0: (0x7F, 0x8C, 0x8D), 1: (0xC0, 0x39, 0x2B), 2: (0x1C, 0xDC, 0x9A),
          3: (0xFD, 0xBC, 0x4B), 4: (0x3D, 0xAE, 0xE9), 5: (0x8E, 0x44, 0xAD),
          6: (0x16, 0xA0, 0x85), 7: (0xFF, 0xFF, 0xFF)}

# One terminal cell, in unscaled pixels, measured off the original screenshots.
CELL_W, CELL_H, FONT_SIZE, PAD = 8, 17, 13, 8

FONT_DIR = "/usr/share/fonts/truetype/dejavu"
REGULAR = FONT_DIR + "/DejaVuSansMono.ttf"
BOLD = FONT_DIR + "/DejaVuSansMono-Bold.ttf"

# The prompt the existing screenshots were taken with; kept so regenerated
# images match the ones they replace.
PROMPT = "stdcall@stdcall-pc:~/dev"

SGR = re.compile(r"\x1b\[([0-9;]*)m")


def capture(command):
    """Run command under a pty, so bitwise colors its output, and return it."""
    done = subprocess.run(["script", "-qec", command, "/dev/null"],
                          capture_output=True)
    if done.returncode:
        sys.exit("%s failed:\n%s" % (command, done.stderr.decode()))
    text = done.stdout.decode("utf-8", "replace")
    return text.replace("\r\n", "\n").replace("\r", "")


def parse(text):
    """Turn ANSI text into lines of (character, color, bold) cells."""
    lines, cells = [], []
    color, bold = FG, False
    i = 0
    while i < len(text):
        match = SGR.match(text, i)
        if match:
            for param in (match.group(1) or "0").split(";"):
                param = int(param or 0)
                if param == 0:
                    color, bold = FG, False
                elif param == 1:
                    bold = True
                elif 30 <= param <= 37:
                    color = (BRIGHT if bold else NORMAL)[param - 30]
                elif 90 <= param <= 97:
                    color = BRIGHT[param - 90]
            i = match.end()
            continue
        if text[i] == "\n":
            lines.append(cells)
            cells = []
        else:
            cells.append((text[i], color, bold))
        i += 1
    if cells:
        lines.append(cells)
    while lines and not lines[-1]:
        lines.pop()
    return lines


def prompt_line(command):
    """The shell prompt above the output, styled like the old screenshots."""
    return ([(c, BRIGHT[2], True) for c in PROMPT] +
            [(c, FG, True) for c in " $ " + command])


def render(lines, path, scale, colors):
    cell_w, cell_h, pad = CELL_W * scale, CELL_H * scale, PAD * scale
    size = FONT_SIZE * scale
    width = max((len(line) for line in lines), default=0) * cell_w + 2 * pad
    height = len(lines) * cell_h + 2 * pad

    image = Image.new("RGB", (width, height), BG)
    draw = ImageDraw.Draw(image)
    regular = ImageFont.truetype(REGULAR, size)
    bold_font = ImageFont.truetype(BOLD, size)

    for row, line in enumerate(lines):
        # Characters are placed on the grid rather than advanced by the font,
        # so columns line up exactly as a terminal would draw them.
        y = pad + row * cell_h + (cell_h - size) // 2 - scale
        for col, (char, color, bold) in enumerate(line):
            if char != " ":
                draw.text((pad + col * cell_w, y), char,
                          font=bold_font if bold else regular, fill=color)

    if colors:
        image = image.quantize(colors=colors, method=Image.MEDIANCUT)
    image.save(path, optimize=True)
    print("%s: %dx%d" % (path, width, height))


def main():
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        epilog="Example: %(prog)s 'bitwise 0xdeadbeef' resources/cmdline.png")
    parser.add_argument("command", help="the bitwise invocation to screenshot")
    parser.add_argument("output", help="PNG to write")
    parser.add_argument("--scale", type=int, default=2,
                        help="supersampling factor, for high-DPI (default: 2)")
    parser.add_argument("--colors", type=int, default=256,
                        help="palette size, 0 for truecolor (default: 256)")
    args = parser.parse_args()

    command = args.command
    if not command.startswith(("./", "/")):
        command = "./" + command      # the freshly built binary, not $PATH

    render([prompt_line(args.command)] + parse(capture(command)),
           args.output, args.scale, args.colors)


if __name__ == "__main__":
    main()

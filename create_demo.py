"""Capture actual executable output and render it as a terminal replay GIF.

Optional documentation tool: python -m pip install pillow
The C++ application and tests do not depend on Python or Pillow.
"""
import argparse
import os
import subprocess
import textwrap
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--executable", type=Path)
options = parser.parse_args()
exe = options.executable or ROOT / "bin" / ("airports.exe" if os.name == "nt" else "airports")
result = subprocess.run(
    [str(exe.resolve()), "--script", "docs/demo.commands"],
    cwd=ROOT, capture_output=True, text=True, encoding="utf-8", check=True,
)
assert "113 airports, 127 nodes visited" in result.stdout
assert "Deleted AEY (case D3: two children" in result.stdout
assert "Same shape as previous tree: yes" in result.stdout
assert "Xiaoxing Tong Airport, Shanghai, China" in result.stdout
(ROOT / "docs/demo-session.txt").write_text(result.stdout, encoding="utf-8")

font_paths = [
    Path("C:/Windows/Fonts/consola.ttf"),
    Path("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"),
    Path("/System/Library/Fonts/Menlo.ttc"),
]
font_path = next((path for path in font_paths if path.exists()), None)
font = ImageFont.truetype(str(font_path), 19) if font_path else ImageFont.load_default(size=19)
small = ImageFont.truetype(str(font_path), 16) if font_path else ImageFont.load_default(size=16)
width, height = 1280, 800
char_width = font.getlength("M")
columns = int((width - 72) / char_width)
visible_rows = 27

sections = []
for line in result.stdout.splitlines():
    if line.startswith("> "):
        sections.append([line, []])
    elif sections:
        sections[-1][1].append(line)

frames, durations = [], []


def render(command, output, step):
    image = Image.new("RGB", (width, height), "#0d1522")
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, width, 91), fill="#172338")
    draw.text((32, 19), "MINI PROJECT 02  |  AIRPORT BST  |  Xiaoxing Tong", font=font, fill="#f1f5f9")
    draw.text((32, 53), "Terminal session replay - captured executable output", font=small, fill="#a8bad2")
    lines = []
    for original in [command] + output:
        wrapped = textwrap.wrap(original, width=columns, replace_whitespace=False,
                                drop_whitespace=False) or [""]
        for part in wrapped:
            lines.append((part, original.startswith("> "), original))
    for index, (line, is_command, original) in enumerate(lines[-visible_rows:]):
        color = "#70dcff" if is_command else "#dde5f0"
        if "validate: OK" in original or "nodes visited" in original or "Same shape" in original:
            color = "#8de7b6"
        draw.text((32, 112 + index * 23), line, font=font, fill=color)
    draw.line((32, 749, width - 32, 749), fill="#2a3e58")
    draw.text((32, 766), f"Step {step}/{len(sections)}   {command[2:]}", font=small, fill="#a8bad2")
    frames.append(image)


for step, (command, output) in enumerate(sections, start=1):
    render(command, [], step)
    durations.append(650)
    if len(output) > 20:
        for count in range(16, len(output), 16):
            render(command, output[:count], step)
            durations.append(260)
    render(command, output, step)
    durations.append(2300 if command.startswith("> remove") else 1800)

frames[0].save(ROOT / "docs/demo.gif", save_all=True, append_images=frames[1:],
               duration=durations, loop=0, optimize=False)
print(f"Captured {len(sections)} commands; {len(frames)} GIF frames; {sum(durations) / 1000:.1f} seconds")

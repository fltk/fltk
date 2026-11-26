#!/usr/bin/env python3
#
# Generates the "FLTK Pixel" demo font for examples/howto-load-fonts.cxx
#
# Copyright 2026 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#     https://www.fltk.org/bugs.php
#
# FLTK Pixel is a 5x7 pixel font covering printable ASCII. It is generated
# entirely by this script and is distributed under the FLTK license.
#
# Usage: pip install fonttools; python3 make_fltk_pixel.py
# Writes FLTKPixel-{Regular,Bold,Italic,BoldItalic}.ttf and FLTKPixel.h
# (Regular as a C array) into the directory of this script.

import os
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

# Rows from top to bottom, 7 rows above the baseline, optional 2 below.
GLYPHS = {
  ' ': [".....", ".....", ".....", ".....", ".....", ".....", "....."],
  '!': ["..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.."],
  '"': [".#.#.", ".#.#.", ".#.#.", ".....", ".....", ".....", "....."],
  '#': [".#.#.", ".#.#.", "#####", ".#.#.", "#####", ".#.#.", ".#.#."],
  '$': ["..#..", ".####", "#.#..", ".###.", "..#.#", "####.", "..#.."],
  '%': ["##...", "##..#", "...#.", "..#..", ".#...", "#..##", "...##"],
  '&': [".##..", "#..#.", "#.#..", ".#...", "#.#.#", "#..#.", ".##.#"],
  "'": ["..#..", "..#..", "..#..", ".....", ".....", ".....", "....."],
  '(': ["...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#."],
  ')': [".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#..."],
  '*': [".....", "..#..", "#.#.#", ".###.", "#.#.#", "..#..", "....."],
  '+': [".....", "..#..", "..#..", "#####", "..#..", "..#..", "....."],
  ',': [".....", ".....", ".....", ".....", ".....", "..##.", "..##.", "...#.", "..#.."],
  '-': [".....", ".....", ".....", "#####", ".....", ".....", "....."],
  '.': [".....", ".....", ".....", ".....", ".....", ".##..", ".##.."],
  '/': [".....", "....#", "...#.", "..#..", ".#...", "#....", "....."],
  '0': [".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."],
  '1': ["..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."],
  '2': [".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"],
  '3': ["#####", "...#.", "..#..", "...#.", "....#", "#...#", ".###."],
  '4': ["...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."],
  '5': ["#####", "#....", "####.", "....#", "....#", "#...#", ".###."],
  '6': ["..##.", ".#...", "#....", "####.", "#...#", "#...#", ".###."],
  '7': ["#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."],
  '8': [".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."],
  '9': [".###.", "#...#", "#...#", ".####", "....#", "...#.", ".##.."],
  ':': [".....", ".##..", ".##..", ".....", ".##..", ".##..", "....."],
  ';': [".....", ".##..", ".##..", ".....", ".##..", ".##..", "..#..", ".#..."],
  '<': ["...#.", "..#..", ".#...", "#....", ".#...", "..#..", "...#."],
  '=': [".....", ".....", "#####", ".....", "#####", ".....", "....."],
  '>': [".#...", "..#..", "...#.", "....#", "...#.", "..#..", ".#..."],
  '?': [".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.."],
  '@': [".###.", "#...#", "....#", ".##.#", "#.#.#", "#.#.#", ".###."],
  'A': [".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
  'B': ["####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."],
  'C': [".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."],
  'D': ["###..", "#..#.", "#...#", "#...#", "#...#", "#..#.", "###.."],
  'E': ["#####", "#....", "#....", "####.", "#....", "#....", "#####"],
  'F': ["#####", "#....", "#....", "####.", "#....", "#....", "#...."],
  'G': [".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".####"],
  'H': ["#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
  'I': [".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."],
  'J': ["..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."],
  'K': ["#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"],
  'L': ["#....", "#....", "#....", "#....", "#....", "#....", "#####"],
  'M': ["#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"],
  'N': ["#...#", "#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#"],
  'O': [".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
  'P': ["####.", "#...#", "#...#", "####.", "#....", "#....", "#...."],
  'Q': [".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"],
  'R': ["####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"],
  'S': [".####", "#....", "#....", ".###.", "....#", "....#", "####."],
  'T': ["#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
  'U': ["#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
  'V': ["#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."],
  'W': ["#...#", "#...#", "#...#", "#.#.#", "#.#.#", "#.#.#", ".#.#."],
  'X': ["#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"],
  'Y': ["#...#", "#...#", "#...#", ".#.#.", "..#..", "..#..", "..#.."],
  'Z': ["#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"],
  '[': [".###.", ".#...", ".#...", ".#...", ".#...", ".#...", ".###."],
  '\\': [".....", "#....", ".#...", "..#..", "...#.", "....#", "....."],
  ']': [".###.", "...#.", "...#.", "...#.", "...#.", "...#.", ".###."],
  '^': ["..#..", ".#.#.", "#...#", ".....", ".....", ".....", "....."],
  '_': [".....", ".....", ".....", ".....", ".....", ".....", "#####"],
  '`': [".#...", "..#..", "...#.", ".....", ".....", ".....", "....."],
  'a': [".....", ".....", ".###.", "....#", ".####", "#...#", ".####"],
  'b': ["#....", "#....", "#.##.", "##..#", "#...#", "#...#", "####."],
  'c': [".....", ".....", ".###.", "#....", "#....", "#...#", ".###."],
  'd': ["....#", "....#", ".##.#", "#..##", "#...#", "#...#", ".####"],
  'e': [".....", ".....", ".###.", "#...#", "#####", "#....", ".###."],
  'f': ["..##.", ".#..#", ".#...", "###..", ".#...", ".#...", ".#..."],
  'g': [".....", ".....", ".####", "#...#", "#...#", "#...#", ".####", "....#", ".###."],
  'h': ["#....", "#....", "#.##.", "##..#", "#...#", "#...#", "#...#"],
  'i': ["..#..", ".....", ".##..", "..#..", "..#..", "..#..", ".###."],
  'j': ["...#.", ".....", "..##.", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."],
  'k': ["#....", "#....", "#..#.", "#.#..", "##...", "#.#..", "#..#."],
  'l': [".##..", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."],
  'm': [".....", ".....", "##.#.", "#.#.#", "#.#.#", "#...#", "#...#"],
  'n': [".....", ".....", "#.##.", "##..#", "#...#", "#...#", "#...#"],
  'o': [".....", ".....", ".###.", "#...#", "#...#", "#...#", ".###."],
  'p': [".....", ".....", "####.", "#...#", "#...#", "#...#", "####.", "#....", "#...."],
  'q': [".....", ".....", ".####", "#...#", "#...#", "#...#", ".####", "....#", "....#"],
  'r': [".....", ".....", "#.##.", "##..#", "#....", "#....", "#...."],
  's': [".....", ".....", ".###.", "#....", ".###.", "....#", "####."],
  't': [".#...", ".#...", "###..", ".#...", ".#...", ".#..#", "..##."],
  'u': [".....", ".....", "#...#", "#...#", "#...#", "#..##", ".##.#"],
  'v': [".....", ".....", "#...#", "#...#", "#...#", ".#.#.", "..#.."],
  'w': [".....", ".....", "#...#", "#...#", "#.#.#", "#.#.#", ".#.#."],
  'x': [".....", ".....", "#...#", ".#.#.", "..#..", ".#.#.", "#...#"],
  'y': [".....", ".....", "#...#", "#...#", "#...#", "#...#", ".####", "....#", ".###."],
  'z': [".....", ".....", "#####", "...#.", "..#..", ".#...", "#####"],
  '{': ["...#.", "..#..", "..#..", ".#...", "..#..", "..#..", "...#."],
  '|': ["..#..", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
  '}': [".#...", "..#..", "..#..", "...#.", "..#..", "..#..", ".#..."],
  '~': [".....", ".....", ".#...", "#.#.#", "...#.", ".....", "....."],
}

PIXEL = 100       # font units per pixel, 1000 units per em
LSB = 50          # left side bearing
ADVANCE = 600     # 5 pixels plus one pixel of spacing
BOLD_EXTRA = 50   # bold widens every horizontal run of pixels
SLANT = 0.2       # italic shear, about 11 degrees

STYLES = [  # style name, bold, italic
  ("Regular", False, False),
  ("Bold", True, False),
  ("Italic", False, True),
  ("Bold Italic", True, True),
]


def draw_glyph(rows, bold, italic):
  # collect horizontal runs of pixels, then merge equal runs of adjacent rows
  rects = []  # [first column, end column, top row, bottom row]
  for r, row in enumerate(rows):
    c = 0
    while c < len(row):
      if row[c] != '#':
        c += 1
        continue
      start = c
      while c < len(row) and row[c] == '#':
        c += 1
      for rect in rects:
        if rect[0] == start and rect[1] == c and rect[3] == r - 1:
          rect[3] = r
          break
      else:
        rects.append([start, c, r, r])
  pen = TTGlyphPen(None)
  extra = BOLD_EXTRA if bold else 0
  for c0, c1, r0, r1 in rects:
    x0 = LSB + c0 * PIXEL
    x1 = LSB + c1 * PIXEL + extra
    y0 = (6 - r1) * PIXEL
    y1 = (7 - r0) * PIXEL
    # clockwise rectangle, sheared for italic
    pts = [(x0, y0), (x0, y1), (x1, y1), (x1, y0)]
    if italic:
      pts = [(x + y * SLANT, y) for x, y in pts]
    pts = [(round(x), round(y)) for x, y in pts]
    pen.moveTo(pts[0])
    for p in pts[1:]:
      pen.lineTo(p)
    pen.closePath()
  return pen.glyph()


def notdef_glyph():
  pen = TTGlyphPen(None)
  for pts in ([(50, 0), (50, 700), (550, 700), (550, 0)],       # outer, clockwise
              [(150, 100), (450, 100), (450, 600), (150, 600)]):  # inner, counter-clockwise
    pen.moveTo(pts[0])
    for p in pts[1:]:
      pen.lineTo(p)
    pen.closePath()
  return pen.glyph()


def build(style, bold, italic):
  family = "FLTK Pixel"
  ps_name = "FLTKPixel-" + style.replace(" ", "")
  names = {c: "uni%04X" % ord(c) for c in GLYPHS}
  order = [".notdef"] + [names[c] for c in GLYPHS]
  advance = ADVANCE + (BOLD_EXTRA if bold else 0)

  fb = FontBuilder(1000, isTTF=True)
  fb.setupGlyphOrder(order)
  fb.setupCharacterMap({ord(c): names[c] for c in GLYPHS})
  glyphs = {".notdef": notdef_glyph()}
  for c, rows in GLYPHS.items():
    glyphs[names[c]] = draw_glyph(rows, bold, italic)
  fb.setupGlyf(glyphs)
  metrics = {}
  for name, g in glyphs.items():
    g.recalcBounds(fb.font["glyf"])
    metrics[name] = (advance, getattr(g, "xMin", 0))
  fb.setupHorizontalMetrics(metrics)
  fb.setupHorizontalHeader(ascent=800, descent=-200)
  fb.setupNameTable({
    "copyright": "Copyright 2026 by Bill Spitzak and others",
    "familyName": family,
    "styleName": style,
    "uniqueFontIdentifier": ps_name,
    "fullName": family + ("" if style == "Regular" else " " + style),
    "psName": ps_name,
    "licenseDescription": "FLTK License, see https://www.fltk.org/COPYING.php",
  })
  fs_selection = (0x20 if bold else 0) | (0x01 if italic else 0) or 0x40  # 0x40: regular
  fb.setupOS2(usWeightClass=700 if bold else 400, fsSelection=fs_selection,
              sTypoAscender=800, sTypoDescender=-200, sTypoLineGap=0,
              usWinAscent=800, usWinDescent=200, sxHeight=500, sCapHeight=700,
              ulCodePageRange1=1, achVendID="FLTK")  # code page 1252, GDI needs it
  fb.setupPost(italicAngle=-11.3 if italic else 0, isFixedPitch=1, keepGlyphNames=False)
  fb.font["head"].macStyle = (1 if bold else 0) | (2 if italic else 0)
  return fb.font


def c_array(name, data):
  lines = ["static const unsigned char %s[%d] = {" % (name, len(data))]
  for i in range(0, len(data), 24):
    lines.append("  " + ",".join("%d" % b for b in data[i:i + 24]) + ",")
  lines.append("};\n")
  return "\n".join(lines)


def main():
  here = os.path.dirname(os.path.abspath(__file__))
  header = ["// FLTK Pixel font, generated by make_fltk_pixel.py - do not edit\n"]
  for style, bold, italic in STYLES:
    path = os.path.join(here, "FLTKPixel-%s.ttf" % style.replace(" ", ""))
    build(style, bold, italic).save(path)
    if style == "Regular":
      with open(path, "rb") as f:
        header.append(c_array("fltk_pixel_%s_ttf" % style.lower(), f.read()))
    print("wrote", path, os.path.getsize(path), "bytes")
  with open(os.path.join(here, "FLTKPixel.h"), "w") as f:
    f.write("\n".join(header))


if __name__ == "__main__":
  main()

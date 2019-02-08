"""
Generates a shields.io-like code coverage SVG badge from lcov .info files.
Accepts a path to .info file and destination path (including filename) to output SVG.

MIT License

Copyright (c) 2019 Vilkov Adel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import io
import sys

USAGE = "Usage: python lcov-badger.py (path-to-info-file) (path-for-output-svg)"

SVG_TEMPLATE = """
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="96" height="20">
 <linearGradient id="b" x2="0" y2="100%"><stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
  <stop offset="1" stop-opacity=".1"/>
 </linearGradient>
 <clipPath id="a">
  <rect width="96" height="20" rx="3" fill="#fff"/>
 </clipPath>
 <g clip-path="url(#a)">
  <path fill="#555" d="M0 0h61v20H0z"/>
  <path fill="#4c1" d="M61 0h35v20H61z"/>
  <path fill="url(#b)" d="M0 0h96v20H0z"/>
 </g>
 <g fill="#fff" text-anchor="middle" font-family="DejaVu Sans,Verdana,Geneva,sans-serif" font-size="110">
  <text x="315" y="150" fill="#010101" fill-opacity=".3" transform="scale(.1)" textLength="510">coverage</text>
  <text x="315" y="140" transform="scale(.1)" textLength="510">coverage</text>
  <text x="775" y="150" fill="#010101" fill-opacity=".3" transform="scale(.1)" textLength="250">{{PERCENT}}</text>
  <text x="775" y="140" transform="scale(.1)" textLength="250">{{PERCENT}}</text>
 </g>
</svg>
"""

def create_svg(percent):
    return SVG_TEMPLATE.replace("{{PERCENT}}", str(percent) + '%')

def extract_coverage(data):
    lines = data.split("\n")
    lines_found = float(next(line[3:] for line in lines if line.startswith("LF:")))
    lines_exec = float(next(line[3:] for line in lines if line.startswith("LH:")))
    return int(round(lines_exec / lines_found * 100))

if (len(sys.argv) != 3):
    print(USAGE)
    exit(-1)

source_path = sys.argv[1]
svg_path = sys.argv[2]
print("Reading coverage info from " + source_path)

info = ""
with open(source_path, 'r') as info_file:
    info = info_file.read()

coverage = extract_coverage(info)
badge_data = create_svg(coverage)

print("Creating a coverage badge " + svg_path)
with open(svg_path, 'w') as badge_file:
    badge_file.write(badge_data)
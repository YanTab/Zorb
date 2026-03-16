"""Crop a centered square from a heightmap and optionally resize.

Usage:
	python crop_heightmap.py --input in.png --output out.png --size 2049
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def main() -> None:
	parser = argparse.ArgumentParser(description="Center-crop and resize a heightmap")
	parser.add_argument("--input", required=True, type=Path, help="Input image")
	parser.add_argument("--output", required=True, type=Path, help="Output image")
	parser.add_argument("--size", required=True, type=int, help="Final square size, e.g. 1025 or 2049")
	args = parser.parse_args()

	image = Image.open(args.input)
	arr = np.array(image)

	h, w = arr.shape[:2]
	side = min(h, w)
	y0 = (h - side) // 2
	x0 = (w - side) // 2
	cropped = arr[y0 : y0 + side, x0 : x0 + side]

	cropped_img = Image.fromarray(cropped)
	resample = Image.Resampling.BILINEAR if args.size != side else Image.Resampling.NEAREST
	out_img = cropped_img.resize((args.size, args.size), resample=resample)

	args.output.parent.mkdir(parents=True, exist_ok=True)
	out_img.save(args.output)
	print(f"Saved cropped heightmap: {args.output}")


if __name__ == "__main__":
	main()

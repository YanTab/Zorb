"""Generate a visual preview image from a heightmap.

Usage:
	python generate_preview.py --input in.png --output preview.png
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def colorize(norm: np.ndarray) -> np.ndarray:
	# Terrain-like gradient: dark valleys -> green hills -> bright ridges.
	stops = np.array(
		[
			[0.00, 20, 35, 55],
			[0.20, 35, 85, 50],
			[0.45, 90, 125, 65],
			[0.65, 125, 110, 80],
			[0.85, 165, 160, 140],
			[1.00, 235, 235, 235],
		],
		dtype=np.float64,
	)

	out = np.zeros((*norm.shape, 3), dtype=np.float64)
	x = stops[:, 0]
	for c in range(3):
		out[..., c] = np.interp(norm, x, stops[:, c + 1])
	return out.astype(np.uint8)


def main() -> None:
	parser = argparse.ArgumentParser(description="Generate colored preview from a heightmap")
	parser.add_argument("--input", required=True, type=Path, help="Input heightmap")
	parser.add_argument("--output", required=True, type=Path, help="Output preview png")
	args = parser.parse_args()

	image = Image.open(args.input)
	arr = np.array(image, dtype=np.float64)
	if arr.ndim == 3:
		arr = arr[..., 0]

	min_v = float(arr.min())
	max_v = float(arr.max())
	norm = np.zeros_like(arr) if max_v - min_v < 1e-8 else (arr - min_v) / (max_v - min_v)

	rgb = colorize(norm)
	args.output.parent.mkdir(parents=True, exist_ok=True)
	Image.fromarray(rgb, mode="RGB").save(args.output)
	print(f"Saved preview image: {args.output}")


if __name__ == "__main__":
	main()

"""Normalize a grayscale heightmap to full 16-bit range.

Usage:
	python normalize_heightmap.py --input in.png --output out.png
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def load_heightmap(path: Path) -> np.ndarray:
	image = Image.open(path)
	if image.mode not in ("I;16", "I", "L"):
		image = image.convert("L")
	arr = np.array(image, dtype=np.float64)
	if arr.ndim == 3:
		arr = arr[..., 0]
	return arr


def normalize_to_uint16(arr: np.ndarray) -> np.ndarray:
	min_v = float(arr.min())
	max_v = float(arr.max())
	if max_v - min_v < 1e-8:
		return np.zeros_like(arr, dtype=np.uint16)
	norm = (arr - min_v) / (max_v - min_v)
	return np.clip(norm * 65535.0, 0, 65535).astype(np.uint16)


def main() -> None:
	parser = argparse.ArgumentParser(description="Normalize heightmap to 16-bit grayscale")
	parser.add_argument("--input", required=True, type=Path, help="Input image path")
	parser.add_argument("--output", required=True, type=Path, help="Output 16-bit PNG path")
	args = parser.parse_args()

	heightmap = load_heightmap(args.input)
	out = normalize_to_uint16(heightmap)

	args.output.parent.mkdir(parents=True, exist_ok=True)
	Image.fromarray(out).save(args.output)
	print(f"Saved normalized heightmap: {args.output}")


if __name__ == "__main__":
	main()

"""Generate a procedural 16-bit heightmap with hills, valleys, ridges, and bumps.

Usage:
    python generate_heightmap.py --output out.png --size 2049 --seed 42
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def _smooth_noise(size: int, rng: np.random.Generator, scale: int) -> np.ndarray:
    coarse_h = max(2, size // scale)
    coarse_w = max(2, size // scale)
    coarse = rng.random((coarse_h, coarse_w), dtype=np.float64)
    coarse_img = Image.fromarray((coarse * 65535.0).astype(np.uint16))
    up = coarse_img.resize((size, size), resample=Image.Resampling.BICUBIC)
    return np.array(up, dtype=np.float64) / 65535.0


def _fbm(size: int, rng: np.random.Generator) -> np.ndarray:
    layers = [
        (24, 1.00),
        (12, 0.70),
        (6, 0.45),
        (3, 0.20),
    ]
    acc = np.zeros((size, size), dtype=np.float64)
    total_w = 0.0
    for scale, weight in layers:
        acc += _smooth_noise(size, rng, scale) * weight
        total_w += weight
    return acc / total_w


def _gaussian_hill(xx: np.ndarray, yy: np.ndarray, cx: float, cy: float, sigma: float, amp: float) -> np.ndarray:
    d2 = (xx - cx) ** 2 + (yy - cy) ** 2
    return amp * np.exp(-d2 / (2.0 * sigma * sigma))


def build_terrain(size: int, seed: int) -> np.ndarray:
    rng = np.random.default_rng(seed)

    y, x = np.mgrid[0:size, 0:size]
    xx = x / (size - 1)
    yy = y / (size - 1)

    terrain = _fbm(size, rng)

    # Add broad mountain/valley structures.
    macro = np.zeros_like(terrain)
    features = [
        (0.25, 0.30, 0.16, +1.00),
        (0.70, 0.25, 0.18, +0.85),
        (0.55, 0.70, 0.22, -0.95),
        (0.20, 0.75, 0.12, +0.60),
        (0.82, 0.62, 0.10, -0.55),
    ]
    for cx, cy, sigma, amp in features:
        macro += _gaussian_hill(xx, yy, cx, cy, sigma, amp)

    # Add a directional ridge to create speed lines for rolling gameplay.
    ridge = np.sin((xx * 9.0 + yy * 5.5) * np.pi) * 0.10

    # Add fine bumps for micro-variation.
    bumps = (_smooth_noise(size, rng, 2) - 0.5) * 0.10

    terrain = 0.55 * terrain + 0.35 * macro + ridge + bumps

    # Carve a gentle bowl near center to keep gameplay area readable.
    cx, cy = 0.5, 0.5
    center_dist = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2)
    terrain -= np.clip(0.20 - center_dist, 0.0, 0.20) * 0.6

    # Normalize and add shoreline-like flattening near outer bounds.
    terrain = (terrain - terrain.min()) / max(1e-8, terrain.max() - terrain.min())
    edge = np.minimum.reduce([xx, yy, 1.0 - xx, 1.0 - yy])
    edge_mask = np.clip(edge / 0.08, 0.0, 1.0)
    terrain = terrain * edge_mask + 0.04 * (1.0 - edge_mask)

    return np.clip(terrain, 0.0, 1.0)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate a procedural 16-bit terrain heightmap")
    parser.add_argument("--output", required=True, type=Path, help="Output 16-bit png")
    parser.add_argument("--size", type=int, default=2049, help="Heightmap size (recommended: 1009, 2017, 2049)")
    parser.add_argument("--seed", type=int, default=42, help="Random seed for reproducibility")
    args = parser.parse_args()

    if args.size < 129:
        raise ValueError("--size must be at least 129")

    terrain = build_terrain(args.size, args.seed)
    height_u16 = np.clip(terrain * 65535.0, 0, 65535).astype(np.uint16)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(height_u16).save(args.output)
    print(f"Saved procedural heightmap: {args.output}")


if __name__ == "__main__":
    main()

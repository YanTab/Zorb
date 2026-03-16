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


def _box_blur(arr: np.ndarray, radius: int, passes: int = 1) -> np.ndarray:
    if radius <= 0 or passes <= 0:
        return arr

    out = arr
    for _ in range(passes):
        # Horizontal pass
        padded = np.pad(out, ((0, 0), (radius, radius)), mode="edge")
        csum = np.cumsum(padded, axis=1)
        csum = np.pad(csum, ((0, 0), (1, 0)), mode="constant", constant_values=0.0)
        out = (csum[:, 2 * radius + 1 :] - csum[:, : -(2 * radius + 1)]) / float(2 * radius + 1)

        # Vertical pass
        padded = np.pad(out, ((radius, radius), (0, 0)), mode="edge")
        csum = np.cumsum(padded, axis=0)
        csum = np.pad(csum, ((1, 0), (0, 0)), mode="constant", constant_values=0.0)
        out = (csum[2 * radius + 1 :, :] - csum[: -(2 * radius + 1), :]) / float(2 * radius + 1)

    return out


def _fbm(size: int, rng: np.random.Generator) -> np.ndarray:
    layers = [
        (48, 1.00),
        (24, 0.65),
        (12, 0.30),
        (8, 0.18),
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
        (0.24, 0.28, 0.22, +0.70),
        (0.72, 0.25, 0.24, +0.55),
        (0.52, 0.70, 0.28, -0.60),
        (0.20, 0.76, 0.18, +0.35),
        (0.84, 0.60, 0.16, -0.30),
    ]
    for cx, cy, sigma, amp in features:
        macro += _gaussian_hill(xx, yy, cx, cy, sigma, amp)

    # Add smooth directional undulation to create rolling lanes.
    ridge = np.sin((xx * 4.0 + yy * 2.7) * np.pi) * 0.045

    # Add mild micro-variation only.
    bumps = (_smooth_noise(size, rng, 16) - 0.5) * 0.02

    terrain = 0.58 * terrain + 0.34 * macro + ridge + bumps

    # Carve a gentle bowl near center to keep gameplay area readable.
    cx, cy = 0.5, 0.5
    center_dist = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2)
    terrain -= np.clip(0.20 - center_dist, 0.0, 0.20) * 0.6

    # Global smoothing to remove spikes and make rolling gameplay readable.
    blur_radius = max(2, size // 150)
    terrain = _box_blur(terrain, radius=blur_radius, passes=2)

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

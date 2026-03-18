"""Generate controllable procedural 16-bit heightmaps for playable downhill runs.

This generator is corridor-driven (not noise-driven):
- global downhill slope is explicit,
- valleys are guided by centerlines aligned with slope,
- optional secondary corridors create lane choices,
- a slope limiter removes chaotic bumps and micro-spikes.

Usage:
    python generate_heightmap.py --output out.png --size 2049 --seed 42 --profile flow
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
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


@dataclass(frozen=True)
class TerrainParams:
    flow_angle_deg: float
    global_slope: float
    corridor_count: int
    corridor_depth: float
    corridor_width: float
    corridor_meander: float
    corridor_frequency: float
    sidebank_strength: float
    macro_variation: float
    low_noise: float
    smoothness: float
    slope_limit: float
    slope_limit_passes: int
    edge_flat_width: float


PROFILE_PRESETS: dict[str, TerrainParams] = {
    "flow": TerrainParams(
        flow_angle_deg=-52.0,
        global_slope=0.48,
        corridor_count=1,
        corridor_depth=0.62,
        corridor_width=0.13,
        corridor_meander=0.035,
        corridor_frequency=0.9,
        sidebank_strength=0.13,
        macro_variation=0.08,
        low_noise=0.012,
        smoothness=0.86,
        slope_limit=0.016,
        slope_limit_passes=3,
        edge_flat_width=0.07,
    ),
    "switchback": TerrainParams(
        flow_angle_deg=-50.0,
        global_slope=0.41,
        corridor_count=1,
        corridor_depth=0.56,
        corridor_width=0.115,
        corridor_meander=0.085,
        corridor_frequency=2.0,
        sidebank_strength=0.16,
        macro_variation=0.09,
        low_noise=0.014,
        smoothness=0.82,
        slope_limit=0.017,
        slope_limit_passes=3,
        edge_flat_width=0.07,
    ),
    "risk_reward": TerrainParams(
        flow_angle_deg=-56.0,
        global_slope=0.52,
        corridor_count=2,
        corridor_depth=0.60,
        corridor_width=0.105,
        corridor_meander=0.06,
        corridor_frequency=1.4,
        sidebank_strength=0.22,
        macro_variation=0.11,
        low_noise=0.016,
        smoothness=0.78,
        slope_limit=0.018,
        slope_limit_passes=4,
        edge_flat_width=0.06,
    ),
}


def _normalize(arr: np.ndarray) -> np.ndarray:
    lo = float(arr.min())
    hi = float(arr.max())
    if hi - lo < 1e-8:
        return np.zeros_like(arr)
    return (arr - lo) / (hi - lo)


def _resolve_params(args: argparse.Namespace) -> TerrainParams:
    base = PROFILE_PRESETS[args.profile]

    def pick(name: str):
        val = getattr(args, name)
        return getattr(base, name) if val is None else val

    return TerrainParams(
        flow_angle_deg=float(pick("flow_angle_deg")),
        global_slope=float(pick("global_slope")),
        corridor_count=max(1, int(pick("corridor_count"))),
        corridor_depth=float(pick("corridor_depth")),
        corridor_width=float(pick("corridor_width")),
        corridor_meander=float(pick("corridor_meander")),
        corridor_frequency=float(pick("corridor_frequency")),
        sidebank_strength=float(pick("sidebank_strength")),
        macro_variation=float(pick("macro_variation")),
        low_noise=float(pick("low_noise")),
        smoothness=float(pick("smoothness")),
        slope_limit=float(pick("slope_limit")),
        slope_limit_passes=max(1, int(pick("slope_limit_passes"))),
        edge_flat_width=float(pick("edge_flat_width")),
    )


def _directional_coords(xx: np.ndarray, yy: np.ndarray, angle_deg: float) -> tuple[np.ndarray, np.ndarray]:
    # Coordinates centered around map center.
    cx = xx - 0.5
    cy = yy - 0.5

    theta = np.deg2rad(angle_deg)
    ux = np.cos(theta)
    uy = np.sin(theta)
    vx = -uy
    vy = ux

    u = (cx * ux) + (cy * uy)
    v = (cx * vx) + (cy * vy)
    return _normalize(u), (_normalize(v) - 0.5)


def _slope_limiter(terrain: np.ndarray, slope_limit: float, passes: int, blur_radius: int) -> np.ndarray:
    out = terrain
    for _ in range(passes):
        gy, gx = np.gradient(out)
        slope = np.sqrt(gx * gx + gy * gy)
        blend = np.clip((slope - slope_limit) / max(1e-6, slope_limit), 0.0, 1.0)
        blurred = _box_blur(out, radius=blur_radius, passes=1)
        out = (out * (1.0 - blend)) + (blurred * blend)
    return out


def build_terrain(size: int, seed: int, params: TerrainParams) -> np.ndarray:
    rng = np.random.default_rng(seed)

    y, x = np.mgrid[0:size, 0:size]
    xx = x / (size - 1)
    yy = y / (size - 1)

    u01, vc = _directional_coords(xx, yy, params.flow_angle_deg)

    # Global one-way slope: high near u=0, low near u=1.
    terrain = 0.55 + ((0.5 - u01) * params.global_slope)

    # Build directed corridors that follow the slope direction.
    offsets = np.linspace(-0.14, 0.14, params.corridor_count)
    corridor_field = np.zeros((size, size), dtype=np.float64)
    sidebank_field = np.zeros((size, size), dtype=np.float64)

    for i, lateral_offset in enumerate(offsets):
        phase = float(rng.uniform(0.0, 2.0 * np.pi))
        local_amp = params.corridor_meander * (0.90 + 0.25 * rng.random())
        centerline = lateral_offset + (np.sin((u01 * params.corridor_frequency * 2.0 * np.pi) + phase) * local_amp)

        dist = vc - centerline
        corridor = np.exp(-(dist * dist) / (2.0 * params.corridor_width * params.corridor_width))
        corridor_weight = 1.0 if params.corridor_count == 1 else (0.88 + 0.24 * (i / (params.corridor_count - 1)))
        corridor_field += corridor * corridor_weight

        # Side banks around the corridor for readable risk/reward shaping.
        side_dist = np.abs(dist) - (params.corridor_width * 1.7)
        side_sigma = params.corridor_width * 0.95
        side = np.exp(-(side_dist * side_dist) / (2.0 * side_sigma * side_sigma))
        sidebank_field += side

    corridor_field = _normalize(corridor_field)
    sidebank_field = _normalize(sidebank_field)
    terrain -= corridor_field * params.corridor_depth
    terrain += sidebank_field * params.sidebank_strength

    # Longitudinal segments to avoid perfectly monotonic downhill.
    terrain += np.sin((u01 * 2.0 * np.pi * 2.0) + rng.uniform(0.0, 2.0 * np.pi)) * params.macro_variation

    # Controlled low-frequency variation only (no high-frequency bump chaos).
    macro = (_fbm(size, rng) - 0.5) * params.macro_variation
    low_noise = (_smooth_noise(size, rng, 64) - 0.5) * params.low_noise
    terrain += macro + low_noise

    # Smooth enough for playability while keeping shape identity.
    smooth_passes = 1 + int(round(params.smoothness * 2.0))
    blur_radius = max(2, int(round((size / 170.0) * (0.75 + params.smoothness))))
    terrain = _box_blur(terrain, radius=blur_radius, passes=smooth_passes)

    # Hard anti-chaos step: clamp local slopes by adaptive blending with blurred terrain.
    terrain = _slope_limiter(
        terrain=terrain,
        slope_limit=params.slope_limit,
        passes=params.slope_limit_passes,
        blur_radius=max(2, blur_radius // 2),
    )

    # Edge flattening to reduce unplayable border cliffs.
    terrain = _normalize(terrain)
    edge = np.minimum.reduce([xx, yy, 1.0 - xx, 1.0 - yy])
    edge_mask = np.clip(edge / max(1e-5, params.edge_flat_width), 0.0, 1.0)
    terrain = terrain * edge_mask + 0.04 * (1.0 - edge_mask)

    return np.clip(terrain, 0.0, 1.0)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate a procedural 16-bit terrain heightmap")
    parser.add_argument("--output", required=True, type=Path, help="Output 16-bit png")
    parser.add_argument("--size", type=int, default=2049, help="Heightmap size (recommended: 1009, 2017, 2049)")
    parser.add_argument("--seed", type=int, default=42, help="Random seed for reproducibility")

    parser.add_argument(
        "--profile",
        type=str,
        default="flow",
        choices=sorted(PROFILE_PRESETS.keys()),
        help="Gameplay terrain profile preset",
    )

    # Optional per-run overrides (if omitted, profile defaults are used).
    parser.add_argument("--flow-angle-deg", type=float, default=None, help="Global downhill direction angle in degrees")
    parser.add_argument("--global-slope", type=float, default=None, help="Global slope intensity [0..1]")
    parser.add_argument("--corridor-count", type=int, default=None, help="Number of guided downhill corridors")
    parser.add_argument("--corridor-depth", type=float, default=None, help="Corridor carving strength")
    parser.add_argument("--corridor-width", type=float, default=None, help="Corridor half-width in normalized lateral space")
    parser.add_argument("--corridor-meander", type=float, default=None, help="Corridor meander amplitude")
    parser.add_argument("--corridor-frequency", type=float, default=None, help="Corridor meander frequency along flow")
    parser.add_argument("--sidebank-strength", type=float, default=None, help="Sidebank ridge strength")
    parser.add_argument("--macro-variation", type=float, default=None, help="Large shape variation intensity")
    parser.add_argument("--low-noise", type=float, default=None, help="Very low-frequency micro variation")
    parser.add_argument("--smoothness", type=float, default=None, help="Smoothing intensity [0..1]")
    parser.add_argument("--slope-limit", type=float, default=None, help="Max local slope target after limiter")
    parser.add_argument("--slope-limit-passes", type=int, default=None, help="Slope limiter passes")
    parser.add_argument("--edge-flat-width", type=float, default=None, help="Border flatten width")

    args = parser.parse_args()

    if args.size < 129:
        raise ValueError("--size must be at least 129")

    params = _resolve_params(args)

    if not (0.0 <= params.global_slope <= 1.0):
        raise ValueError("--global-slope must be in [0, 1]")
    if params.corridor_width <= 0.001:
        raise ValueError("--corridor-width must be > 0")
    if params.smoothness < 0.0 or params.smoothness > 1.0:
        raise ValueError("--smoothness must be in [0, 1]")
    if params.slope_limit <= 0.0:
        raise ValueError("--slope-limit must be > 0")

    terrain = build_terrain(args.size, args.seed, params)
    height_u16 = np.clip(terrain * 65535.0, 0, 65535).astype(np.uint16)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(height_u16).save(args.output)
    print(f"Saved procedural heightmap: {args.output}")
    print(
        "Profile/params: "
        f"profile={args.profile}, angle={params.flow_angle_deg:.1f}, slope={params.global_slope:.2f}, "
        f"corridors={params.corridor_count}, depth={params.corridor_depth:.2f}, width={params.corridor_width:.3f}, "
        f"meander={params.corridor_meander:.3f}, freq={params.corridor_frequency:.2f}, "
        f"low_noise={params.low_noise:.3f}, smooth={params.smoothness:.2f}, slope_limit={params.slope_limit:.3f}"
    )


if __name__ == "__main__":
    main()

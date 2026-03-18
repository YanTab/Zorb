"""Generate automatic course markers (start/checkpoints/finish) from a 16-bit heightmap.

Outputs:
1) JSON layout file with world transforms.
2) Unreal Python script that can spawn/move markers in the currently opened level.

Usage:
    python generate_course_markers.py \
      --input Tools/HeightmapProcessing/output/candidates_v1/valley_flow_1009_norm.png \
      --output-json Tools/HeightmapProcessing/output/candidates_v1/valley_flow_course.json \
      --output-unreal-py Tools/HeightmapProcessing/output/candidates_v1/valley_flow_apply_course.py \
      --map-name valley_flow
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image


def _load_height(path: Path) -> np.ndarray:
    image = Image.open(path)
    arr = np.array(image)
    if arr.ndim == 3:
        arr = arr[..., 0]
    if arr.dtype != np.uint16:
        arr = arr.astype(np.float64)
        low = float(arr.min())
        high = float(arr.max())
        if high - low < 1e-8:
            return np.zeros_like(arr, dtype=np.float64)
        return (arr - low) / (high - low)
    return arr.astype(np.float64) / 65535.0


def _box_blur(arr: np.ndarray, radius: int) -> np.ndarray:
    if radius <= 0:
        return arr

    # Horizontal pass
    padded = np.pad(arr, ((0, 0), (radius, radius)), mode="edge")
    csum = np.cumsum(padded, axis=1)
    csum = np.pad(csum, ((0, 0), (1, 0)), mode="constant", constant_values=0.0)
    out = (csum[:, 2 * radius + 1 :] - csum[:, : -(2 * radius + 1)]) / float(2 * radius + 1)

    # Vertical pass
    padded = np.pad(out, ((radius, radius), (0, 0)), mode="edge")
    csum = np.cumsum(padded, axis=0)
    csum = np.pad(csum, ((1, 0), (0, 0)), mode="constant", constant_values=0.0)
    out = (csum[2 * radius + 1 :, :] - csum[: -(2 * radius + 1), :]) / float(2 * radius + 1)
    return out


def _compute_slope(height_norm: np.ndarray) -> np.ndarray:
    gy, gx = np.gradient(height_norm)
    slope = np.sqrt(gx * gx + gy * gy)
    low = float(slope.min())
    high = float(slope.max())
    if high - low < 1e-8:
        return np.zeros_like(slope)
    return (slope - low) / (high - low)


def _pick_anchor_points(height_norm: np.ndarray, slope_norm: np.ndarray) -> tuple[tuple[int, int], tuple[int, int]]:
    h, w = height_norm.shape
    margin_y = max(8, h // 10)
    margin_x = max(8, w // 10)

    yy, xx = np.mgrid[0:h, 0:w]
    inside = (
        (yy >= margin_y)
        & (yy < h - margin_y)
        & (xx >= margin_x)
        & (xx < w - margin_x)
    )

    start_score = (height_norm * 0.85) + ((1.0 - slope_norm) * 0.15)
    finish_score = ((1.0 - height_norm) * 0.90) + ((1.0 - slope_norm) * 0.10)

    start_score = np.where(inside, start_score, -1e9)
    finish_score = np.where(inside, finish_score, -1e9)

    sidx = np.unravel_index(int(np.argmax(start_score)), start_score.shape)
    fidx = np.unravel_index(int(np.argmax(finish_score)), finish_score.shape)

    sy, sx = int(sidx[0]), int(sidx[1])
    fy, fx = int(fidx[0]), int(fidx[1])

    # Ensure non-trivial distance between start and finish.
    if math.hypot(float(fx - sx), float(fy - sy)) < min(h, w) * 0.35:
        # If too close, push finish to farthest low-altitude candidate.
        low_mask = (height_norm <= np.percentile(height_norm, 30.0)) & inside
        if np.any(low_mask):
            cand_y, cand_x = np.where(low_mask)
            distances = np.sqrt((cand_x - sx) ** 2 + (cand_y - sy) ** 2)
            j = int(np.argmax(distances))
            fy, fx = int(cand_y[j]), int(cand_x[j])

    return (sx, sy), (fx, fy)


def _refine_to_low_slope(
    x: float,
    y: float,
    slope_norm: np.ndarray,
    window_radius: int,
) -> tuple[int, int]:
    h, w = slope_norm.shape
    cx = int(round(x))
    cy = int(round(y))
    cx = max(0, min(w - 1, cx))
    cy = max(0, min(h - 1, cy))

    best = (cx, cy)
    best_cost = 1e9
    for iy in range(max(0, cy - window_radius), min(h, cy + window_radius + 1)):
        for ix in range(max(0, cx - window_radius), min(w, cx + window_radius + 1)):
            dist = math.hypot(float(ix - x), float(iy - y))
            cost = float(slope_norm[iy, ix]) + (dist * 0.02)
            if cost < best_cost:
                best_cost = cost
                best = (ix, iy)
    return best


def _build_polyline_pixels(
    start_px: tuple[int, int],
    finish_px: tuple[int, int],
    slope_norm: np.ndarray,
    checkpoint_count: int,
) -> list[tuple[int, int]]:
    sx, sy = start_px
    fx, fy = finish_px
    h, w = slope_norm.shape

    total_points = checkpoint_count + 2
    dx = float(fx - sx)
    dy = float(fy - sy)
    length = math.hypot(dx, dy)
    if length < 1e-6:
        length = 1.0

    # Perpendicular for gentle meander.
    px = -dy / length
    py = dx / length
    amp = min(h, w) * 0.06
    turns = 1.5

    points: list[tuple[int, int]] = []
    for i in range(total_points):
        t = 0.0 if total_points <= 1 else (i / float(total_points - 1))
        base_x = sx + (dx * t)
        base_y = sy + (dy * t)

        wobble = math.sin(t * math.pi * 2.0 * turns) * amp
        target_x = base_x + (px * wobble)
        target_y = base_y + (py * wobble)

        target_x = max(1.0, min((w - 2.0), target_x))
        target_y = max(1.0, min((h - 2.0), target_y))

        ix, iy = _refine_to_low_slope(target_x, target_y, slope_norm, window_radius=8)
        points.append((ix, iy))

    # Force exact anchors at endpoints.
    points[0] = (sx, sy)
    points[-1] = (fx, fy)
    return points


def _height_to_world_z(height_norm: float, z_scale: float) -> float:
    # Unreal Landscape canonical conversion approximation:
    # normalized [0..1] -> height units [-256..256], then multiplied by Z scale.
    return ((height_norm * 512.0) - 256.0) * z_scale


def _pixel_to_world(
    px: int,
    py: int,
    height_norm: np.ndarray,
    xy_scale: float,
    z_scale: float,
    world_offset_x: float,
    world_offset_y: float,
    world_offset_z: float,
    spawn_height_offset: float,
) -> tuple[float, float, float]:
    h, w = height_norm.shape
    cx = (w - 1) * 0.5
    cy = (h - 1) * 0.5

    wx = ((float(px) - cx) * xy_scale) + world_offset_x
    wy = ((float(py) - cy) * xy_scale) + world_offset_y
    wz = _height_to_world_z(float(height_norm[py, px]), z_scale) + world_offset_z + spawn_height_offset
    return wx, wy, wz


def _yaw_from_points(a: tuple[float, float, float], b: tuple[float, float, float]) -> float:
    dx = b[0] - a[0]
    dy = b[1] - a[1]
    if abs(dx) < 1e-6 and abs(dy) < 1e-6:
        return 0.0
    return math.degrees(math.atan2(dy, dx))


def _build_layout(
    map_name: str,
    input_path: Path,
    height_norm: np.ndarray,
    points_px: list[tuple[int, int]],
    xy_scale: float,
    z_scale: float,
    world_offset_x: float,
    world_offset_y: float,
    world_offset_z: float,
    spawn_height_offset: float,
) -> dict:
    world_points = [
        _pixel_to_world(
            px=pt[0],
            py=pt[1],
            height_norm=height_norm,
            xy_scale=xy_scale,
            z_scale=z_scale,
            world_offset_x=world_offset_x,
            world_offset_y=world_offset_y,
            world_offset_z=world_offset_z,
            spawn_height_offset=spawn_height_offset,
        )
        for pt in points_px
    ]

    start_loc = world_points[0]
    finish_loc = world_points[-1]
    start_yaw = _yaw_from_points(world_points[0], world_points[1]) if len(world_points) > 1 else 0.0
    finish_yaw = _yaw_from_points(world_points[-2], world_points[-1]) if len(world_points) > 1 else 0.0

    checkpoints = []
    for i in range(1, len(world_points) - 1):
        loc = world_points[i]
        prev_loc = world_points[max(0, i - 1)]
        next_loc = world_points[min(len(world_points) - 1, i + 1)]
        yaw = _yaw_from_points(prev_loc, next_loc)
        checkpoints.append(
            {
                "index": i,
                "location": {"x": loc[0], "y": loc[1], "z": loc[2]},
                "rotation": {"pitch": 0.0, "yaw": yaw, "roll": 0.0},
            }
        )

    return {
        "version": "course-layout-v1",
        "map_name": map_name,
        "source_heightmap": str(input_path).replace("\\", "/"),
        "import": {
            "xy_scale": xy_scale,
            "z_scale": z_scale,
            "world_offset": {
                "x": world_offset_x,
                "y": world_offset_y,
                "z": world_offset_z,
            },
            "spawn_height_offset": spawn_height_offset,
        },
        "start": {
            "location": {"x": start_loc[0], "y": start_loc[1], "z": start_loc[2]},
            "rotation": {"pitch": 0.0, "yaw": start_yaw, "roll": 0.0},
        },
        "checkpoints": checkpoints,
        "finish": {
            "location": {"x": finish_loc[0], "y": finish_loc[1], "z": finish_loc[2]},
            "rotation": {"pitch": 0.0, "yaw": finish_yaw, "roll": 0.0},
        },
    }


def _build_unreal_script(layout_json_path: Path) -> str:
    # Keep script standalone so user can run it from Unreal Python console.
    json_path = str(layout_json_path.resolve()).replace("\\", "/")

    return f'''import json
import unreal

LAYOUT_JSON = r"{json_path}"

CHECKPOINT_BP = "/Game/Blueprints/BP_CheckPointTrigger.BP_CheckPointTrigger_C"
FINISH_BP = "/Game/Blueprints/BP_FinishTrigger.BP_FinishTrigger_C"


def _vec(d):
    return unreal.Vector(float(d["x"]), float(d["y"]), float(d["z"]))


def _rot(d):
    return unreal.Rotator(float(d["pitch"]), float(d["yaw"]), float(d["roll"]))


def _load_class(path):
    c = unreal.load_class(None, path)
    if c is None:
        raise RuntimeError(f"Cannot load class: {{path}}")
    return c


def _move_or_spawn_player_start(start_loc, start_rot):
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if isinstance(actor, unreal.PlayerStart):
            actor.set_actor_location(start_loc, False, False)
            actor.set_actor_rotation(start_rot, False)
            actor.set_actor_label("AUTO_PlayerStart")
            return actor

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, start_loc, start_rot)
    actor.set_actor_label("AUTO_PlayerStart")
    return actor


def _delete_previous_auto_markers():
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    to_delete = []
    for actor in actors:
        label = actor.get_actor_label()
        if label.startswith("AUTO_CP_") or label == "AUTO_Finish":
            to_delete.append(actor)
    for actor in to_delete:
        unreal.EditorLevelLibrary.destroy_actor(actor)


def main():
    with open(LAYOUT_JSON, "r", encoding="utf-8") as f:
        data = json.load(f)

    cp_class = _load_class(CHECKPOINT_BP)
    finish_class = _load_class(FINISH_BP)

    _delete_previous_auto_markers()

    start_loc = _vec(data["start"]["location"])
    start_rot = _rot(data["start"]["rotation"])
    _move_or_spawn_player_start(start_loc, start_rot)

    for cp in data["checkpoints"]:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            cp_class,
            _vec(cp["location"]),
            _rot(cp["rotation"]),
        )
        actor.set_actor_label(f"AUTO_CP_{{int(cp['index']):02d}}")

    finish = data["finish"]
    finish_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        finish_class,
        _vec(finish["location"]),
        _rot(finish["rotation"]),
    )
    finish_actor.set_actor_label("AUTO_Finish")

    unreal.log(f"Auto course generated for map: {{data.get('map_name', 'unknown')}}")
    unreal.log(f"Checkpoints count: {{len(data.get('checkpoints', []))}}")


main()
'''


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate automatic start/checkpoints/finish from heightmap")
    parser.add_argument("--input", required=True, type=Path, help="Input 16-bit normalized heightmap PNG")
    parser.add_argument("--output-json", required=True, type=Path, help="Output layout json")
    parser.add_argument("--output-unreal-py", required=True, type=Path, help="Output Unreal placement python script")
    parser.add_argument("--map-name", required=True, type=str, help="Logical map name")
    parser.add_argument("--checkpoint-count", type=int, default=5, help="Number of checkpoints between start/finish")
    parser.add_argument("--xy-scale", type=float, default=100.0, help="Landscape XY scale used at import")
    parser.add_argument("--z-scale", type=float, default=50.0, help="Landscape Z scale used at import")
    parser.add_argument("--world-offset-x", type=float, default=0.0, help="Landscape actor world offset X")
    parser.add_argument("--world-offset-y", type=float, default=0.0, help="Landscape actor world offset Y")
    parser.add_argument("--world-offset-z", type=float, default=0.0, help="Landscape actor world offset Z")
    parser.add_argument("--spawn-height-offset", type=float, default=120.0, help="Extra Z offset for markers")
    args = parser.parse_args()

    if args.checkpoint_count < 1:
        raise ValueError("--checkpoint-count must be >= 1")

    height_norm = _load_height(args.input)
    height_norm = _box_blur(height_norm, radius=max(1, min(height_norm.shape) // 250))
    slope_norm = _compute_slope(height_norm)

    start_px, finish_px = _pick_anchor_points(height_norm, slope_norm)
    points_px = _build_polyline_pixels(start_px, finish_px, slope_norm, checkpoint_count=args.checkpoint_count)

    layout = _build_layout(
        map_name=args.map_name,
        input_path=args.input,
        height_norm=height_norm,
        points_px=points_px,
        xy_scale=args.xy_scale,
        z_scale=args.z_scale,
        world_offset_x=args.world_offset_x,
        world_offset_y=args.world_offset_y,
        world_offset_z=args.world_offset_z,
        spawn_height_offset=args.spawn_height_offset,
    )

    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(layout, indent=2), encoding="utf-8")

    script_text = _build_unreal_script(args.output_json)
    args.output_unreal_py.parent.mkdir(parents=True, exist_ok=True)
    args.output_unreal_py.write_text(script_text, encoding="utf-8")

    print(f"Saved course layout json: {args.output_json}")
    print(f"Saved Unreal placement script: {args.output_unreal_py}")
    print(f"Start pixel: {start_px}")
    print(f"Finish pixel: {finish_px}")


if __name__ == "__main__":
    main()
from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path


def load_rows(path: Path) -> list[dict[str, float]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        rows = []
        for row in reader:
            parsed: dict[str, float] = {}
            for key, value in row.items():
                parsed[key] = 0.0 if value in (None, "") else float(value)
            rows.append(parsed)
    return rows


def summarize_air_segment(segment_rows: list[dict[str, float]]) -> dict[str, float]:
    if len(segment_rows) < 2:
        return {}
    first = segment_rows[0]
    last = segment_rows[-1]
    start_speed = first["planar_speed"]
    end_speed = last["planar_speed"]
    loss = max(0.0, start_speed - end_speed)
    loss_pct = (loss / start_speed * 100.0) if start_speed > 1e-3 else 0.0
    return {
        "start_time": first["time_seconds"],
        "end_time": last["time_seconds"],
        "duration": last["time_seconds"] - first["time_seconds"],
        "start_planar_speed": start_speed,
        "end_planar_speed": end_speed,
        "planar_speed_loss": loss,
        "planar_speed_loss_pct": loss_pct,
        "min_vertical_speed": min(row["vertical_speed"] for row in segment_rows),
        "max_vertical_speed": max(row["vertical_speed"] for row in segment_rows),
    }


def compute_air_segments(rows: list[dict[str, float]]) -> list[dict[str, float]]:
    segments: list[dict[str, float]] = []
    start = None
    for index, row in enumerate(rows):
        grounded = row["grounded"] >= 0.5
        if not grounded and start is None:
            start = index
        elif grounded and start is not None:
            segments.append(summarize_air_segment(rows[start:index]))
            start = None
    if start is not None:
        segments.append(summarize_air_segment(rows[start:]))
    return [segment for segment in segments if segment]


def summarize_brake_segment(segment_rows: list[dict[str, float]]) -> dict[str, float]:
    if len(segment_rows) < 2:
        return {}
    first = segment_rows[0]
    last = segment_rows[-1]
    distance = 0.0
    for previous, current in zip(segment_rows, segment_rows[1:]):
        dx = current["pos_x"] - previous["pos_x"]
        dy = current["pos_y"] - previous["pos_y"]
        dz = current["pos_z"] - previous["pos_z"]
        distance += math.sqrt((dx * dx) + (dy * dy) + (dz * dz))
    return {
        "start_time": first["time_seconds"],
        "end_time": last["time_seconds"],
        "duration": last["time_seconds"] - first["time_seconds"],
        "start_planar_speed": first["planar_speed"],
        "end_planar_speed": last["planar_speed"],
        "distance": distance,
    }


def compute_brake_segments(rows: list[dict[str, float]]) -> list[dict[str, float]]:
    segments: list[dict[str, float]] = []
    start = None
    for index, row in enumerate(rows):
        braking = row["grounded"] >= 0.5 and row["forward_input"] < -0.5
        if braking and start is None:
            start = index
        elif not braking and start is not None:
            segments.append(summarize_brake_segment(rows[start:index]))
            start = None
    if start is not None:
        segments.append(summarize_brake_segment(rows[start:]))
    return [segment for segment in segments if segment]


def summarize_turn_segment(segment_rows: list[dict[str, float]]) -> dict[str, float]:
    if len(segment_rows) < 2:
        return {}
    first = segment_rows[0]
    last = segment_rows[-1]
    min_speed = min(row["planar_speed"] for row in segment_rows)
    max_speed = max(row["planar_speed"] for row in segment_rows)
    return {
        "start_time": first["time_seconds"],
        "end_time": last["time_seconds"],
        "duration": last["time_seconds"] - first["time_seconds"],
        "start_planar_speed": first["planar_speed"],
        "end_planar_speed": last["planar_speed"],
        "min_planar_speed": min_speed,
        "max_planar_speed": max_speed,
        "speed_loss": max(0.0, first["planar_speed"] - min_speed),
    }


def compute_turn_segments(rows: list[dict[str, float]]) -> list[dict[str, float]]:
    segments: list[dict[str, float]] = []
    start = None
    for index, row in enumerate(rows):
        turning = row["grounded"] >= 0.5 and abs(row["right_input"]) > 0.5
        if turning and start is None:
            start = index
        elif not turning and start is not None:
            segments.append(summarize_turn_segment(rows[start:index]))
            start = None
    if start is not None:
        segments.append(summarize_turn_segment(rows[start:]))
    return [segment for segment in segments if segment]


def build_summary(rows: list[dict[str, float]]) -> dict:
    if not rows:
        return {"error": "No telemetry rows found."}

    first = rows[0]
    last = rows[-1]
    air_segments = compute_air_segments(rows)
    brake_segments = compute_brake_segments(rows)
    turn_segments = compute_turn_segments(rows)

    return {
        "row_count": len(rows),
        "duration": last["time_seconds"] - first["time_seconds"],
        "max_planar_speed": max(row["planar_speed"] for row in rows),
        "avg_planar_speed": sum(row["planar_speed"] for row in rows) / len(rows),
        "max_vertical_speed": max(row["vertical_speed"] for row in rows),
        "min_vertical_speed": min(row["vertical_speed"] for row in rows),
        "airborne_time": sum(
            max(0.0, current["time_seconds"] - previous["time_seconds"])
            for previous, current in zip(rows, rows[1:])
            if previous["grounded"] < 0.5
        ),
        "tuning_snapshot": {
            "max_speed": first["max_speed"],
            "turn_rate_scale": first["turn_rate_scale"],
            "brake_deceleration": first["brake_deceleration"],
            "air_horizontal_deceleration": first["air_horizontal_deceleration"],
        },
        "air_segments": air_segments,
        "brake_segments": brake_segments,
        "turn_segments": turn_segments,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Analyze Zorb telemetry CSV exports.")
    parser.add_argument("input", type=Path, help="Path to a telemetry CSV file.")
    parser.add_argument("--json", action="store_true", help="Print the summary as JSON.")
    args = parser.parse_args()

    rows = load_rows(args.input)
    summary = build_summary(rows)

    if args.json:
        print(json.dumps(summary, indent=2))
        return

    print(f"Rows: {summary['row_count']}")
    print(f"Duration: {summary['duration']:.3f}s")
    print(f"Max planar speed: {summary['max_planar_speed']:.2f}")
    print(f"Avg planar speed: {summary['avg_planar_speed']:.2f}")
    print(f"Airborne time: {summary['airborne_time']:.3f}s")
    print(f"Air segments: {len(summary['air_segments'])}")
    print(f"Brake segments: {len(summary['brake_segments'])}")
    print(f"Turn segments: {len(summary['turn_segments'])}")

    if summary["air_segments"]:
        worst_air = max(summary["air_segments"], key=lambda item: item["planar_speed_loss_pct"])
        print(f"Worst air speed loss: {worst_air['planar_speed_loss_pct']:.2f}% over {worst_air['duration']:.3f}s")


if __name__ == "__main__":
    main()
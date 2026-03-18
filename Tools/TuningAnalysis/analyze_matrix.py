from __future__ import annotations

import argparse
import re
from pathlib import Path

from analyze_telemetry import build_summary, load_rows


FILE_PATTERN = re.compile(
    r"^zorb_(classic|agile|heavy|wild)_(.+)_(\d{8}_\d{6})\.csv$",
    re.IGNORECASE,
)


def score_row(summary: dict) -> float:
    # Higher is better: speed/stability rewarded, airborne time penalized slightly.
    return (
        (summary["avg_planar_speed"] * 0.6)
        + (summary["max_planar_speed"] * 0.2)
        + (max(0.0, 25.0 - summary.get("worst_air_loss_pct", 25.0)) * 20.0)
        - (summary["airborne_time"] * 15.0)
    )


def find_runs(folder: Path) -> list[tuple[Path, str, str]]:
    runs: list[tuple[Path, str, str]] = []
    for path in sorted(folder.glob("*.csv")):
        match = FILE_PATTERN.match(path.name)
        if not match:
            continue
        preset = match.group(1).lower()
        scenario = match.group(2).lower()
        runs.append((path, preset, scenario))
    return runs


def enrich_summary(summary: dict) -> dict:
    worst_air_loss_pct = 0.0
    if summary["air_segments"]:
        worst_air = max(summary["air_segments"], key=lambda item: item["planar_speed_loss_pct"])
        worst_air_loss_pct = worst_air["planar_speed_loss_pct"]

    return {
        "duration": summary["duration"],
        "avg_planar_speed": summary["avg_planar_speed"],
        "max_planar_speed": summary["max_planar_speed"],
        "airborne_time": summary["airborne_time"],
        "worst_air_loss_pct": worst_air_loss_pct,
        "score": 0.0,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Analyze preset/scenario telemetry matrix.")
    parser.add_argument(
        "input_dir",
        type=Path,
        nargs="?",
        default=Path("Saved/Telemetry"),
        help="Telemetry folder (default: Saved/Telemetry).",
    )
    args = parser.parse_args()

    runs = find_runs(args.input_dir)
    if not runs:
        print("No matrix files found (expected zorb_{preset}_{scenario}_YYYYMMDD_HHMMSS.csv).")
        return

    results: dict[str, dict[str, dict]] = {}
    for path, preset, scenario in runs:
        rows = load_rows(path)
        summary = build_summary(rows)
        if "error" in summary:
            continue

        metrics = enrich_summary(summary)
        metrics["score"] = score_row(metrics)

        if scenario not in results:
            results[scenario] = {}
        previous = results[scenario].get(preset)
        if previous is None or metrics["score"] > previous["score"]:
            results[scenario][preset] = metrics

    for scenario in sorted(results.keys()):
        print("=" * 72)
        print(f"SCENARIO: {scenario}")
        print("=" * 72)
        print("preset   | duration | avg_speed | max_speed | air_time | worst_air_loss | score")

        ordered = sorted(results[scenario].items(), key=lambda kv: kv[1]["score"], reverse=True)
        for preset, metrics in ordered:
            print(
                f"{preset:<8} | "
                f"{metrics['duration']:>7.3f}s | "
                f"{metrics['avg_planar_speed']:>8.2f} | "
                f"{metrics['max_planar_speed']:>8.2f} | "
                f"{metrics['airborne_time']:>7.3f}s | "
                f"{metrics['worst_air_loss_pct']:>12.2f}% | "
                f"{metrics['score']:>7.2f}"
            )

        winner = ordered[0][0]
        print(f"Winner: {winner}")
        print()


if __name__ == "__main__":
    main()

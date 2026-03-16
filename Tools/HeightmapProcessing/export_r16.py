"""Export a 16-bit grayscale heightmap PNG to Unreal-compatible R16 raw file.

Usage:
    python export_r16.py --input in.png --output out.r16
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert 16-bit heightmap image to .r16 raw")
    parser.add_argument("--input", required=True, type=Path, help="Input heightmap PNG/TIF")
    parser.add_argument("--output", required=True, type=Path, help="Output raw file (.r16)")
    args = parser.parse_args()

    img = Image.open(args.input)
    arr = np.array(img)

    if arr.ndim == 3:
        arr = arr[..., 0]

    if arr.dtype != np.uint16:
        arr = arr.astype(np.float64)
        min_v = float(arr.min())
        max_v = float(arr.max())
        if max_v - min_v < 1e-8:
            arr = np.zeros_like(arr, dtype=np.uint16)
        else:
            arr = ((arr - min_v) / (max_v - min_v) * 65535.0).clip(0, 65535).astype(np.uint16)

    # Unreal expects little-endian unsigned 16-bit values in row-major order.
    arr_le = arr.astype("<u2", copy=False)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with open(args.output, "wb") as f:
        f.write(arr_le.tobytes(order="C"))

    print(f"Saved R16 raw: {args.output}")
    print(f"Resolution: {arr.shape[1]}x{arr.shape[0]}")


if __name__ == "__main__":
    main()

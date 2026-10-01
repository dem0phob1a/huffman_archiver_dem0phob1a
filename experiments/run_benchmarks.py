#!/usr/bin/env python3
"""Run repeatable Huffman CLI benchmarks and create CSV summaries and plots."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import platform
import statistics
import subprocess
import sys
import tempfile
import time
import zipfile
from pathlib import Path
from typing import Any


TEXT_DATASETS = (
    ("text", "text_1kb", Path("text/text_1kb.txt")),
    ("text", "text_100kb", Path("text/text_100kb.txt")),
    ("text", "text_1mb", Path("text/text_1mb.txt")),
    ("text", "text_full", Path("text/text_full.txt")),
)
RANDOM_DATASETS = (
    ("random", "random_1kb", Path("random/random_1kb.bin")),
    ("random", "random_100kb", Path("random/random_100kb.bin")),
    ("random", "random_1mb", Path("random/random_1mb.bin")),
)

RAW_FIELDS = (
    "type",
    "dataset",
    "run",
    "input_bytes",
    "archive_bytes",
    "compression_ratio_percent",
    "compression_ms",
    "decompression_ms",
)
SUMMARY_FIELDS = (
    "type",
    "dataset",
    "input_bytes",
    "archive_bytes",
    "compression_ratio_percent",
    "space_saving_percent",
    "compression_mean_ms",
    "compression_stddev_ms",
    "decompression_mean_ms",
    "decompression_stddev_ms",
    "runs",
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def run_command(executable: Path, mode: str, source: Path, destination: Path) -> float:
    started = time.perf_counter_ns()
    completed = subprocess.run(
        [str(executable), mode, str(source), str(destination)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        check=False,
    )
    elapsed_ms = (time.perf_counter_ns() - started) / 1_000_000
    if completed.returncode != 0:
        error = completed.stderr.decode("utf-8", errors="replace").strip()
        raise RuntimeError(f"{mode} failed for {source}: {error}")
    return elapsed_ms


def discover_datasets(
    data_root: Path, project_root: Path, temp_root: Path
) -> list[tuple[str, str, Path]]:
    datasets = [
        (data_type, name, data_root / relative_path)
        for data_type, name, relative_path in (*TEXT_DATASETS, *RANDOM_DATASETS)
    ]
    datasets.append(
        ("source_code", "source_code", data_root / "source/source_code.txt")
    )

    source_files = sorted(
        path
        for path in (project_root / "src").iterdir()
        if path.is_file() and path.suffix in {".h", ".cpp"}
    )
    datasets.extend(
        ("source_code", f"source_{path.stem}_{path.suffix[1:]}", path)
        for path in source_files
    )

    full_archive = data_root / "precompressed/text_full.zip"
    datasets.append(("precompressed", "precompressed_full", full_archive))
    for data_type, name, relative_path in TEXT_DATASETS[:-1]:
        source = data_root / relative_path
        archive = temp_root / f"{name}.zip"
        with zipfile.ZipFile(
            archive, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9
        ) as zipped:
            info = zipfile.ZipInfo(relative_path.name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            zipped.writestr(info, source.read_bytes(), compresslevel=9)
        datasets.append(("precompressed", f"precompressed_{name}", archive))

    for _, name, path in datasets:
        if not path.is_file():
            raise FileNotFoundError(f"Missing benchmark input for {name}: {path}")
    return datasets


def benchmark_dataset(
    executable: Path,
    temp_root: Path,
    data_type: str,
    name: str,
    source: Path,
    runs: int,
    warmups: int,
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    archive = temp_root / f"{name}.huf"
    restored = temp_root / f"{name}.restored"
    for _ in range(warmups):
        run_command(executable, "-c", source, archive)
        run_command(executable, "-d", archive, restored)

    input_bytes = source.stat().st_size
    source_hash = sha256(source)
    raw_rows: list[dict[str, Any]] = []
    archive_sizes: set[int] = set()
    compression_times: list[float] = []
    decompression_times: list[float] = []

    for run_number in range(1, runs + 1):
        compression_ms = run_command(executable, "-c", source, archive)
        archive_bytes = archive.stat().st_size
        decompression_ms = run_command(executable, "-d", archive, restored)
        if sha256(restored) != source_hash:
            raise RuntimeError(f"Round-trip hash mismatch for {name}, run {run_number}")

        ratio = 100.0 * archive_bytes / input_bytes if input_bytes else 0.0
        raw_rows.append(
            {
                "type": data_type,
                "dataset": name,
                "run": run_number,
                "input_bytes": input_bytes,
                "archive_bytes": archive_bytes,
                "compression_ratio_percent": ratio,
                "compression_ms": compression_ms,
                "decompression_ms": decompression_ms,
            }
        )
        archive_sizes.add(archive_bytes)
        compression_times.append(compression_ms)
        decompression_times.append(decompression_ms)

    if len(archive_sizes) != 1:
        raise RuntimeError(f"Archive size varied between runs for {name}")

    archive_bytes = archive_sizes.pop()
    ratio = 100.0 * archive_bytes / input_bytes if input_bytes else 0.0
    summary = {
        "type": data_type,
        "dataset": name,
        "input_bytes": input_bytes,
        "archive_bytes": archive_bytes,
        "compression_ratio_percent": ratio,
        "space_saving_percent": 100.0 - ratio,
        "compression_mean_ms": statistics.mean(compression_times),
        "compression_stddev_ms": statistics.stdev(compression_times),
        "decompression_mean_ms": statistics.mean(decompression_times),
        "decompression_stddev_ms": statistics.stdev(decompression_times),
        "runs": runs,
    }
    return raw_rows, summary


def write_csv(path: Path, fields: tuple[str, ...], rows: list[dict[str, Any]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def create_plots(output_dir: Path, summaries: list[dict[str, Any]]) -> None:
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError as error:
        raise RuntimeError(
            "Matplotlib is required to create plots. Install it with "
            "'python -m pip install -r experiments/requirements.txt'."
        ) from error

    charts = (
        ("compression_ratio_percent", "Archive size / input size (%)", "compression_ratio.png"),
        ("compression_mean_ms", "Mean compression time (ms)", "compression_time.png"),
        ("decompression_mean_ms", "Mean decompression time (ms)", "decompression_time.png"),
    )
    for metric, y_label, filename in charts:
        figure, axes = plt.subplots(figsize=(9, 5.5))
        data_types = sorted({row["type"] for row in summaries})
        for data_type in data_types:
            rows = sorted(
                (row for row in summaries if row["type"] == data_type),
                key=lambda row: row["input_bytes"],
            )
            axes.plot(
                [row["input_bytes"] for row in rows],
                [row[metric] for row in rows],
                marker="o",
                linewidth=2,
                label=data_type.replace("_", " "),
            )
        axes.set_xscale("log")
        axes.set_xlabel("Input size (bytes)")
        axes.set_ylabel(y_label)
        axes.grid(True, which="both", alpha=0.25)
        axes.legend()
        figure.tight_layout()
        figure.savefig(output_dir / filename, dpi=160)
        plt.close(figure)


def parse_args() -> argparse.Namespace:
    project_root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True, help="Path to huffman_archiver")
    parser.add_argument(
        "--data-dir", type=Path, default=project_root / "experiments" / "data"
    )
    parser.add_argument(
        "--output-dir", type=Path, default=project_root / "experiments" / "results"
    )
    parser.add_argument("--runs", type=int, default=30, help="Measured runs per dataset")
    parser.add_argument("--warmups", type=int, default=1, help="Unrecorded warmup runs")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    executable = args.exe.resolve()
    data_root = args.data_dir.resolve()
    output_dir = args.output_dir.resolve()
    if not executable.is_file():
        raise FileNotFoundError(f"Archiver executable not found: {executable}")
    if args.runs < 2:
        raise ValueError("--runs must be at least 2 to calculate standard deviation")
    if args.warmups < 0:
        raise ValueError("--warmups cannot be negative")

    output_dir.mkdir(parents=True, exist_ok=True)
    raw_rows: list[dict[str, Any]] = []
    summaries: list[dict[str, Any]] = []
    project_root = Path(__file__).resolve().parent.parent
    with tempfile.TemporaryDirectory(prefix="huffman-bench-") as temporary:
        temp_root = Path(temporary)
        datasets = discover_datasets(data_root, project_root, temp_root)
        for data_type, name, source in datasets:
            rows, summary = benchmark_dataset(
                executable,
                temp_root,
                data_type,
                name,
                source,
                args.runs,
                args.warmups,
            )
            raw_rows.extend(rows)
            summaries.append(summary)
            print(
                f"{name}: {summary['input_bytes']} -> {summary['archive_bytes']} bytes; "
                f"compress {summary['compression_mean_ms']:.3f} +/- "
                f"{summary['compression_stddev_ms']:.3f} ms, decompress "
                f"{summary['decompression_mean_ms']:.3f} +/- "
                f"{summary['decompression_stddev_ms']:.3f} ms"
            )

    write_csv(output_dir / "benchmark_runs.csv", RAW_FIELDS, raw_rows)
    write_csv(output_dir / "benchmark_summary.csv", SUMMARY_FIELDS, summaries)
    metadata = {
        "executable": str(executable),
        "data_directory": str(data_root),
        "runs_per_dataset": args.runs,
        "dataset_count": len(summaries),
        "warmup_runs_per_dataset": args.warmups,
        "timer": "perf_counter_ns; includes process startup and file I/O",
        "python": sys.version,
        "platform": platform.platform(),
        "processor": platform.processor(),
    }
    (output_dir / "benchmark_metadata.json").write_text(
        json.dumps(metadata, indent=2) + "\n", encoding="utf-8"
    )
    create_plots(output_dir, summaries)
    print(f"Results written to {output_dir}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, RuntimeError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        raise SystemExit(1) from error

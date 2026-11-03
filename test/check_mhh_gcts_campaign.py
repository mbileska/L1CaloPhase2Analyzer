#!/usr/bin/env python3

import argparse
import csv
import pathlib
import sys


def read_key_values(path):
    values = {}
    for line in path.read_text().splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            values[key] = value
    return values


def main():
    parser = argparse.ArgumentParser(
        description="Check that every MHH/GCTS campaign shard completed and write a CSV manifest"
    )
    parser.add_argument("campaign_dir")
    args = parser.parse_args()

    campaign = pathlib.Path(args.campaign_dir).resolve()
    jobs_path = campaign / "jobs.txt"
    if not jobs_path.is_file():
        raise RuntimeError(f"missing {jobs_path}")

    rows = []
    failures = []
    for line in jobs_path.read_text().splitlines():
        if not line.strip():
            continue
        index_text, input_lfn = line.split(maxsplit=1)
        index = int(index_text)
        shard = f"{index:04d}"
        success = campaign / f"SUCCESS_{shard}"
        manifest_path = campaign / f"manifest_{shard}.txt"
        edm = campaign / f"mhh_gcts_{shard}_edm.root"
        training = campaign / f"mhh_gcts_{shard}_training.root"
        edm_validation = campaign / f"edm_validation_{shard}.txt"
        ntuple_validation = campaign / f"ntuple_validation_{shard}.txt"
        checksums = campaign / f"SHA256SUMS_{shard}"
        stderr_path = campaign / "logs" / f"job_{index}.err"

        required = (
            success,
            manifest_path,
            edm,
            training,
            edm_validation,
            ntuple_validation,
            checksums,
        )
        missing = [str(path.name) for path in required if not path.is_file()]
        if missing:
            failures.append(f"shard {shard}: missing {', '.join(missing)}")
            continue
        if edm.stat().st_size == 0 or training.stat().st_size == 0:
            failures.append(f"shard {shard}: empty ROOT output")
            continue
        if "PASS:" not in edm_validation.read_text():
            failures.append(f"shard {shard}: EDM validation has no PASS result")
        if "PASS:" not in ntuple_validation.read_text():
            failures.append(f"shard {shard}: ntuple validation has no PASS result")
        if stderr_path.is_file() and stderr_path.stat().st_size:
            failures.append(f"shard {shard}: nonempty {stderr_path.name}")

        manifest = read_key_values(manifest_path)
        if manifest.get("status") != "SUCCESS":
            failures.append(f"shard {shard}: manifest status is not SUCCESS")
        if manifest.get("input_lfn") != input_lfn:
            failures.append(f"shard {shard}: input LFN does not match jobs.txt")
        rows.append(
            {
                "job_index": index,
                "input_lfn": input_lfn,
                "event_count": manifest.get("event_count", "UNKNOWN"),
                "output_edm": edm.name,
                "edm_bytes": edm.stat().st_size,
                "output_training": training.name,
                "training_bytes": training.stat().st_size,
                "gctb_present": manifest.get("gctb_present", "UNKNOWN"),
                "status": manifest.get("status", "UNKNOWN"),
            }
        )

    output_csv = campaign / "campaign_manifest.csv"
    fieldnames = (
        "job_index",
        "input_lfn",
        "event_count",
        "output_edm",
        "edm_bytes",
        "output_training",
        "training_bytes",
        "gctb_present",
        "status",
    )
    with output_csv.open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        print(f"FAIL: {len(failures)} campaign checks failed", file=sys.stderr)
        return 1

    total_events = sum(int(row["event_count"]) for row in rows)
    total_edm = sum(int(row["edm_bytes"]) for row in rows)
    total_training = sum(int(row["training_bytes"]) for row in rows)
    print(f"PASS: all {len(rows)} campaign shards completed")
    print(f"PASS: {total_events} synchronized EDM/training events")
    print(f"EDM bytes: {total_edm}; training bytes: {total_training}")
    print(f"Manifest: {output_csv}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)

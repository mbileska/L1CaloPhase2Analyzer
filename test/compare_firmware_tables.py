#!/usr/bin/env python3
import sys


def read_table(path, links):
    rows = []
    with open(path, encoding="ascii") as handle:
        for line in handle:
            fields = line.split()
            if not fields or fields[0].startswith("#"):
                continue
            try:
                int(fields[0], 16)
            except ValueError:
                continue
            if len(fields) < links + 1:
                raise ValueError(f"{path}: short data row: {line.rstrip()}")
            rows.append([int(word, 16) for word in fields[1 : links + 1]])
    return rows


def main():
    if len(sys.argv) not in (3, 4):
        raise SystemExit("Usage: compare_firmware_tables.py CMSSW_OUTPUT FIRMWARE_OUTPUT [LABEL]")
    label = sys.argv[3] if len(sys.argv) == 4 else "MHH"
    cmssw = read_table(sys.argv[1], 6)
    firmware = read_table(sys.argv[2], 6)
    if len(cmssw) != len(firmware):
        raise SystemExit(f"FAILED: row count differs: CMSSW={len(cmssw)}, firmware={len(firmware)}")
    mismatches = []
    for row, (cmssw_words, firmware_words) in enumerate(zip(cmssw, firmware)):
        for link, (cmssw_word, firmware_word) in enumerate(zip(cmssw_words, firmware_words)):
            if cmssw_word != firmware_word:
                mismatches.append((row, link, cmssw_word, firmware_word))
                if len(mismatches) == 20:
                    break
        if len(mismatches) == 20:
            break
    if mismatches:
        for row, link, cmssw_word, firmware_word in mismatches:
            print(f"row={row} link={link} CMSSW={cmssw_word:016x} firmware={firmware_word:016x}")
        raise SystemExit(f"FAILED: {label} CMSSW/firmware mismatch")
    print(f"{label} CMSSW/firmware comparison: {len(cmssw) // 9} events, all 6 links match")


if __name__ == "__main__":
    main()

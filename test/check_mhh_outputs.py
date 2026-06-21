#!/usr/bin/env python3
import sys

import awkward as ak
import uproot


ROOT_FILE = sys.argv[1] if len(sys.argv) > 1 else "mhh_testvectors.root"
TREE_NAME = "l1TMHHAnalyzer/mhhTree"


def values(array, event):
    return ak.to_list(array[event])


def fail(message):
    print(f"FAILED: {message}")
    raise SystemExit(1)


def main():
    tree = uproot.open(ROOT_FILE)[TREE_NAME]
    required = (
        ["patternId", "egEt", "egiEt", "jetEt", "tauEt", "sumEx", "sumEy", "sumHt", "sumEt", "sumNObj"]
        + [f"linkIn{i}_words" for i in range(16)]
        + [f"linkOut{i}_words" for i in range(6)]
    )
    missing = [name for name in required if name not in tree.keys()]
    if missing:
        fail(f"missing ROOT branches: {missing}")

    arrays = tree.arrays(required, library="ak")
    if len(arrays["patternId"]) != 16:
        fail(f"expected 16 events, got {len(arrays['patternId'])}")

    for event in range(16):
        for link in range(16):
            if len(values(arrays[f"linkIn{link}_words"], event)) != 9:
                fail(f"event {event + 1} input link {link} does not contain 9 words")
        for link in range(6):
            if len(values(arrays[f"linkOut{link}_words"], event)) != 9:
                fail(f"event {event + 1} output link {link} does not contain 9 words")
        for link in range(3, 6):
            if any(values(arrays[f"linkOut{link}_words"], event)):
                fail(f"event {event + 1} reserved output link {link} is nonzero")

    def object_ets(branch, event):
        return [value for value in values(arrays[branch], event) if value]

    if any(any(values(arrays[f"linkOut{link}_words"], 0)) for link in range(6)):
        fail("all-zero pattern produced nonzero output")
    if 40 not in object_ets("egEt", 1):
        fail("HF gamma pattern did not propagate ET=40")
    if 55 not in object_ets("egEt", 2):
        fail("HGCAL gamma pattern did not propagate ET=55")
    if object_ets("egEt", 3).count(50) != 1 or len(object_ets("egEt", 3)) != 1:
        fail(f"gamma boundary stitch is wrong: {object_ets('egEt', 3)}")
    if sorted(object_ets("egEt", 4), reverse=True) != [30, 20]:
        fail(f"non-stitching gamma pattern is wrong: {object_ets('egEt', 4)}")
    if 60 not in object_ets("jetEt", 5):
        fail("hadronic boundary stitch did not produce ET=60")
    if values(arrays["sumEx"], 6) != [17, -12, 39]:
        fail(f"Ex aggregation is wrong: {values(arrays['sumEx'], 6)}")
    if values(arrays["sumEy"], 6) != [-1, 20, -19]:
        fail(f"Ey aggregation is wrong: {values(arrays['sumEy'], 6)}")
    if (int(arrays["sumHt"][6]), int(arrays["sumEt"][6]), int(arrays["sumNObj"][6])) != (150, 275, 7):
        fail("scalar sum aggregation is wrong")
    if any(any(values(arrays[f"linkOut{link}_words"], 7)) for link in range(6)):
        fail("reserved input links affected MHH output")

    for event in range(8):
        for link in range(6):
            if values(arrays[f"linkOut{link}_words"], event) != values(arrays[f"linkOut{link}_words"], event + 8):
                fail(f"determinism mismatch for pattern {event}, output link {link}")

    print("MHH analyzer validation: all 16 deterministic events passed")


if __name__ == "__main__":
    main()

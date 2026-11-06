#!/usr/bin/env python3

import argparse
import math
import sys

import ROOT


REQUIRED_BRANCHES = {
    "run",
    "lumi",
    "event",
    "productPresenceMask",
    "gctbPresent",
    "gctbSourceMask",
    "gctsInputDerived",
    "hf_words",
    "dmx_words",
    "mhh_words",
    "gcts_input_words",
    "gcts_sum_words",
    "gcts_gt_words",
    "mhhObjectRaw48",
    "mhhObjectSourceFlag",
    "mhhObjectHwEt",
    "mhhObjectHwEta",
    "mhhObjectPtGeV",
    "gctsObjectRaw48",
    "gctsObjectHwEt",
    "gctsObjectHwEta",
    "gctsObjectIsBarrel",
    "gctsObjectPtGeV",
    "gctsObjectEta",
    "mhhSumEx",
    "mhhSumExGeV",
    "gctsSumEx",
    "gctsSumExGeV",
    "gtObjectHwPt",
    "gtObjectPtGeV",
    "gtMETGeV",
    "gtMHTGeV",
    "gtETGeV",
    "gtHTGeV",
    "genJetPt",
    "genMETPt",
    "dataset",
    "sourceLFN",
    "inputEDM",
    "cmsswRelease",
    "globalTag",
    "cmsswCommit",
    "analyzerCommit",
    "emulatorConfigurationHash",
    "ntupleConfigurationHash",
}


def close(lhs, rhs, tolerance=1e-5):
    return math.isclose(float(lhs), float(rhs), rel_tol=tolerance, abs_tol=tolerance)


def uint64(leaf, index):
    return int(leaf.GetValueLong64(index)) & ((1 << 64) - 1)


def extract48(link_leaf, side, links_per_side, link, slot):
    first = slot * 48
    word = first // 64
    offset = first % 64
    base = side * links_per_side * 9 + link * 9
    value = uint64(link_leaf, base + word) >> offset
    if offset + 48 > 64:
        value |= uint64(link_leaf, base + word + 1) << (64 - offset)
    return value & ((1 << 48) - 1)


def main():
    parser = argparse.ArgumentParser(
        description="Validate the MHH/GCTS training TTree schema and unit conversions"
    )
    parser.add_argument("input", help="Training ROOT file")
    parser.add_argument("--expected-events", type=int)
    parser.add_argument(
        "--expected-edm",
        help="EDM ROOT file whose Events-tree entry count must match the training tree",
    )
    gctb_mode = parser.add_mutually_exclusive_group()
    gctb_mode.add_argument("--expect-zero-gctb", action="store_true")
    gctb_mode.add_argument("--expect-full-gctb", action="store_true")
    parser.add_argument("--require-truth", action="store_true")
    parser.add_argument("--require-nonzero", action="store_true")
    args = parser.parse_args()

    root_file = ROOT.TFile.Open(args.input)
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"cannot open {args.input}")
    tree = root_file.Get("mhhGCTSTrainingNtuplizer/trainingTree")
    if not tree:
        raise RuntimeError("missing mhhGCTSTrainingNtuplizer/trainingTree")

    entries = int(tree.GetEntries())
    if args.expected_events is not None and entries != args.expected_events:
        raise RuntimeError(f"expected {args.expected_events} entries, found {entries}")
    if args.expected_edm:
        edm_file = ROOT.TFile.Open(args.expected_edm)
        if not edm_file or edm_file.IsZombie():
            raise RuntimeError(f"cannot open expected EDM {args.expected_edm}")
        events_tree = edm_file.Get("Events")
        if not events_tree:
            raise RuntimeError(f"missing Events tree in expected EDM {args.expected_edm}")
        edm_entries = int(events_tree.GetEntries())
        if entries != edm_entries:
            raise RuntimeError(
                f"training tree has {entries} entries but expected EDM has {edm_entries}"
            )
        edm_file.Close()

    branches = {branch.GetName() for branch in tree.GetListOfBranches()}
    missing = sorted(REQUIRED_BRANCHES - branches)
    if missing:
        raise RuntimeError("missing branches: " + ", ".join(missing))

    expected_lengths = {
        "hf_words": 2 * 3 * 9,
        "dmx_words": 2 * 16 * 9,
        "mhh_words": 2 * 6 * 9,
        "gcts_input_words": 2 * 12 * 9,
        "gcts_sum_words": 2 * 3 * 9,
        "gcts_gt_words": 6 * 9,
        "mhhObjectRaw48": 4 * 2 * 6,
        "gctsObjectRaw48": 4 * 2 * 6,
        "gtObjectHwPt": 4 * 2 * 6,
    }
    for name, expected in expected_lengths.items():
        actual = int(tree.GetLeaf(name).GetLenStatic())
        if actual != expected:
            raise RuntimeError(f"{name} has static length {actual}, expected {expected}")

    nonzero_payload = False
    nonzero_barrel = False
    mhh_object_pt = tree.GetLeaf("mhhObjectPtGeV")
    mhh_object_raw = tree.GetLeaf("mhhObjectRaw48")
    mhh_object_hw_et = tree.GetLeaf("mhhObjectHwEt")
    mhh_object_hw_eta = tree.GetLeaf("mhhObjectHwEta")
    mhh_object_source = tree.GetLeaf("mhhObjectSourceFlag")
    gcts_object_pt = tree.GetLeaf("gctsObjectPtGeV")
    gcts_object_raw = tree.GetLeaf("gctsObjectRaw48")
    gcts_object_hw_et = tree.GetLeaf("gctsObjectHwEt")
    gcts_object_eta = tree.GetLeaf("gctsObjectEta")
    gcts_object_hw_eta = tree.GetLeaf("gctsObjectHwEta")
    gcts_object_is_barrel = tree.GetLeaf("gctsObjectIsBarrel")
    gt_object_pt = tree.GetLeaf("gtObjectPtGeV")
    gt_object_hw_pt = tree.GetLeaf("gtObjectHwPt")
    mhh_words = tree.GetLeaf("mhh_words")
    gcts_words = tree.GetLeaf("gcts_sum_words")
    gcts_input_words = tree.GetLeaf("gcts_input_words")

    for entry_index in range(entries):
        tree.GetEntry(entry_index)
        if int(tree.productPresenceMask) & 0xFF != 0xFF:
            raise RuntimeError(f"event {tree.event}: a required raw stage is missing")
        if args.expect_zero_gctb:
            if bool(tree.gctbPresent) or int(tree.gctbSourceMask) != 0:
                raise RuntimeError(f"event {tree.event}: GCTB placeholder metadata is incorrect")
            if not bool(tree.gctsInputDerived):
                raise RuntimeError(f"event {tree.event}: expected derived smoke-test GCTS input")
            for side in range(2):
                for link in range(12):
                    for word in range(9):
                        input_index = side * 12 * 9 + link * 9 + word
                        actual = gcts_input_words.GetValueLong64(input_index)
                        if link < 3:
                            mhh_index = side * 6 * 9 + link * 9 + word
                            expected = mhh_words.GetValueLong64(mhh_index)
                        else:
                            expected = 0
                        if actual != expected:
                            raise RuntimeError(
                                f"event {tree.event}: derived GCTS input mismatch at "
                                f"side={side}, link={link}, word={word}"
                            )
        if args.expect_full_gctb:
            if not bool(tree.gctbPresent) or int(tree.gctbSourceMask) != 7:
                raise RuntimeError(f"event {tree.event}: full-barrel metadata is incorrect")
            if bool(tree.gctsInputDerived):
                raise RuntimeError(f"event {tree.event}: expected explicit GCTS input links")
            for side in range(2):
                for link in range(12):
                    for word in range(9):
                        input_index = side * 12 * 9 + link * 9 + word
                        actual = gcts_input_words.GetValueLong64(input_index)
                        if link < 3:
                            mhh_index = side * 6 * 9 + link * 9 + word
                            if actual != mhh_words.GetValueLong64(mhh_index):
                                raise RuntimeError(
                                    f"event {tree.event}: MHH-to-GCTS input mismatch at "
                                    f"side={side}, link={link}, word={word}"
                                )
                        else:
                            nonzero_barrel |= actual != 0

        if args.require_truth and (not bool(tree.genJetsPresent) or not bool(tree.genMETPresent)):
            raise RuntimeError(f"event {tree.event}: generator truth products are missing")

        if not close(tree.gtMETGeV, tree.gtMETHwPt * tree.gtEtLSB):
            raise RuntimeError(f"event {tree.event}: MET unit conversion failed")
        if not close(tree.gtMHTGeV, tree.gtMHTHwPt * tree.gtEtLSB):
            raise RuntimeError(f"event {tree.event}: MHT unit conversion failed")
        if not close(tree.gtETGeV, tree.gtETHw * tree.gtEtLSB):
            raise RuntimeError(f"event {tree.event}: ET unit conversion failed")
        if not close(tree.gtHTGeV, tree.gtHTHw * tree.gtEtLSB):
            raise RuntimeError(f"event {tree.event}: HT unit conversion failed")

        for side in range(2):
            if not close(tree.mhhSumExGeV[side], tree.mhhSumEx[side] * tree.sourceEtLSB):
                raise RuntimeError(f"event {tree.event}: MHH Ex unit conversion failed")
            if not close(tree.gctsSumExGeV[side], tree.gctsSumEx[side] * tree.sourceEtLSB):
                raise RuntimeError(f"event {tree.event}: GCTS Ex unit conversion failed")

        for index in range(4 * 2 * 6):
            object_type = index // 12
            side = (index // 6) % 2
            rank = index % 6
            packed_link = 0 if object_type < 2 else 1
            packed_slot = rank if object_type % 2 == 0 else rank + 6

            expected_mhh_raw = extract48(
                mhh_words, side, 6, packed_link, packed_slot
            )
            if uint64(mhh_object_raw, index) != expected_mhh_raw:
                raise RuntimeError(f"event {tree.event}: MHH raw-object extraction failed")
            if int(mhh_object_source.GetValue(index)) != ((expected_mhh_raw >> 47) & 1):
                raise RuntimeError(f"event {tree.event}: MHH source-flag decoding failed")

            expected_gcts_raw = extract48(
                gcts_words, side, 3, packed_link, packed_slot
            )
            if uint64(gcts_object_raw, index) != expected_gcts_raw:
                raise RuntimeError(f"event {tree.event}: GCTS raw-object extraction failed")
            if int(gcts_object_is_barrel.GetValue(index)) != ((expected_gcts_raw >> 47) & 1):
                raise RuntimeError(f"event {tree.event}: GCTS source-region decoding failed")

            mhh_hw_et = mhh_object_hw_et.GetValue(index)
            if int(mhh_hw_et) != (expected_mhh_raw & 0xFFF):
                raise RuntimeError(f"event {tree.event}: MHH object ET decoding failed")
            expected_mhh_eta = (
                (expected_mhh_raw >> 12) & (0x7F if object_type < 2 else 0x3F)
            )
            if int(mhh_object_hw_eta.GetValue(index)) != expected_mhh_eta:
                raise RuntimeError(f"event {tree.event}: MHH object eta decoding failed")
            if not close(mhh_object_pt.GetValue(index), mhh_hw_et * tree.sourceEtLSB):
                raise RuntimeError(f"event {tree.event}: MHH object unit conversion failed")
            if int(gcts_object_hw_et.GetValue(index)) != (expected_gcts_raw & 0xFFF):
                raise RuntimeError(f"event {tree.event}: GCTS object ET decoding failed")
            if object_type < 2:
                expected_gcts_eta = ((expected_gcts_raw >> 12) & 0x7F) | (
                    ((expected_gcts_raw >> 28) & 1) << 7
                )
            else:
                expected_gcts_eta = (expected_gcts_raw >> 12) & 0x3F
            if int(gcts_object_hw_eta.GetValue(index)) != expected_gcts_eta:
                raise RuntimeError(f"event {tree.event}: GCTS object eta decoding failed")
            if not close(
                gcts_object_pt.GetValue(index),
                gcts_object_hw_et.GetValue(index) * tree.sourceEtLSB,
            ):
                raise RuntimeError(f"event {tree.event}: GCTS object unit conversion failed")
            eta_sign = 1.0 if side == 0 else -1.0
            if not close(
                gcts_object_eta.GetValue(index),
                eta_sign * gcts_object_hw_eta.GetValue(index) * 23.0 * tree.gtEtaLSB,
            ):
                raise RuntimeError(f"event {tree.event}: GCTS object eta conversion failed")
            if not close(
                gt_object_pt.GetValue(index), gt_object_hw_pt.GetValue(index) * tree.gtEtLSB
            ):
                raise RuntimeError(f"event {tree.event}: GT object unit conversion failed")
            nonzero_payload |= bool(mhh_hw_et)

    if args.require_nonzero and not nonzero_payload:
        raise RuntimeError("all decoded MHH object payloads are zero")
    if args.expect_full_gctb and not nonzero_barrel:
        raise RuntimeError("all GCTB input links are zero across the sample")

    print(f"PASS: {entries} training-tree entries and all required branches are valid")
    if args.expected_edm:
        print(f"PASS: training-tree and EDM event counts match ({entries})")
    print("PASS: decoded hardware-to-physical unit conversions are self-consistent")
    if args.expect_zero_gctb:
        print("PASS: every event explicitly records and contains zero-filled GCTB placeholders")
    if args.expect_full_gctb:
        print("PASS: every event has explicit GCTB sources and the sample has nonzero barrel input")
    if args.require_truth:
        print("PASS: generator jet and generator MET products are present in every event")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as error:
        print(f"FAIL: {error}", file=sys.stderr)
        sys.exit(1)

import sys
import awkward as ak
import uproot
import warnings

warnings.filterwarnings("ignore", category=UserWarning, module="numpy")

ROOT_FILE = "gctsum_testvectors.root"
TREE_NAME = "l1TGCTSumAnalyzer/gctSumTree"

def fail(msg):
    print(msg)
    return False

def passed(name):
    print(f"Test {name}: PASSED")
    return True

def failed(name, msg):
    print(f"Test {name}: FAILED")
    print(f"  {msg}")
    return False

def tolist(x):
    return ak.to_list(x)

def nonzero(values):
    return [v for v in tolist(values) if v > 0]

def is_descending(values):
    return values == sorted(values, reverse=True)

def same_words(a, b):
    return tolist(a) == tolist(b)

def only_zero_or_one(words):
    vals = tolist(words)
    bad = [w for w in vals if w not in (0, 1)]
    return len(bad) == 0, bad

def count_nontrivial_words(words):
    return sum(1 for w in tolist(words) if w > 1)

def load_tree():
    f = uproot.open(ROOT_FILE)
    return f[TREE_NAME]

def main():
    try:
        t = load_tree()
    except Exception as exc:
        print(f"FAILED to open {ROOT_FILE}:{TREE_NAME}")
        print(exc)
        sys.exit(1)

    required = [
        "nEgValid", "nEgiValid", "nJetValid", "nTauValid", "nSumValid", "nNonZeroWords",
        "eg_hwPt", "egi_hwPt", "jet_hwPt", "tau_hwPt",
        "sum_ex", "sum_ey", "sum_ht",
        "linkOut0_words", "linkOut1_words", "linkOut2_words",
        "linkOut3_words", "linkOut4_words", "linkOut5_words",
    ]
    missing = [b for b in required if b not in t.keys()]
    if missing:
        print("FAILED: missing branches")
        for m in missing:
            print(f"  {m}")
        sys.exit(1)

    nEg   = t["nEgValid"].array()
    nEgi  = t["nEgiValid"].array()
    nJet  = t["nJetValid"].array()
    nTau  = t["nTauValid"].array()
    nSum  = t["nSumValid"].array()
    nNZ   = t["nNonZeroWords"].array()

    eg_pt  = t["eg_hwPt"].array()
    egi_pt = t["egi_hwPt"].array()
    jet_pt = t["jet_hwPt"].array()
    tau_pt = t["tau_hwPt"].array()

    sum_ex = t["sum_ex"].array()
    sum_ey = t["sum_ey"].array()
    sum_ht = t["sum_ht"].array()

    link0 = t["linkOut0_words"].array()
    link1 = t["linkOut1_words"].array()
    link2 = t["linkOut2_words"].array()
    link3 = t["linkOut3_words"].array()
    link4 = t["linkOut4_words"].array()
    link5 = t["linkOut5_words"].array()

    links = [link0, link1, link2, link3, link4, link5]

    all_ok = True

    # Event count
    if len(nEg) == 12:
        passed("event count")
    else:
        all_ok = failed("event count", f"expected 12 events, got {len(nEg)}")

    # 6 output links, 9 words each
    ok = True
    detail = ""
    for iev in range(len(nEg)):
        sizes = [len(tolist(link[iev])) for link in links]
        if sizes != [9, 9, 9, 9, 9, 9]:
            ok = False
            detail = f"event {iev+1} has link sizes {sizes}, expected [9,9,9,9,9,9]"
            break
    if ok:
        passed("GT output link format (6 links, 9 words each)")
    else:
        all_ok = failed("GT output link format (6 links, 9 words each)", detail)

    # Event 1: all-zero vector should be minimal output
    ok = True
    details = []
    for ilink, link in enumerate(links):
        this_ok, bad = only_zero_or_one(link[0])
        if not this_ok:
            ok = False
            details.append(f"LinkOut{ilink} has non-minimal words {bad[:5]}")
    if ok:
        passed("all-zero vector minimal output")
    else:
        all_ok = failed("all-zero vector minimal output", "; ".join(details))

    # Event 2: single positive EG-like object propagates
    evt = 1
    ok = (int(nEg[evt]) >= 1) and any(count_nontrivial_words(link[evt]) > 0 for link in links)
    if ok:
        passed("single EG-like object propagates")
    else:
        all_ok = failed(
            "single EG-like object propagates",
            f"event 2 has nEgValid={int(nEg[evt])} and no nontrivial output words"
        )

    # Event 3: EG + sums case
    evt = 2
    ok = (int(nEg[evt]) >= 1) and (int(nSum[evt]) >= 1) and (count_nontrivial_words(link5[evt]) > 0)
    if ok:
        passed("EG plus sums case")
    else:
        all_ok = failed(
            "EG plus sums case",
            f"event 3 has nEgValid={int(nEg[evt])}, nSumValid={int(nSum[evt])}, "
            f"nontrivial LinkOut5 words={count_nontrivial_words(link5[evt])}"
        )

    # Event 4: dense positive-side ordering
    evt = 3
    eg_dense = nonzero(eg_pt[evt])
    egi_dense = nonzero(egi_pt[evt])
    ok = True
    problems = []
    if len(eg_dense) > 1 and not is_descending(eg_dense):
        ok = False
        problems.append(f"EG not descending: {eg_dense}")
    if len(egi_dense) > 1 and not is_descending(egi_dense):
        ok = False
        problems.append(f"EGiso not descending: {egi_dense}")
    if ok:
        passed("dense positive-side EG/EGiso ordering")
    else:
        all_ok = failed("dense positive-side EG/EGiso ordering", "; ".join(problems))

    # Event 5: positive/negative eta population changes output
    evt_a = 3  # dense positive only
    evt_b = 4  # both positive and negative eta populated
    identical = all(same_words(link[evt_a], link[evt_b]) for link in links)
    if not identical:
        passed("positive/negative eta separation affects GT output")
    else:
        all_ok = failed(
            "positive/negative eta separation affects GT output",
            "event 4 and event 5 outputs are identical"
        )

    # Event 6: sparse hadron/tau-like pattern and a sum
    evt = 5
    ok = ((int(nJet[evt]) >= 1) or (int(nTau[evt]) >= 1) or (int(nSum[evt]) >= 1))
    if ok:
        passed("hadron/tau-like pattern propagates")
    else:
        all_ok = failed(
            "hadron/tau-like pattern propagates",
            f"event 6 has nJetValid={int(nJet[evt])}, nTauValid={int(nTau[evt])}, nSumValid={int(nSum[evt])}"
        )

    # Determinism: events 1..6 repeat in 7..12
    ok = True
    detail = ""
    for i in range(6):
        for ilink, link in enumerate(links):
            if not same_words(link[i], link[i + 6]):
                ok = False
                detail = f"event {i+1} != event {i+7} for LinkOut{ilink}"
                break
        if not ok:
            break
    if ok:
        passed("deterministic repeat over second 6-event cycle")
    else:
        all_ok = failed("deterministic repeat over second 6-event cycle", detail)

    # Non-zero word count should be stable across repeated pattern
    ok = True
    detail = ""
    for i in range(6):
        if int(nNZ[i]) != int(nNZ[i + 6]):
            ok = False
            detail = f"event {i+1} nNonZeroWords={int(nNZ[i])}, event {i+7} nNonZeroWords={int(nNZ[i+6])}"
            break
    if ok:
        passed("non-zero word count deterministic")
    else:
        all_ok = failed("non-zero word count deterministic", detail)

    # Sum branches self-consistency when sums are present
    ok = True
    detail = ""
    for iev in range(len(nEg)):
        if int(nSum[iev]) > 4:  #this is because in the FW the valid bits are always 1
            ex = nonzero(sum_ex[iev])
            ey = nonzero(sum_ey[iev])
            ht = nonzero(sum_ht[iev])
            if len(ex) == 0 and len(ey) == 0 and len(ht) == 0:
                ok = False
                detail = f"event {iev+1} has nSumValid={int(nSum[iev])} but all decoded sums are zero"
                break
    if ok:
        passed("decoded sum branches consistent with sum validity")
    else:
        all_ok = failed("decoded sum branches consistent with sum validity", detail)

    if not all_ok:
        sys.exit(1)

if __name__ == "__main__":
    main()
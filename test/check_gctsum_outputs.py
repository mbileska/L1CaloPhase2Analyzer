import sys
import awkward as ak
import uproot
import warnings

warnings.filterwarnings("ignore", category=UserWarning, module="numpy")

ROOT_FILE = "gctsum_testvectors.root"
TREE_NAME = "l1TGCTSumAnalyzer/gctSumTree"

if len(sys.argv) > 1:
    ROOT_FILE = sys.argv[1]

def passed(name):
    print(f"Test {name}: PASSED")
    return True

def failed(name, msg):
    print(f"Test {name}: FAILED")
    print(f"  {msg}")
    return False

def tolist(x):
    return ak.to_list(x)

def nonzero(vals):
    return [v for v in tolist(vals) if v > 0]

def is_desc(vals):
    return vals == sorted(vals, reverse=True)

def count_nonzero_words(words):
    return sum(1 for w in tolist(words) if w != 0)

def load_tree(path):
    f = uproot.open(path)
    return f[TREE_NAME]

def event_links(tree, iev, prefix, n):
    return [tree[f"{prefix}{i}_words"].array()[iev] for i in range(n)]

def active_input_links(tree, iev):
    out = []
    for i in range(24):
        if any(w != 0 for w in tolist(tree[f"linkIn{i}_words"].array()[iev])):
            out.append(i)
    return out

def top_nonzero(vals):
    nz = nonzero(vals)
    return max(nz) if nz else 0

def count_nonzero_by_sign(pt_arr, sign_arr, want_sign):
    pts = tolist(pt_arr)
    signs = tolist(sign_arr)
    n = 0
    for pt, s in zip(pts, signs):
        if pt > 0 and s == want_sign:
            n += 1
    return n

def main():
    try:
      t = load_tree(ROOT_FILE)
    except Exception as exc:
      print(f"FAILED to open {ROOT_FILE}:{TREE_NAME}")
      print(exc)
      sys.exit(1)

    needed = [
        "patternId",
        "nEgNonZero", "nEgiNonZero", "nJetNonZero", "nTauNonZero", "nSumNonZero",
        "eg_hwPt", "eg_isPosEta", "egi_hwPt", "jet_hwPt", "jet_isPosEta",
        "tau_hwPt", "tau_isPosEta", "sum_ht_pos", "sum_ht_neg", "sum_et2", "sum_nobj",
    ] + [f"linkIn{i}_words" for i in range(24)] + [f"linkOut{i}_words" for i in range(6)]

    missing = [b for b in needed if b not in t.keys()]
    if missing:
        print("FAILED: missing branches")
        for m in missing:
            print(f"  {m}")
        sys.exit(1)

    patternId = t["patternId"].array()
    nEg = t["nEgNonZero"].array()
    nEgi = t["nEgiNonZero"].array()
    nJet = t["nJetNonZero"].array()
    nTau = t["nTauNonZero"].array()
    nSum = t["nSumNonZero"].array()

    eg_pt = t["eg_hwPt"].array()
    eg_sign = t["eg_isPosEta"].array()
    egi_pt = t["egi_hwPt"].array()
    jet_pt = t["jet_hwPt"].array()
    jet_sign = t["jet_isPosEta"].array()
    tau_pt = t["tau_hwPt"].array()
    tau_sign = t["tau_isPosEta"].array()

    sum_ht_pos = t["sum_ht_pos"].array()
    sum_ht_neg = t["sum_ht_neg"].array()
    sum_et2 = t["sum_et2"].array()
    sum_nobj = t["sum_nobj"].array()

    all_ok = True

    # 20 events = 10-pattern cycle repeated twice
    if len(patternId) == 20:
        passed("event count")
    else:
        all_ok = failed("event count", f"expected 20 events, got {len(patternId)}")

    # input format
    ok = True
    detail = ""
    for iev in range(len(patternId)):
        for i in range(24):
            vals = tolist(t[f"linkIn{i}_words"].array()[iev])
            if len(vals) != 9:
                ok = False
                detail = f"event {iev+1}, linkIn{i} has {len(vals)} words, expected 9"
                break
        if not ok:
            break
    if ok:
        passed("input link format (24 links, 9 words each)")
    else:
        all_ok = failed("input link format (24 links, 9 words each)", detail)

    # output format
    ok = True
    detail = ""
    for iev in range(len(patternId)):
        for i in range(6):
            vals = tolist(t[f"linkOut{i}_words"].array()[iev])
            if len(vals) != 9:
                ok = False
                detail = f"event {iev+1}, linkOut{i} has {len(vals)} words, expected 9"
                break
        if not ok:
            break
    if ok:
        passed("output link format (6 links, 9 words each)")
    else:
        all_ok = failed("output link format (6 links, 9 words each)", detail)

    # pattern 0: all zero input
    for iev in [0, 10]:
        ok = True
        active = active_input_links(t, iev)
        if active != []:
            ok = False
            all_ok = failed(f"pattern 0 event {iev+1}", f"active input links {active}, expected none")
            continue
        if int(nEg[iev]) != 0 or int(nEgi[iev]) != 0 or int(nJet[iev]) != 0 or int(nTau[iev]) != 0:
            ok = False
            all_ok = failed(
                f"pattern 0 event {iev+1}",
                f"nonzero object counts: EG={int(nEg[iev])}, EGI={int(nEgi[iev])}, Jet={int(nJet[iev])}, Tau={int(nTau[iev])}"
            )
            continue
        if int(nSum[iev]) != 0:
            ok = False
            all_ok = failed(f"pattern 0 event {iev+1}", f"nSumNonZero={int(nSum[iev])}, expected 0")
            continue
        passed(f"pattern 0 event {iev+1}")

    # pattern 1: single EG
    for iev in [1, 11]:
        active = active_input_links(t, iev)
        if active != [0]:
            all_ok = failed(f"pattern 1 event {iev+1}", f"active input links {active}, expected [0]")
            continue
        if int(nEg[iev]) < 1:
            all_ok = failed(f"pattern 1 event {iev+1}", f"nEgNonZero={int(nEg[iev])}, expected >= 1")
            continue
        if int(nJet[iev]) != 0 or int(nTau[iev]) != 0 or int(nSum[iev]) != 0:
            all_ok = failed(
                f"pattern 1 event {iev+1}",
                f"unexpected nonzero counts: Jet={int(nJet[iev])}, Tau={int(nTau[iev])}, Sum={int(nSum[iev])}"
            )
            continue
        passed(f"pattern 1 event {iev+1}")

    # pattern 2: EG + sum
    for iev in [2, 12]:
        active = active_input_links(t, iev)
        if active != [0, 2]:
            all_ok = failed(f"pattern 2 event {iev+1}", f"active input links {active}, expected [0, 2]")
            continue
        if int(nEg[iev]) < 1:
            all_ok = failed(f"pattern 2 event {iev+1}", f"nEgNonZero={int(nEg[iev])}, expected >= 1")
            continue
        if int(nSum[iev]) < 1:
            all_ok = failed(f"pattern 2 event {iev+1}", f"nSumNonZero={int(nSum[iev])}, expected >= 1")
            continue
        passed(f"pattern 2 event {iev+1}")

    # pattern 3: dense ordering EG/EGiso
    for iev in [3, 13]:
        egvals = nonzero(eg_pt[iev])
        egivals = nonzero(egi_pt[iev])
        if len(egvals) < 2 or not is_desc(egvals):
            all_ok = failed(f"pattern 3 event {iev+1}", f"EG not descending: {egvals}")
            continue
        if len(egivals) < 2 or not is_desc(egivals):
            all_ok = failed(f"pattern 3 event {iev+1}", f"EGiso not descending: {egivals}")
            continue
        passed(f"pattern 3 event {iev+1}")

    # pattern 4: both eta sides active
    for iev in [4, 14]:
        pos_objs = count_nonzero_by_sign(eg_pt[iev], eg_sign[iev], 1) \
                 + count_nonzero_by_sign(jet_pt[iev], jet_sign[iev], 1) \
                 + count_nonzero_by_sign(tau_pt[iev], tau_sign[iev], 1)
        neg_objs = count_nonzero_by_sign(eg_pt[iev], eg_sign[iev], 0) \
                 + count_nonzero_by_sign(jet_pt[iev], jet_sign[iev], 0) \
                 + count_nonzero_by_sign(tau_pt[iev], tau_sign[iev], 0)
        if pos_objs < 1 or neg_objs < 1:
            all_ok = failed(f"pattern 4 event {iev+1}", f"pos_objs={pos_objs}, neg_objs={neg_objs}, expected both >= 1")
            continue
        passed(f"pattern 4 event {iev+1}")

    # pattern 5: hadron/tau + sum
    for iev in [5, 15]:
        if int(nJet[iev]) < 1 and int(nTau[iev]) < 1:
            all_ok = failed(f"pattern 5 event {iev+1}", f"nJetNonZero={int(nJet[iev])}, nTauNonZero={int(nTau[iev])}, expected at least one nonzero")
            continue
        if int(nSum[iev]) < 1:
            all_ok = failed(f"pattern 5 event {iev+1}", f"nSumNonZero={int(nSum[iev])}, expected >= 1")
            continue
        passed(f"pattern 5 event {iev+1}")

    # pattern 6: stitch same phi => 20 + 12 = 32, GT hwPt = 32 << 4
    for iev in [6, 16]:
        lead = top_nonzero(eg_pt[iev])
        if lead != (32 << 4):
            all_ok = failed(f"pattern 6 event {iev+1}", f"leading EG hwPt={lead}, expected {32<<4}")
            continue
        passed(f"pattern 6 event {iev+1}")

    # pattern 7: stitch dphi=+1 => 18 + 11 = 29, GT hwPt = 29 << 4
    for iev in [7, 17]:
        lead = top_nonzero(eg_pt[iev])
        if lead != (29 << 4):
            all_ok = failed(f"pattern 7 event {iev+1}", f"leading EG hwPt={lead}, expected {29<<4}")
            continue
        passed(f"pattern 7 event {iev+1}")

    # pattern 8: no stitch dphi>1 => leading stays 20 << 4, not 32 << 4
    for iev in [8, 18]:
        vals = nonzero(eg_pt[iev])
        lead = max(vals) if vals else 0
        if lead != (20 << 4):
            all_ok = failed(f"pattern 8 event {iev+1}", f"leading EG hwPt={lead}, expected {20<<4}")
            continue
        if (32 << 4) in vals:
            all_ok = failed(f"pattern 8 event {iev+1}", f"found stitched value {(32<<4)} in {vals}, expected no stitch")
            continue
        passed(f"pattern 8 event {iev+1}")

    # pattern 9: side-reduced sum aggregation with source-hypothesis selection
    for iev in [9, 19]:
        ht_pos_vals = nonzero(sum_ht_pos[iev])
        ht_neg_vals = nonzero(sum_ht_neg[iev])
        et2_vals = nonzero(sum_et2[iev])
        nobj_vals = nonzero(sum_nobj[iev])
        if 17 not in ht_pos_vals or 60 not in ht_neg_vals or 58301 not in et2_vals or 30 not in nobj_vals:
            all_ok = failed(
                f"pattern 9 event {iev+1}",
                f"decoded sums ht_pos={ht_pos_vals}, ht_neg={ht_neg_vals}, et2={et2_vals}, nobj={nobj_vals}, expected 17, 60, 58301, 30"
            )
            continue
        passed(f"pattern 9 event {iev+1}")

    # determinism
    ok = True
    detail = ""
    for i in range(10):
        for j in range(6):
            a = tolist(t[f"linkOut{j}_words"].array()[i])
            b = tolist(t[f"linkOut{j}_words"].array()[i + 10])
            if a != b:
                ok = False
                detail = f"event {i+1} != event {i+11} for linkOut{j}"
                break
        if not ok:
            break
    if ok:
        passed("deterministic repeat over second 10-event cycle")
    else:
        all_ok = failed("deterministic repeat over second 10-event cycle", detail)

    if not all_ok:
        sys.exit(1)

if __name__ == "__main__":
    main()

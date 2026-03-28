import uproot
import awkward as ak

f = uproot.open("gctsum_testvectors.root")
t = f["l1TGCTSumAnalyzer/gctSumTree"]

nEg   = t["nEgValid"].array()
nEgi  = t["nEgiValid"].array()
nJet  = t["nJetValid"].array()
nTau  = t["nTauValid"].array()
nSum  = t["nSumValid"].array()

eg_pt  = t["eg_hwPt"].array()
jet_pt = t["jet_hwPt"].array()

link0 = t["linkOut0_words"].array()
link1 = t["linkOut1_words"].array()
link2 = t["linkOut2_words"].array()
link3 = t["linkOut3_words"].array()
link4 = t["linkOut4_words"].array()
link5 = t["linkOut5_words"].array()

print("nEgValid unique:", set(ak.to_list(nEg)))
print("nEgiValid unique:", set(ak.to_list(nEgi)))
print("nJetValid unique:", set(ak.to_list(nJet)))
print("nTauValid unique:", set(ak.to_list(nTau)))
print("nSumValid unique:", set(ak.to_list(nSum)))

print("EG hwPt first event:", ak.to_list(eg_pt[0]))
print("Jet hwPt first event:", ak.to_list(jet_pt[0]))
print("LinkOut0 first event:", ak.to_list(link0[0]))
print("LinkOut1 first event:", ak.to_list(link1[0]))
print("LinkOut2 first event:", ak.to_list(link2[0]))
print("LinkOut3 first event:", ak.to_list(link3[0]))
print("LinkOut4 first event:", ak.to_list(link4[0]))
print("LinkOut5 first event:", ak.to_list(link5[0]))

# determinism check
print("All LinkOut0 identical:", all(ak.to_list(x) == ak.to_list(link0[0]) for x in link0))
print("All LinkOut2 identical:", all(ak.to_list(x) == ak.to_list(link2[0]) for x in link2))
print("All LinkOut3 identical:", all(ak.to_list(x) == ak.to_list(link3[0]) for x in link3))

# monotonic sort checks
eg0 = [x for x in ak.to_list(eg_pt[0]) if x > 0]
jet0 = [x for x in ak.to_list(jet_pt[0]) if x > 0]
print("EG sorted descending:", eg0 == sorted(eg0, reverse=True))
print("Jet sorted descending:", jet0 == sorted(jet0, reverse=True))
#include <ap_int.h>
#include <array>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1Trigger/L1CaloPhase2Analyzer/interface/L1TGCTSumAnalyzer.h"
#include "L1Trigger/L1CaloTrigger/interface/GCTSumToGT_h.h"

using namespace edm;

namespace {
  inline ap_uint<576> packWordsTo576(const std::vector<uint64_t>& words) {
    ap_uint<576> out = 0;
    for (unsigned int i = 0; i < words.size() && i < 9; ++i) {
      out.range(i * 64 + 63, i * 64) = words[i];
    }
    return out;
  }

  inline std::vector<unsigned long long> unpack576ToWords(ap_uint<576> data) {
    std::vector<unsigned long long> out;
    out.reserve(9);
    for (int i = 0; i < 9; ++i) {
      out.push_back((unsigned long long)data.range(i * 64 + 63, i * 64).to_uint64());
    }
    return out;
  }

  inline int leadingNonZero(const std::vector<int>& values) {
    int lead = 0;
    for (const auto& v : values) {
      if (v > lead)
        lead = v;
    }
    return lead;
  }

  inline void decodeCandidateWord(const ap_uint<64>& word,
                                  std::vector<int>& hwPt,
                                  std::vector<int>& hwEta,
                                  std::vector<int>& hwPhi,
                                  std::vector<int>& isPosEta,
                                  bool posEtaFlag) {
    p2gctsumGT::GTvar obj;
    obj.getGTvar(word);

    hwPt.push_back((int)obj.ET);
    hwEta.push_back((int)obj.Eta);
    hwPhi.push_back((int)obj.Phi);
    isPosEta.push_back(posEtaFlag ? 1 : 0);
  }

  inline int decodeScalar16Word(const ap_uint<64>& word) {
    return (int)word.range(16, 1);
  }

  inline int decodeScalar32Word(const ap_uint<64>& word) {
    return (int)word.range(32, 1);
  }

  inline void decodeGTLinks(const std::array<ap_uint<576>, 6>& links,
                            std::vector<int>& eg_hwPt,
                            std::vector<int>& eg_hwEta,
                            std::vector<int>& eg_hwPhi,
                            std::vector<int>& eg_isPosEta,
                            std::vector<int>& egi_hwPt,
                            std::vector<int>& egi_hwEta,
                            std::vector<int>& egi_hwPhi,
                            std::vector<int>& egi_isPosEta,
                            std::vector<int>& jet_hwPt,
                            std::vector<int>& jet_hwEta,
                            std::vector<int>& jet_hwPhi,
                            std::vector<int>& jet_isPosEta,
                            std::vector<int>& tau_hwPt,
                            std::vector<int>& tau_hwEta,
                            std::vector<int>& tau_hwPhi,
                            std::vector<int>& tau_isPosEta,
                            std::vector<int>& sum_ht_pos,
                            std::vector<int>& sum_ht_neg,
                            std::vector<int>& sum_et2,
                            std::vector<int>& sum_nobj) {
    for (int j = 0; j < 6; ++j)
      decodeCandidateWord(links[0].range(j * 64 + 63, j * 64), eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta, true);
    for (int j = 6; j < 9; ++j)
      decodeCandidateWord(links[0].range(j * 64 + 63, j * 64), eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta, false);

    for (int j = 0; j < 3; ++j)
      decodeCandidateWord(links[1].range(j * 64 + 63, j * 64), eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta, false);
    for (int j = 3; j < 9; ++j)
      decodeCandidateWord(links[1].range(j * 64 + 63, j * 64), egi_hwPt, egi_hwEta, egi_hwPhi, egi_isPosEta, true);

    for (int j = 0; j < 6; ++j)
      decodeCandidateWord(links[2].range(j * 64 + 63, j * 64), egi_hwPt, egi_hwEta, egi_hwPhi, egi_isPosEta, false);
    for (int j = 6; j < 9; ++j)
      decodeCandidateWord(links[2].range(j * 64 + 63, j * 64), jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta, true);

    for (int j = 0; j < 3; ++j)
      decodeCandidateWord(links[3].range(j * 64 + 63, j * 64), jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta, true);
    for (int j = 3; j < 9; ++j)
      decodeCandidateWord(links[3].range(j * 64 + 63, j * 64), jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta, false);

    for (int j = 0; j < 6; ++j)
      decodeCandidateWord(links[4].range(j * 64 + 63, j * 64), tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta, true);
    for (int j = 6; j < 9; ++j)
      decodeCandidateWord(links[4].range(j * 64 + 63, j * 64), tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta, false);

    for (int j = 0; j < 3; ++j)
      decodeCandidateWord(links[5].range(j * 64 + 63, j * 64), tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta, false);
    sum_ht_pos.push_back(decodeScalar16Word(links[5].range(3 * 64 + 63, 3 * 64)));
    sum_ht_neg.push_back(decodeScalar16Word(links[5].range(4 * 64 + 63, 4 * 64)));
    sum_et2.push_back(decodeScalar32Word(links[5].range(5 * 64 + 63, 5 * 64)));
    sum_nobj.push_back(decodeScalar16Word(links[5].range(6 * 64 + 63, 6 * 64)));
  }

  inline int countNonZero(const std::vector<int>& vals) {
    return std::count_if(vals.begin(), vals.end(), [](int v) { return v > 0; });
  }
}

L1TGCTSumAnalyzer::L1TGCTSumAnalyzer(const ParameterSet& cfg)
    : debug_(cfg.getUntrackedParameter<bool>("debug", false)),
      sumInputDumpFile_(cfg.getUntrackedParameter<std::string>(
          "sumInputDumpFile",
          "/afs/hep.wisc.edu/home/mbileska/apx-fs-r2-gctsum/xF13P/AlgoSRC/SUM_IP/dummy_sum_input.txt")),
      cmsswGtOutputDumpFile_(cfg.getUntrackedParameter<std::string>(
          "cmsswGtOutputDumpFile",
          "/afs/hep.wisc.edu/home/mbileska/apx-fs-r2-gctsum/xF13P/AlgoSRC/TO_GT_IP/cmssw_gt_output.txt")) {
  usesResource("TFileService");

  folderName_ = cfg.getUntrackedParameter<std::string>("folderName");

  const std::vector<edm::InputTag> inputLinks = cfg.getParameter<std::vector<edm::InputTag> >("inputLinks");
  const std::vector<edm::InputTag> outputLinks = cfg.getParameter<std::vector<edm::InputTag> >("outputLinks");

  if (inputLinks.size() != 24) {
    throw cms::Exception("L1TGCTSumAnalyzer") << "Expected 24 input link tags";
  }
  if (outputLinks.size() != 6) {
    throw cms::Exception("L1TGCTSumAnalyzer") << "Expected 6 output link tags";
  }

  for (unsigned int i = 0; i < 24; ++i) {
    inputLinkSrc_[i] = consumes<std::vector<uint64_t> >(inputLinks[i]);
  }
  for (unsigned int i = 0; i < 6; ++i) {
    outputLinkSrc_[i] = consumes<std::vector<uint64_t> >(outputLinks[i]);
  }

  gctSumTree = tfs_->make<TTree>("gctSumTree", "GCT SumCard IO Tree");
  gctSumTree->Branch("run", &run, "run/I");
  gctSumTree->Branch("lumi", &lumi, "lumi/I");
  gctSumTree->Branch("event", &event, "event/I");
  gctSumTree->Branch("patternId", &patternId, "patternId/I");

  gctSumTree->Branch("nEgNonZero", &nEgNonZero, "nEgNonZero/I");
  gctSumTree->Branch("nEgiNonZero", &nEgiNonZero, "nEgiNonZero/I");
  gctSumTree->Branch("nJetNonZero", &nJetNonZero, "nJetNonZero/I");
  gctSumTree->Branch("nTauNonZero", &nTauNonZero, "nTauNonZero/I");
  gctSumTree->Branch("nSumNonZero", &nSumNonZero, "nSumNonZero/I");

  for (unsigned int i = 0; i < 24; ++i) {
    gctSumTree->Branch((std::string("linkIn") + std::to_string(i) + "_words").c_str(), &linkIn_words[i], 32000, 0);
  }
  for (unsigned int i = 0; i < 6; ++i) {
    gctSumTree->Branch((std::string("linkOut") + std::to_string(i) + "_words").c_str(), &linkOut_words[i], 32000, 0);
  }

  gctSumTree->Branch("eg_hwPt", &eg_hwPt, 32000, 0);
  gctSumTree->Branch("eg_hwEta", &eg_hwEta, 32000, 0);
  gctSumTree->Branch("eg_hwPhi", &eg_hwPhi, 32000, 0);
  gctSumTree->Branch("eg_isPosEta", &eg_isPosEta, 32000, 0);

  gctSumTree->Branch("egi_hwPt", &egi_hwPt, 32000, 0);
  gctSumTree->Branch("egi_hwEta", &egi_hwEta, 32000, 0);
  gctSumTree->Branch("egi_hwPhi", &egi_hwPhi, 32000, 0);
  gctSumTree->Branch("egi_isPosEta", &egi_isPosEta, 32000, 0);

  gctSumTree->Branch("jet_hwPt", &jet_hwPt, 32000, 0);
  gctSumTree->Branch("jet_hwEta", &jet_hwEta, 32000, 0);
  gctSumTree->Branch("jet_hwPhi", &jet_hwPhi, 32000, 0);
  gctSumTree->Branch("jet_isPosEta", &jet_isPosEta, 32000, 0);

  gctSumTree->Branch("tau_hwPt", &tau_hwPt, 32000, 0);
  gctSumTree->Branch("tau_hwEta", &tau_hwEta, 32000, 0);
  gctSumTree->Branch("tau_hwPhi", &tau_hwPhi, 32000, 0);
  gctSumTree->Branch("tau_isPosEta", &tau_isPosEta, 32000, 0);

  gctSumTree->Branch("sum_ht_pos", &sum_ht_pos, 32000, 0);
  gctSumTree->Branch("sum_ht_neg", &sum_ht_neg, 32000, 0);
  gctSumTree->Branch("sum_et2", &sum_et2, 32000, 0);
  gctSumTree->Branch("sum_nobj", &sum_nobj, 32000, 0);

  h_nEgNonZero = tfs_->make<TH1F>("h_nEgNonZero", "Nonzero EG objects;N;Events", 13, -0.5, 12.5);
  h_nEgiNonZero = tfs_->make<TH1F>("h_nEgiNonZero", "Nonzero EGiso objects;N;Events", 13, -0.5, 12.5);
  h_nJetNonZero = tfs_->make<TH1F>("h_nJetNonZero", "Nonzero Jet objects;N;Events", 13, -0.5, 12.5);
  h_nTauNonZero = tfs_->make<TH1F>("h_nTauNonZero", "Nonzero Tau objects;N;Events", 13, -0.5, 12.5);
  h_nSumNonZero = tfs_->make<TH1F>("h_nSumNonZero", "Nonzero sums;N;Events", 5, -0.5, 4.5);
}

void L1TGCTSumAnalyzer::beginJob() {
  writeSumInputHeader();
  writeCmsswGtOutputHeader();
}

void L1TGCTSumAnalyzer::clearVectors() {
  for (auto& v : linkIn_words)
    v.clear();
  for (auto& v : linkOut_words)
    v.clear();

  eg_hwPt.clear(); eg_hwEta.clear(); eg_hwPhi.clear(); eg_isPosEta.clear();
  egi_hwPt.clear(); egi_hwEta.clear(); egi_hwPhi.clear(); egi_isPosEta.clear();
  jet_hwPt.clear(); jet_hwEta.clear(); jet_hwPhi.clear(); jet_isPosEta.clear();
  tau_hwPt.clear(); tau_hwEta.clear(); tau_hwPhi.clear(); tau_isPosEta.clear();
  sum_ht_pos.clear();
  sum_ht_neg.clear();
  sum_et2.clear();
  sum_nobj.clear();
}

void L1TGCTSumAnalyzer::writeSumInputHeader() {
  std::ofstream out(sumInputDumpFile_, std::ios::out);
  out << "WordCnt    ";
  for (int i = 0; i < 32; ++i) {
    out << "L" << std::setw(2) << std::setfill('0') << i;
    if (i != 31)
      out << "                  ";
  }
  out << "\n#BeginData\n";
  out.close();
}

void L1TGCTSumAnalyzer::writeCmsswGtOutputHeader() {
  std::ofstream out(cmsswGtOutputDumpFile_, std::ios::out);
  out << "WordCnt               T118                T119                T120                T121                T122                T123\n";
  out << "#BeginData\n";
  out.close();
}

void L1TGCTSumAnalyzer::appendSumInputEvent(const std::array<std::vector<unsigned long long>, 24>& inWords) {
  std::ofstream out(sumInputDumpFile_, std::ios::app);

  for (int row = 0; row < 9; ++row) {
    out << std::setw(4) << std::setfill('0') << std::hex << row << "        ";

    // Columns 0..11 -> SLR2 inputs (CMSSW links 0..11)
    for (int col = 0; col < 12; ++col) {
      out << std::nouppercase << std::setw(16) << std::setfill('0') << std::hex << inWords[col][row] << "      ";
    }

    // Columns 12..15 -> unused, zero
    for (int col = 12; col < 16; ++col) {
      out << std::nouppercase << std::setw(16) << std::setfill('0') << std::hex << 0ULL << "      ";
    }

    // Columns 16..27 -> SLR3 inputs (CMSSW links 12..23)
    for (int col = 16; col < 28; ++col) {
      out << std::nouppercase << std::setw(16) << std::setfill('0') << std::hex << inWords[col - 4][row] << "      ";
    }

    // Columns 28..31 -> unused, zero
    for (int col = 28; col < 32; ++col) {
      out << std::nouppercase << std::setw(16) << std::setfill('0') << std::hex << 0ULL;
      if (col != 31)
        out << "      ";
    }
    out << "\n";
  }

  out.close();
}

void L1TGCTSumAnalyzer::appendCmsswGtOutputEvent(const std::array<std::vector<unsigned long long>, 6>& outWords) {
  std::ofstream out(cmsswGtOutputDumpFile_, std::ios::app);

  for (int row = 0; row < 9; ++row) {
    out << std::setw(4) << std::setfill('0') << std::hex << row << "        ";
    for (int col = 0; col < 6; ++col) {
      out << std::nouppercase << std::setw(16) << std::setfill('0') << std::hex << outWords[col][row];
      if (col != 5)
        out << "      ";
    }
    out << "\n";
  }

  out.close();
}

void L1TGCTSumAnalyzer::analyze(const Event& evt, const EventSetup& es) {
  (void)es;

  run = evt.id().run();
  lumi = evt.luminosityBlock();
  event = evt.id().event();
  patternId = (event - 1) % 10;

  clearVectors();

  std::array<ap_uint<576>, 6> outLinks;
  outLinks.fill(0);

  for (unsigned int i = 0; i < 24; ++i) {
    edm::Handle<std::vector<uint64_t> > handle;
    evt.getByToken(inputLinkSrc_[i], handle);
    if (handle.isValid()) {
      linkIn_words[i] = unpack576ToWords(packWordsTo576(*handle));
    } else {
      linkIn_words[i] = std::vector<unsigned long long>(9, 0ULL);
    }
  }

  for (unsigned int i = 0; i < 6; ++i) {
    edm::Handle<std::vector<uint64_t> > handle;
    evt.getByToken(outputLinkSrc_[i], handle);
    if (handle.isValid()) {
      outLinks[i] = packWordsTo576(*handle);
      linkOut_words[i] = unpack576ToWords(outLinks[i]);
    } else {
      linkOut_words[i] = std::vector<unsigned long long>(9, 0ULL);
    }
  }

  decodeGTLinks(outLinks,
                eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta,
                egi_hwPt, egi_hwEta, egi_hwPhi, egi_isPosEta,
                jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta,
                tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta,
                sum_ht_pos, sum_ht_neg, sum_et2, sum_nobj);

  nEgNonZero = countNonZero(eg_hwPt);
  nEgiNonZero = countNonZero(egi_hwPt);
  nJetNonZero = countNonZero(jet_hwPt);
  nTauNonZero = countNonZero(tau_hwPt);
  nSumNonZero = 0;
  for (const int value : sum_ht_pos)
    if (value > 0)
      ++nSumNonZero;
  for (const int value : sum_ht_neg)
    if (value > 0)
      ++nSumNonZero;
  for (const int value : sum_et2)
    if (value > 0)
      ++nSumNonZero;
  for (const int value : sum_nobj)
    if (value > 0)
      ++nSumNonZero;

  h_nEgNonZero->Fill(nEgNonZero);
  h_nEgiNonZero->Fill(nEgiNonZero);
  h_nJetNonZero->Fill(nJetNonZero);
  h_nTauNonZero->Fill(nTauNonZero);
  h_nSumNonZero->Fill(nSumNonZero);

  appendSumInputEvent(linkIn_words);
  appendCmsswGtOutputEvent(linkOut_words);

  if (debug_) {
    edm::LogVerbatim("L1TGCTSumAnalyzer") << "event=" << event
                                          << " patternId=" << patternId
                                          << " nEgNonZero=" << nEgNonZero
                                          << " nEgiNonZero=" << nEgiNonZero
                                          << " nJetNonZero=" << nJetNonZero
                                          << " nTauNonZero=" << nTauNonZero
                                          << " nSumNonZero=" << nSumNonZero;
  }

  gctSumTree->Fill();
}

void L1TGCTSumAnalyzer::endJob() {}
L1TGCTSumAnalyzer::~L1TGCTSumAnalyzer() {}
DEFINE_FWK_MODULE(L1TGCTSumAnalyzer);

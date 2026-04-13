// /*
//  *  \file L1TGCTSumAnalyzer.cc
//  *  GCT SumCard analyzer for exact GT-link validation
//  */

// #include <ap_int.h>
// #include <array>
// #include <algorithm>
// #include <vector>
// #include <iomanip>
// #include <sstream>
// #include <iostream>

// #include "FWCore/Framework/interface/MakerMacros.h"
// #include "FWCore/MessageLogger/interface/MessageLogger.h"

// #include "L1Trigger/L1CaloPhase2Analyzer/interface/L1TGCTSumAnalyzer.h"
// #include "L1Trigger/L1CaloTrigger/interface/GCTSumToGT_h.h"

// using namespace edm;


// namespace {
//   void dumpLinkWords(const std::vector<uint64_t>& words, const std::string& name) {
//     std::cout << "  " << name << " nWords=" << words.size() << std::endl;
//     for (size_t i = 0; i < words.size(); ++i) {
//       std::cout << "    [" << i << "] 0x"
//                 << std::hex << std::setw(16) << std::setfill('0') << words[i]
//                 << std::dec << std::setfill(' ') << std::endl;
//     }
//   }
// }

// namespace {
//   inline ap_uint<576> packWordsTo576(const std::vector<uint64_t>& words) {
//     ap_uint<576> out = 0;
//     for (unsigned int i = 0; i < words.size() && i < 9; ++i) {
//       out.range(i * 64 + 63, i * 64) = words[i];
//     }
//     return out;
//   }

//   inline std::vector<unsigned long long> unpack576ToWords(ap_uint<576> data) {
//     std::vector<unsigned long long> out;
//     out.reserve(9);
//     for (int i = 0; i < 9; ++i) {
//       out.push_back((unsigned long long)data.range(i * 64 + 63, i * 64).to_uint64());
//     }
//     return out;
//   }

//   inline int leadingNonZero(const std::vector<int>& values) {
//     int lead = 0;
//     for (const auto& v : values) {
//       if (v > lead)
//         lead = v;
//     }
//     return lead;
//   }

//   inline std::string wordsToHexString(const std::vector<unsigned long long>& words) {
//     std::ostringstream ss;
//     for (unsigned int i = 0; i < words.size(); ++i) {
//       if (i)
//         ss << " ";
//       ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << words[i];
//     }
//     return ss.str();
//   }

//   inline void decodeCandidateWord(const ap_uint<64>& word,
//                                   std::vector<int>& valid,
//                                   std::vector<int>& hwPt,
//                                   std::vector<int>& hwEta,
//                                   std::vector<int>& hwPhi,
//                                   std::vector<int>& isPosEta,
//                                   bool posEtaFlag) {
//     p2gctsumGT::GTvar obj;
//     obj.getGTvar(word);

//     valid.push_back((int)obj.isValid);
//     hwPt.push_back((int)obj.ET);
//     hwEta.push_back((int)obj.Eta);
//     hwPhi.push_back((int)obj.Phi);
//     isPosEta.push_back(posEtaFlag ? 1 : 0);
//   }

//   inline void decodeSumWord(const ap_uint<64>& word,
//                             std::vector<int>& valid,
//                             std::vector<int>& ex,
//                             std::vector<int>& ey,
//                             std::vector<int>& ht) {
//     int sumValid = (int)word.range(0, 0);
//     int sumEx = (int)word.range(16, 1);
//     int sumEy = (int)word.range(29, 17);
//     int sumHt = (int)word.range(45, 30);

//     valid.push_back(sumValid);
//     ex.push_back(sumEx >> 4);
//     ey.push_back(sumEy);
//     ht.push_back(sumHt >> 4);
//   }

//   inline void decodeGTLinks(const std::array<ap_uint<576>, 6>& links,
//                             std::vector<int>& eg_valid,
//                             std::vector<int>& eg_hwPt,
//                             std::vector<int>& eg_hwEta,
//                             std::vector<int>& eg_hwPhi,
//                             std::vector<int>& eg_isPosEta,
//                             std::vector<int>& egi_valid,
//                             std::vector<int>& egi_hwPt,
//                             std::vector<int>& egi_hwEta,
//                             std::vector<int>& egi_hwPhi,
//                             std::vector<int>& egi_isPosEta,
//                             std::vector<int>& jet_valid,
//                             std::vector<int>& jet_hwPt,
//                             std::vector<int>& jet_hwEta,
//                             std::vector<int>& jet_hwPhi,
//                             std::vector<int>& jet_isPosEta,
//                             std::vector<int>& tau_valid,
//                             std::vector<int>& tau_hwPt,
//                             std::vector<int>& tau_hwEta,
//                             std::vector<int>& tau_hwPhi,
//                             std::vector<int>& tau_isPosEta,
//                             std::vector<int>& sum_valid,
//                             std::vector<int>& sum_ex,
//                             std::vector<int>& sum_ey,
//                             std::vector<int>& sum_ht) {
//     for (int j = 0; j < 6; ++j)
//       decodeCandidateWord(links[0].range(j * 64 + 63, j * 64), eg_valid, eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta, true);
//     for (int j = 6; j < 9; ++j)
//       decodeCandidateWord(links[0].range(j * 64 + 63, j * 64), eg_valid, eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta, false);

//     for (int j = 0; j < 3; ++j)
//       decodeCandidateWord(links[1].range(j * 64 + 63, j * 64), eg_valid, eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta, false);
//     for (int j = 3; j < 9; ++j)
//       decodeCandidateWord(links[1].range(j * 64 + 63, j * 64), egi_valid, egi_hwPt, egi_hwEta, egi_hwPhi, egi_isPosEta, true);

//     for (int j = 0; j < 6; ++j)
//       decodeCandidateWord(links[2].range(j * 64 + 63, j * 64), egi_valid, egi_hwPt, egi_hwEta, egi_hwPhi, egi_isPosEta, false);
//     for (int j = 6; j < 9; ++j)
//       decodeCandidateWord(links[2].range(j * 64 + 63, j * 64), jet_valid, jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta, true);

//     for (int j = 0; j < 3; ++j)
//       decodeCandidateWord(links[3].range(j * 64 + 63, j * 64), jet_valid, jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta, true);
//     for (int j = 3; j < 9; ++j)
//       decodeCandidateWord(links[3].range(j * 64 + 63, j * 64), jet_valid, jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta, false);

//     for (int j = 0; j < 6; ++j)
//       decodeCandidateWord(links[4].range(j * 64 + 63, j * 64), tau_valid, tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta, true);
//     for (int j = 6; j < 9; ++j)
//       decodeCandidateWord(links[4].range(j * 64 + 63, j * 64), tau_valid, tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta, false);

//     for (int j = 0; j < 3; ++j)
//       decodeCandidateWord(links[5].range(j * 64 + 63, j * 64), tau_valid, tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta, false);
//     for (int j = 3; j < 7; ++j)
//       decodeSumWord(links[5].range(j * 64 + 63, j * 64), sum_valid, sum_ex, sum_ey, sum_ht);
//   }
// }

// L1TGCTSumAnalyzer::L1TGCTSumAnalyzer(const ParameterSet& cfg)
//     : link0Src_(consumes<std::vector<uint64_t> >(cfg.getParameter<edm::InputTag>("LinkOut0"))),
//       link1Src_(consumes<std::vector<uint64_t> >(cfg.getParameter<edm::InputTag>("LinkOut1"))),
//       link2Src_(consumes<std::vector<uint64_t> >(cfg.getParameter<edm::InputTag>("LinkOut2"))),
//       link3Src_(consumes<std::vector<uint64_t> >(cfg.getParameter<edm::InputTag>("LinkOut3"))),
//       link4Src_(consumes<std::vector<uint64_t> >(cfg.getParameter<edm::InputTag>("LinkOut4"))),
//       link5Src_(consumes<std::vector<uint64_t> >(cfg.getParameter<edm::InputTag>("LinkOut5"))) {
//   usesResource("TFileService");

//   folderName_ = cfg.getUntrackedParameter<std::string>("folderName");
//   debug_ = cfg.getUntrackedParameter<bool>("debug", false);

//   gctSumTree = tfs_->make<TTree>("gctSumTree", "GCT SumCard Output Tree");
//   gctSumTree->Branch("run", &run, "run/I");
//   gctSumTree->Branch("lumi", &lumi, "lumi/I");
//   gctSumTree->Branch("event", &event, "event/I");
//   gctSumTree->Branch("nEgValid", &nEgValid, "nEgValid/I");
//   gctSumTree->Branch("nEgiValid", &nEgiValid, "nEgiValid/I");
//   gctSumTree->Branch("nJetValid", &nJetValid, "nJetValid/I");
//   gctSumTree->Branch("nTauValid", &nTauValid, "nTauValid/I");
//   gctSumTree->Branch("nSumValid", &nSumValid, "nSumValid/I");
//   gctSumTree->Branch("nNonZeroWords", &nNonZeroWords, "nNonZeroWords/I");

//   gctSumTree->Branch("linkOut0_words", "vector<unsigned long long>", &linkOut0_words, 32000, 0);
//   gctSumTree->Branch("linkOut1_words", "vector<unsigned long long>", &linkOut1_words, 32000, 0);
//   gctSumTree->Branch("linkOut2_words", "vector<unsigned long long>", &linkOut2_words, 32000, 0);
//   gctSumTree->Branch("linkOut3_words", "vector<unsigned long long>", &linkOut3_words, 32000, 0);
//   gctSumTree->Branch("linkOut4_words", "vector<unsigned long long>", &linkOut4_words, 32000, 0);
//   gctSumTree->Branch("linkOut5_words", "vector<unsigned long long>", &linkOut5_words, 32000, 0);

//   gctSumTree->Branch("eg_valid", "vector<int>", &eg_valid, 32000, 0);
//   gctSumTree->Branch("eg_hwPt", "vector<int>", &eg_hwPt, 32000, 0);
//   gctSumTree->Branch("eg_hwEta", "vector<int>", &eg_hwEta, 32000, 0);
//   gctSumTree->Branch("eg_hwPhi", "vector<int>", &eg_hwPhi, 32000, 0);
//   gctSumTree->Branch("eg_isPosEta", "vector<int>", &eg_isPosEta, 32000, 0);

//   gctSumTree->Branch("egi_valid", "vector<int>", &egi_valid, 32000, 0);
//   gctSumTree->Branch("egi_hwPt", "vector<int>", &egi_hwPt, 32000, 0);
//   gctSumTree->Branch("egi_hwEta", "vector<int>", &egi_hwEta, 32000, 0);
//   gctSumTree->Branch("egi_hwPhi", "vector<int>", &egi_hwPhi, 32000, 0);
//   gctSumTree->Branch("egi_isPosEta", "vector<int>", &egi_isPosEta, 32000, 0);

//   gctSumTree->Branch("jet_valid", "vector<int>", &jet_valid, 32000, 0);
//   gctSumTree->Branch("jet_hwPt", "vector<int>", &jet_hwPt, 32000, 0);
//   gctSumTree->Branch("jet_hwEta", "vector<int>", &jet_hwEta, 32000, 0);
//   gctSumTree->Branch("jet_hwPhi", "vector<int>", &jet_hwPhi, 32000, 0);
//   gctSumTree->Branch("jet_isPosEta", "vector<int>", &jet_isPosEta, 32000, 0);

//   gctSumTree->Branch("tau_valid", "vector<int>", &tau_valid, 32000, 0);
//   gctSumTree->Branch("tau_hwPt", "vector<int>", &tau_hwPt, 32000, 0);
//   gctSumTree->Branch("tau_hwEta", "vector<int>", &tau_hwEta, 32000, 0);
//   gctSumTree->Branch("tau_hwPhi", "vector<int>", &tau_hwPhi, 32000, 0);
//   gctSumTree->Branch("tau_isPosEta", "vector<int>", &tau_isPosEta, 32000, 0);

//   gctSumTree->Branch("sum_valid", "vector<int>", &sum_valid, 32000, 0);
//   gctSumTree->Branch("sum_ex", "vector<int>", &sum_ex, 32000, 0);
//   gctSumTree->Branch("sum_ey", "vector<int>", &sum_ey, 32000, 0);
//   gctSumTree->Branch("sum_ht", "vector<int>", &sum_ht, 32000, 0);

//   h_nEgValid = tfs_->make<TH1F>("h_nEgValid", "Number of valid EG objects;N valid;Events", 13, -0.5, 12.5);
//   h_nEgiValid = tfs_->make<TH1F>("h_nEgiValid", "Number of valid EGI objects;N valid;Events", 13, -0.5, 12.5);
//   h_nJetValid = tfs_->make<TH1F>("h_nJetValid", "Number of valid jet objects;N valid;Events", 13, -0.5, 12.5);
//   h_nTauValid = tfs_->make<TH1F>("h_nTauValid", "Number of valid tau objects;N valid;Events", 13, -0.5, 12.5);
//   h_nSumValid = tfs_->make<TH1F>("h_nSumValid", "Number of valid sums;N valid;Events", 5, -0.5, 4.5);
//   h_nonZeroWords = tfs_->make<TH1F>("h_nonZeroWords", "Number of non-zero GT words per event;N non-zero words;Events", 55, -0.5, 54.5);
//   h_leadingEgHwPt = tfs_->make<TH1F>("h_leadingEgHwPt", "Leading EG p_{T};hw p_{T};Events", 256, -0.5, 4095.5);
//   h_leadingEgiHwPt = tfs_->make<TH1F>("h_leadingEgiHwPt", "Leading EGI p_{T};hw p_{T};Events", 256, -0.5, 4095.5);
//   h_leadingJetHwPt = tfs_->make<TH1F>("h_leadingJetHwPt", "Leading Jet p_{T};hw p_{T};Events", 256, -0.5, 4095.5);
//   h_leadingTauHwPt = tfs_->make<TH1F>("h_leadingTauHwPt", "Leading Tau p_{T};hw p_{T};Events", 256, -0.5, 4095.5);
//   h_sumEx = tfs_->make<TH1F>("h_sumEx", "Ex;Ex;Entries", 256, -0.5, 4095.5);
//   h_sumEy = tfs_->make<TH1F>("h_sumEy", "Ey;Ey;Entries", 256, -0.5, 4095.5);
//   h_sumHt = tfs_->make<TH1F>("h_sumHt", "Ht;Ht;Entries", 256, -0.5, 4095.5);
// }

// void L1TGCTSumAnalyzer::beginJob() {
//   edm::LogVerbatim("L1TGCTSumAnalyzer") << "L1TGCTSumAnalyzer beginJob: expecting 6 GT output links as vector<uint64_t>";
// }

// void L1TGCTSumAnalyzer::clearVectors() {
//   linkOut0_words.clear(); linkOut1_words.clear(); linkOut2_words.clear();
//   linkOut3_words.clear(); linkOut4_words.clear(); linkOut5_words.clear();
//   eg_valid.clear(); eg_hwPt.clear(); eg_hwEta.clear(); eg_hwPhi.clear(); eg_isPosEta.clear();
//   egi_valid.clear(); egi_hwPt.clear(); egi_hwEta.clear(); egi_hwPhi.clear(); egi_isPosEta.clear();
//   jet_valid.clear(); jet_hwPt.clear(); jet_hwEta.clear(); jet_hwPhi.clear(); jet_isPosEta.clear();
//   tau_valid.clear(); tau_hwPt.clear(); tau_hwEta.clear(); tau_hwPhi.clear(); tau_isPosEta.clear();
//   sum_valid.clear(); sum_ex.clear(); sum_ey.clear(); sum_ht.clear();
// }

// void L1TGCTSumAnalyzer::analyze(const Event& evt, const EventSetup& es) {
//   std::cout << "\n[L1TGCTSumAnalyzer] run=" << evt.id().run()
//             << " lumi=" << evt.luminosityBlock()
//             << " event=" << evt.id().event() << std::endl;
//   (void)es;
//   run = evt.id().run();
//   lumi = evt.id().luminosityBlock();
//   event = evt.id().event();
//   clearVectors();
//   nEgValid = nEgiValid = nJetValid = nTauValid = nSumValid = nNonZeroWords = 0;

//   edm::Handle<std::vector<uint64_t> > linkHandles[6];
//   evt.getByToken(link0Src_, linkHandles[0]);
//   evt.getByToken(link1Src_, linkHandles[1]);
//   evt.getByToken(link2Src_, linkHandles[2]);
//   evt.getByToken(link3Src_, linkHandles[3]);
//   evt.getByToken(link4Src_, linkHandles[4]);
//   evt.getByToken(link5Src_, linkHandles[5]);

//   std::array<ap_uint<576>, 6> links;
//   links.fill(0);

//   for (unsigned int i = 0; i < 6; ++i) {
//     if (!linkHandles[i].isValid()) {
//       std::cout << "  LinkOut" << i << " valid=0" << std::endl;
//       edm::LogWarning("L1TGCTSumAnalyzer") << "Missing input product for LinkOut" << i;
//       continue;
//     }

//     std::cout << "  LinkOut" << i << " valid=1 size=" << linkHandles[i]->size() << std::endl;
//     dumpLinkWords(*linkHandles[i], std::string("LinkOut") + std::to_string(i));

//     links[i] = packWordsTo576(*linkHandles[i]);
//   }

//   linkOut0_words = unpack576ToWords(links[0]);
//   linkOut1_words = unpack576ToWords(links[1]);
//   linkOut2_words = unpack576ToWords(links[2]);
//   linkOut3_words = unpack576ToWords(links[3]);
//   linkOut4_words = unpack576ToWords(links[4]);
//   linkOut5_words = unpack576ToWords(links[5]);

//   decodeGTLinks(links,
//                 eg_valid, eg_hwPt, eg_hwEta, eg_hwPhi, eg_isPosEta,
//                 egi_valid, egi_hwPt, egi_hwEta, egi_hwPhi, egi_isPosEta,
//                 jet_valid, jet_hwPt, jet_hwEta, jet_hwPhi, jet_isPosEta,
//                 tau_valid, tau_hwPt, tau_hwEta, tau_hwPhi, tau_isPosEta,
//                 sum_valid, sum_ex, sum_ey, sum_ht);

//   const std::vector<std::vector<unsigned long long>*> allWordVecs = {
//       &linkOut0_words, &linkOut1_words, &linkOut2_words, &linkOut3_words, &linkOut4_words, &linkOut5_words};
//   for (const auto* v : allWordVecs) {
//     for (const auto& word : *v) {
//       if (word != 0ULL)
//         ++nNonZeroWords;
//     }
//   }

//   nEgValid = std::count(eg_valid.begin(), eg_valid.end(), 1);
//   nEgiValid = std::count(egi_valid.begin(), egi_valid.end(), 1);
//   nJetValid = std::count(jet_valid.begin(), jet_valid.end(), 1);
//   nTauValid = std::count(tau_valid.begin(), tau_valid.end(), 1);
//   nSumValid = std::count(sum_valid.begin(), sum_valid.end(), 1);

//   h_nEgValid->Fill(nEgValid);
//   h_nEgiValid->Fill(nEgiValid);
//   h_nJetValid->Fill(nJetValid);
//   h_nTauValid->Fill(nTauValid);
//   h_nSumValid->Fill(nSumValid);
//   h_nonZeroWords->Fill(nNonZeroWords);
//   h_leadingEgHwPt->Fill(leadingNonZero(eg_hwPt));
//   h_leadingEgiHwPt->Fill(leadingNonZero(egi_hwPt));
//   h_leadingJetHwPt->Fill(leadingNonZero(jet_hwPt));
//   h_leadingTauHwPt->Fill(leadingNonZero(tau_hwPt));
//   for (const auto& v : sum_ex) if (v > 0) h_sumEx->Fill(v);
//   for (const auto& v : sum_ey) if (v > 0) h_sumEy->Fill(v);
//   for (const auto& v : sum_ht) if (v > 0) h_sumHt->Fill(v);

//   if (debug_) {
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "Event " << event
//                                            << " nEgValid=" << nEgValid
//                                            << " nEgiValid=" << nEgiValid
//                                            << " nJetValid=" << nJetValid
//                                            << " nTauValid=" << nTauValid
//                                            << " nSumValid=" << nSumValid;
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "LinkOut0 words: " << wordsToHexString(linkOut0_words);
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "LinkOut1 words: " << wordsToHexString(linkOut1_words);
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "LinkOut2 words: " << wordsToHexString(linkOut2_words);
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "LinkOut3 words: " << wordsToHexString(linkOut3_words);
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "LinkOut4 words: " << wordsToHexString(linkOut4_words);
//     edm::LogVerbatim("L1TGCTSumAnalyzer") << "LinkOut5 words: " << wordsToHexString(linkOut5_words);
//   }

//   gctSumTree->Fill();
// }

// void L1TGCTSumAnalyzer::endJob() {}
// L1TGCTSumAnalyzer::~L1TGCTSumAnalyzer() {}
// DEFINE_FWK_MODULE(L1TGCTSumAnalyzer);
/*
 *  \file L1TGCTSumAnalyzer.cc
 *  GCT SumCard analyzer for GT-link validation
 */

/*
 *  \file L1TGCTSumAnalyzer.cc
 *  GCT SumCard analyzer for exact GT-link validation
 */

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

  inline void decodeSumWord(const ap_uint<64>& word,
                            std::vector<int>& ex,
                            std::vector<int>& ey,
                            std::vector<int>& ht) {
    int sumEx = (int)word.range(16, 1);
    int sumEy = (int)word.range(29, 17);
    int sumHt = (int)word.range(45, 30);

    ex.push_back(sumEx >> 4);
    ey.push_back(sumEy);
    ht.push_back(sumHt >> 4);
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
                            std::vector<int>& sum_ex,
                            std::vector<int>& sum_ey,
                            std::vector<int>& sum_ht) {
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
    for (int j = 3; j < 7; ++j)
      decodeSumWord(links[5].range(j * 64 + 63, j * 64), sum_ex, sum_ey, sum_ht);
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

  gctSumTree->Branch("sum_ex", &sum_ex, 32000, 0);
  gctSumTree->Branch("sum_ey", &sum_ey, 32000, 0);
  gctSumTree->Branch("sum_ht", &sum_ht, 32000, 0);

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
  sum_ex.clear(); sum_ey.clear(); sum_ht.clear();
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
                sum_ex, sum_ey, sum_ht);

  nEgNonZero = countNonZero(eg_hwPt);
  nEgiNonZero = countNonZero(egi_hwPt);
  nJetNonZero = countNonZero(jet_hwPt);
  nTauNonZero = countNonZero(tau_hwPt);
  nSumNonZero = 0;
  for (unsigned int i = 0; i < sum_ex.size(); ++i) {
    if (sum_ex[i] > 0 || sum_ey[i] > 0 || sum_ht[i] > 0)
      ++nSumNonZero;
  }

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
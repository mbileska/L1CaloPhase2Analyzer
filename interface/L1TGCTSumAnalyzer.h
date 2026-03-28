#ifndef L1Trigger_L1CaloPhase2Analyzer_L1TGCTSumAnalyzer_h
#define L1Trigger_L1CaloPhase2Analyzer_L1TGCTSumAnalyzer_h

#include <vector>
#include <string>

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TH1F.h"
#include "TTree.h"

class L1TGCTSumAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit L1TGCTSumAnalyzer(const edm::ParameterSet&);
  ~L1TGCTSumAnalyzer() override;

  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

private:
  void clearVectors();

  edm::EDGetTokenT<std::vector<uint64_t> > link0Src_;
  edm::EDGetTokenT<std::vector<uint64_t> > link1Src_;
  edm::EDGetTokenT<std::vector<uint64_t> > link2Src_;
  edm::EDGetTokenT<std::vector<uint64_t> > link3Src_;
  edm::EDGetTokenT<std::vector<uint64_t> > link4Src_;
  edm::EDGetTokenT<std::vector<uint64_t> > link5Src_;

  edm::Service<TFileService> tfs_;
  std::string folderName_;
  bool debug_;

  TTree* gctSumTree;

  int run;
  int lumi;
  int event;
  int nEgValid;
  int nEgiValid;
  int nJetValid;
  int nTauValid;
  int nSumValid;
  int nNonZeroWords;

  std::vector<unsigned long long> linkOut0_words;
  std::vector<unsigned long long> linkOut1_words;
  std::vector<unsigned long long> linkOut2_words;
  std::vector<unsigned long long> linkOut3_words;
  std::vector<unsigned long long> linkOut4_words;
  std::vector<unsigned long long> linkOut5_words;

  std::vector<int> eg_valid;
  std::vector<int> eg_hwPt;
  std::vector<int> eg_hwEta;
  std::vector<int> eg_hwPhi;
  std::vector<int> eg_isPosEta;

  std::vector<int> egi_valid;
  std::vector<int> egi_hwPt;
  std::vector<int> egi_hwEta;
  std::vector<int> egi_hwPhi;
  std::vector<int> egi_isPosEta;

  std::vector<int> jet_valid;
  std::vector<int> jet_hwPt;
  std::vector<int> jet_hwEta;
  std::vector<int> jet_hwPhi;
  std::vector<int> jet_isPosEta;

  std::vector<int> tau_valid;
  std::vector<int> tau_hwPt;
  std::vector<int> tau_hwEta;
  std::vector<int> tau_hwPhi;
  std::vector<int> tau_isPosEta;

  std::vector<int> sum_valid;
  std::vector<int> sum_ex;
  std::vector<int> sum_ey;
  std::vector<int> sum_ht;

  TH1F* h_nEgValid;
  TH1F* h_nEgiValid;
  TH1F* h_nJetValid;
  TH1F* h_nTauValid;
  TH1F* h_nSumValid;
  TH1F* h_nonZeroWords;
  TH1F* h_leadingEgHwPt;
  TH1F* h_leadingEgiHwPt;
  TH1F* h_leadingJetHwPt;
  TH1F* h_leadingTauHwPt;
  TH1F* h_sumEx;
  TH1F* h_sumEy;
  TH1F* h_sumHt;
};

#endif

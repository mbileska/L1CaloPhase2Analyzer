#ifndef L1Trigger_L1CaloPhase2Analyzer_L1TGCTSumAnalyzer_h
#define L1Trigger_L1CaloPhase2Analyzer_L1TGCTSumAnalyzer_h

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

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
  void writeSumInputHeader();
  void writeCmsswGtOutputHeader();
  void appendSumInputEvent(const std::array<std::vector<unsigned long long>, 24>& inWords);
  void appendCmsswGtOutputEvent(const std::array<std::vector<unsigned long long>, 6>& outWords);

  std::array<edm::EDGetTokenT<std::vector<uint64_t>>, 24> inputLinkSrc_;
  std::array<edm::EDGetTokenT<std::vector<uint64_t>>, 6> outputLinkSrc_;

  std::string folderName_;
  bool debug_;

  std::string sumInputDumpFile_;
  std::string cmsswGtOutputDumpFile_;

  edm::Service<TFileService> tfs_;

  TTree* gctSumTree;

  int run;
  int lumi;
  int event;
  int patternId;

  int nEgNonZero;
  int nEgiNonZero;
  int nJetNonZero;
  int nTauNonZero;
  int nSumNonZero;

  std::array<std::vector<unsigned long long>, 24> linkIn_words;
  std::array<std::vector<unsigned long long>, 6> linkOut_words;

  std::vector<int> eg_hwPt;
  std::vector<int> eg_hwEta;
  std::vector<int> eg_hwPhi;
  std::vector<int> eg_isPosEta;

  std::vector<int> egi_hwPt;
  std::vector<int> egi_hwEta;
  std::vector<int> egi_hwPhi;
  std::vector<int> egi_isPosEta;

  std::vector<int> jet_hwPt;
  std::vector<int> jet_hwEta;
  std::vector<int> jet_hwPhi;
  std::vector<int> jet_isPosEta;

  std::vector<int> tau_hwPt;
  std::vector<int> tau_hwEta;
  std::vector<int> tau_hwPhi;
  std::vector<int> tau_isPosEta;

  std::vector<int> sum_ht_pos;
  std::vector<int> sum_ht_neg;
  std::vector<int> sum_et2;
  std::vector<int> sum_nobj;

  TH1F* h_nEgNonZero;
  TH1F* h_nEgiNonZero;
  TH1F* h_nJetNonZero;
  TH1F* h_nTauNonZero;
  TH1F* h_nSumNonZero;
};

#endif

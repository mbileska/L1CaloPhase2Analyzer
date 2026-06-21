#ifndef L1Trigger_L1CaloPhase2Analyzer_L1TMHHAnalyzer_h
#define L1Trigger_L1CaloPhase2Analyzer_L1TMHHAnalyzer_h

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "TTree.h"

class L1TMHHAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit L1TMHHAnalyzer(const edm::ParameterSet&);
  ~L1TMHHAnalyzer() override = default;

  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override {}

private:
  static constexpr unsigned int kInputLinks = 16;
  static constexpr unsigned int kOutputLinks = 6;
  static constexpr unsigned int kWordsPerLink = 9;

  void writeHeader(const std::string&, unsigned int, const std::string&) const;
  void appendEvent(const std::string&,
                   const std::array<std::vector<unsigned long long>, kInputLinks>&) const;
  void appendEvent(const std::string&,
                   const std::array<std::vector<unsigned long long>, kOutputLinks>&) const;
  void decodeOutputs();

  std::array<edm::EDGetTokenT<std::vector<uint64_t>>, kInputLinks> inputTokens_;
  std::array<edm::EDGetTokenT<std::vector<uint64_t>>, kOutputLinks> outputTokens_;
  std::string inputDumpFile_;
  std::string outputDumpFile_;
  bool debug_;

  edm::Service<TFileService> tfs_;
  TTree* tree_;
  int run_;
  int lumi_;
  int event_;
  int patternId_;
  std::array<std::vector<unsigned long long>, kInputLinks> inputWords_;
  std::array<std::vector<unsigned long long>, kOutputLinks> outputWords_;
  std::vector<int> egEt_;
  std::vector<int> egiEt_;
  std::vector<int> jetEt_;
  std::vector<int> tauEt_;
  std::vector<int> sumEx_;
  std::vector<int> sumEy_;
  int sumHt_;
  int sumEt_;
  int sumNObj_;
};

#endif

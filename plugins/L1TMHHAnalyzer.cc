#include "L1Trigger/L1CaloPhase2Analyzer/interface/L1TMHHAnalyzer.h"

#include <ap_int.h>

#include <iomanip>
#include <fstream>
#include <sstream>

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

namespace {
template <std::size_t N>
void appendLinkTable(const std::string& path,
                     const std::array<std::vector<unsigned long long>, N>& links) {
  std::ofstream output(path, std::ios::app);
  if (!output.is_open()) {
    throw cms::Exception("L1TMHHAnalyzer") << "Could not append to " << path;
  }
  for (unsigned int row = 0; row < 9; ++row) {
    output << std::setw(4) << std::setfill('0') << std::hex << row;
    for (unsigned int link = 0; link < N; ++link) {
      output << "    " << std::setw(16) << std::setfill('0') << std::nouppercase << links[link][row];
    }
    output << '\n';
  }
}

int signed12(unsigned int value) {
  return (value & 0x800U) != 0U ? static_cast<int>(value) - 0x1000 : static_cast<int>(value);
}
}  // namespace

L1TMHHAnalyzer::L1TMHHAnalyzer(const edm::ParameterSet& config)
    : inputDumpFile_(config.getUntrackedParameter<std::string>("inputDumpFile", "mhh_input.txt")),
      outputDumpFile_(config.getUntrackedParameter<std::string>("outputDumpFile", "mhh_cmssw_output.txt")),
      debug_(config.getUntrackedParameter<bool>("debug", false)),
      tree_(nullptr),
      run_(0),
      lumi_(0),
      event_(0),
      patternId_(0),
      sumHt_(0),
      sumEt_(0),
      sumNObj_(0) {
  usesResource("TFileService");

  const auto inputLinks = config.getParameter<std::vector<edm::InputTag>>("inputLinks");
  const auto outputLinks = config.getParameter<std::vector<edm::InputTag>>("outputLinks");
  if (inputLinks.size() != kInputLinks || outputLinks.size() != kOutputLinks) {
    throw cms::Exception("L1TMHHAnalyzer")
        << "Expected " << kInputLinks << " input tags and " << kOutputLinks << " output tags; got "
        << inputLinks.size() << " and " << outputLinks.size();
  }
  for (unsigned int i = 0; i < kInputLinks; ++i) {
    inputTokens_[i] = consumes<std::vector<uint64_t>>(inputLinks[i]);
  }
  for (unsigned int i = 0; i < kOutputLinks; ++i) {
    outputTokens_[i] = consumes<std::vector<uint64_t>>(outputLinks[i]);
  }

  tree_ = tfs_->make<TTree>("mhhTree", "MHH emulator input and output");
  tree_->Branch("run", &run_, "run/I");
  tree_->Branch("lumi", &lumi_, "lumi/I");
  tree_->Branch("event", &event_, "event/I");
  tree_->Branch("patternId", &patternId_, "patternId/I");
  for (unsigned int i = 0; i < kInputLinks; ++i) {
    tree_->Branch((std::string("linkIn") + std::to_string(i) + "_words").c_str(), &inputWords_[i], 32000, 0);
  }
  for (unsigned int i = 0; i < kOutputLinks; ++i) {
    tree_->Branch((std::string("linkOut") + std::to_string(i) + "_words").c_str(), &outputWords_[i], 32000, 0);
  }
  tree_->Branch("egEt", &egEt_, 32000, 0);
  tree_->Branch("egiEt", &egiEt_, 32000, 0);
  tree_->Branch("jetEt", &jetEt_, 32000, 0);
  tree_->Branch("tauEt", &tauEt_, 32000, 0);
  tree_->Branch("sumEx", &sumEx_, 32000, 0);
  tree_->Branch("sumEy", &sumEy_, 32000, 0);
  tree_->Branch("sumHt", &sumHt_, "sumHt/I");
  tree_->Branch("sumEt", &sumEt_, "sumEt/I");
  tree_->Branch("sumNObj", &sumNObj_, "sumNObj/I");
}

void L1TMHHAnalyzer::writeHeader(const std::string& path,
                                unsigned int links,
                                const std::string& label) const {
  std::ofstream output(path, std::ios::out);
  if (!output.is_open()) {
    throw cms::Exception("L1TMHHAnalyzer") << "Could not create " << path;
  }
  output << "WordCnt";
  for (unsigned int i = 0; i < links; ++i) {
    output << "    " << label << i;
  }
  output << "\n#BeginData\n";
}

void L1TMHHAnalyzer::beginJob() {
  writeHeader(inputDumpFile_, kInputLinks, "IN_");
  writeHeader(outputDumpFile_, kOutputLinks, "OUT_");
}

void L1TMHHAnalyzer::appendEvent(
    const std::string& path,
    const std::array<std::vector<unsigned long long>, kInputLinks>& links) const {
  appendLinkTable(path, links);
}

void L1TMHHAnalyzer::appendEvent(
    const std::string& path,
    const std::array<std::vector<unsigned long long>, kOutputLinks>& links) const {
  appendLinkTable(path, links);
}

void L1TMHHAnalyzer::decodeOutputs() {
  egEt_.clear();
  egiEt_.clear();
  jetEt_.clear();
  tauEt_.clear();
  sumEx_.clear();
  sumEy_.clear();
  sumHt_ = 0;
  sumEt_ = 0;
  sumNObj_ = 0;

  std::array<ap_uint<576>, kOutputLinks> links{};
  for (unsigned int link = 0; link < kOutputLinks; ++link) {
    for (unsigned int word = 0; word < kWordsPerLink; ++word) {
      links[link].range(word * 64 + 63, word * 64) = outputWords_[link][word];
    }
  }

  for (unsigned int slot = 0; slot < 12; ++slot) {
    const int gammaEt = links[0].range(slot * 48 + 11, slot * 48).to_uint();
    const int hadronEt = links[1].range(slot * 48 + 11, slot * 48).to_uint();
    (slot < 6 ? egEt_ : egiEt_).push_back(gammaEt);
    (slot < 6 ? jetEt_ : tauEt_).push_back(hadronEt);
  }
  for (unsigned int scenario = 0; scenario < 3; ++scenario) {
    sumEx_.push_back(signed12(links[2].range(scenario * 12 + 11, scenario * 12).to_uint()));
    const unsigned int start = 48 + scenario * 12;
    sumEy_.push_back(signed12(links[2].range(start + 11, start).to_uint()));
  }
  sumHt_ = links[2].range(107, 96).to_uint();
  sumEt_ = links[2].range(155, 144).to_uint();
  sumNObj_ = links[2].range(203, 192).to_uint();
}

void L1TMHHAnalyzer::analyze(const edm::Event& event, const edm::EventSetup&) {
  run_ = event.id().run();
  lumi_ = event.luminosityBlock();
  event_ = event.id().event();
  patternId_ = (event_ - 1) % 8;

  for (unsigned int link = 0; link < kInputLinks; ++link) {
    edm::Handle<std::vector<uint64_t>> words;
    event.getByToken(inputTokens_[link], words);
    if (!words.isValid() || words->size() != kWordsPerLink) {
      throw cms::Exception("L1TMHHAnalyzer") << "Input link " << link << " is missing or does not contain 9 words";
    }
    inputWords_[link].assign(words->begin(), words->end());
  }
  for (unsigned int link = 0; link < kOutputLinks; ++link) {
    edm::Handle<std::vector<uint64_t>> words;
    event.getByToken(outputTokens_[link], words);
    if (!words.isValid() || words->size() != kWordsPerLink) {
      throw cms::Exception("L1TMHHAnalyzer") << "Output link " << link << " is missing or does not contain 9 words";
    }
    outputWords_[link].assign(words->begin(), words->end());
  }

  decodeOutputs();
  appendEvent(inputDumpFile_, inputWords_);
  appendEvent(outputDumpFile_, outputWords_);
  tree_->Fill();

  if (debug_) {
    edm::LogVerbatim("L1TMHHAnalyzer") << "event=" << event_ << " pattern=" << patternId_;
  }
}

DEFINE_FWK_MODULE(L1TMHHAnalyzer);

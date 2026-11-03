#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/METReco/interface/GenMET.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "TTree.h"

namespace {
  constexpr unsigned int kSides = 2;
  constexpr unsigned int kWords = 9;
  constexpr unsigned int kHFLinks = 3;
  constexpr unsigned int kDMXLinks = 16;
  constexpr unsigned int kMHHLinks = 6;
  constexpr unsigned int kGCTSInputLinks = 12;
  constexpr unsigned int kGCTSSideLinks = 3;
  constexpr unsigned int kGTLinks = 6;
  constexpr unsigned int kObjectTypes = 4;
  constexpr unsigned int kObjectsPerTypeAndSide = 6;
  constexpr double kPi = 3.14159265358979323846;

  enum ObjectType : unsigned int { kEG = 0, kEGIso = 1, kJet = 2, kTau = 3 };

  int signExtend(std::uint64_t value, unsigned int width) {
    const std::uint64_t mask = (std::uint64_t{1} << width) - 1;
    value &= mask;
    const std::uint64_t sign = std::uint64_t{1} << (width - 1);
    return (value & sign) ? static_cast<int>(value - (std::uint64_t{1} << width)) : static_cast<int>(value);
  }

  std::uint64_t extractBits(const unsigned long long words[kWords], unsigned int first, unsigned int width) {
    const unsigned int word = first / 64;
    const unsigned int offset = first % 64;
    unsigned __int128 value = static_cast<unsigned __int128>(words[word]) >> offset;
    if (offset + width > 64 && word + 1 < kWords) {
      value |= static_cast<unsigned __int128>(words[word + 1]) << (64 - offset);
    }
    const unsigned __int128 mask = (static_cast<unsigned __int128>(1) << width) - 1;
    return static_cast<std::uint64_t>(value & mask);
  }
}  // namespace

class MHHGCTSTrainingNtuplizer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit MHHGCTSTrainingNtuplizer(const edm::ParameterSet&);
  ~MHHGCTSTrainingNtuplizer() override = default;

private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;

  void readLink(const edm::Event&,
                const edm::EDGetTokenT<std::vector<std::uint64_t>>&,
                unsigned long long destination[kWords],
                const std::string& label) const;
  void resetEvent();
  void buildGCTSInputs(const edm::Event&);
  void decodeMHHObjects();
  void decodeMHHSums();
  void decodeGCTSObjects();
  void decodeGCTSSideSums();
  void decodeGTObjects();
  void decodeGTSums();
  void decodeGTObject(unsigned int type, unsigned int side, unsigned int rank, std::uint64_t word);

  std::array<edm::EDGetTokenT<std::vector<std::uint64_t>>, kSides * kHFLinks> hfTokens_;
  std::array<edm::EDGetTokenT<std::vector<std::uint64_t>>, kSides * kDMXLinks> dmxTokens_;
  std::array<edm::EDGetTokenT<std::vector<std::uint64_t>>, kSides * kMHHLinks> mhhTokens_;
  std::array<edm::EDGetTokenT<std::vector<std::uint64_t>>, kSides * kGCTSInputLinks> gctsInputTokens_;
  std::array<edm::EDGetTokenT<std::vector<std::uint64_t>>, kSides * kGCTSSideLinks> gctsSideTokens_;
  std::array<edm::EDGetTokenT<std::vector<std::uint64_t>>, kGTLinks> gtTokens_;
  edm::EDGetTokenT<std::vector<reco::GenJet>> genJetToken_;
  edm::EDGetTokenT<std::vector<reco::GenMET>> genMETToken_;

  bool hasExplicitGCTSInputs_;
  bool gctbPresentConfig_;
  unsigned int gctbSourceMaskConfig_;
  double sourceEtLSBConfig_;
  double gtEtLSBConfig_;
  double gtEtaLSBConfig_;
  double gtPhiLSBConfig_;
  std::string datasetConfig_;
  std::string sourceLFNConfig_;
  std::string inputEDMConfig_;
  std::string cmsswReleaseConfig_;
  std::string globalTagConfig_;
  std::string cmsswCommitConfig_;
  std::string analyzerCommitConfig_;
  std::string emulatorConfigurationHashConfig_;
  std::string ntupleConfigurationHashConfig_;

  edm::Service<TFileService> fileService_;
  TTree* tree_;

  unsigned int run_;
  unsigned int lumi_;
  unsigned long long event_;
  unsigned int productPresenceMask_;
  bool gctbPresent_;
  unsigned int gctbSourceMask_;
  bool gctsInputDerived_;
  bool genJetsPresent_;
  bool genMETPresent_;
  float sourceEtLSB_;
  float gtEtLSB_;
  float gtEtaLSB_;
  float gtPhiLSB_;

  std::string dataset_;
  std::string sourceLFN_;
  std::string inputEDM_;
  std::string cmsswRelease_;
  std::string globalTag_;
  std::string cmsswCommit_;
  std::string analyzerCommit_;
  std::string emulatorConfigurationHash_;
  std::string ntupleConfigurationHash_;

  unsigned long long hfWords_[kSides][kHFLinks][kWords];
  unsigned long long dmxWords_[kSides][kDMXLinks][kWords];
  unsigned long long mhhWords_[kSides][kMHHLinks][kWords];
  unsigned long long gctsInputWords_[kSides][kGCTSInputLinks][kWords];
  unsigned long long gctsSideWords_[kSides][kGCTSSideLinks][kWords];
  unsigned long long gtWords_[kGTLinks][kWords];

  unsigned long long mhhObjectRaw48_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  bool mhhObjectValid_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int mhhObjectHwEt_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int mhhObjectHwEta_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int mhhObjectHwPhi_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int mhhObjectAux_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int mhhObjectSourceFlag_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float mhhObjectPtGeV_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float mhhObjectPhiRad_[kObjectTypes][kSides][kObjectsPerTypeAndSide];

  unsigned long long gctsObjectRaw48_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  bool gctsObjectValid_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gctsObjectHwEt_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gctsObjectHwEta_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gctsObjectHwPhi_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gctsObjectAux_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gctsObjectIsBarrel_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float gctsObjectPtGeV_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float gctsObjectEta_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float gctsObjectPhiRad_[kObjectTypes][kSides][kObjectsPerTypeAndSide];

  int mhhSumEx_[kSides];
  int mhhSumEy_[kSides];
  int mhhSumHtx_[kSides];
  int mhhSumHty_[kSides];
  int mhhSumEt_[kSides];
  int mhhSumHt_[kSides];
  int mhhSumNObj_[kSides];
  float mhhSumExGeV_[kSides];
  float mhhSumEyGeV_[kSides];
  float mhhSumHtxGeV_[kSides];
  float mhhSumHtyGeV_[kSides];
  float mhhSumEtGeV_[kSides];
  float mhhSumHtGeV_[kSides];

  int gctsSumEx_[kSides];
  int gctsSumEy_[kSides];
  int gctsSumHtx_[kSides];
  int gctsSumHty_[kSides];
  int gctsSumEt_[kSides];
  int gctsSumHt_[kSides];
  int gctsSumNObj_[kSides];
  float gctsSumExGeV_[kSides];
  float gctsSumEyGeV_[kSides];
  float gctsSumHtxGeV_[kSides];
  float gctsSumHtyGeV_[kSides];
  float gctsSumEtGeV_[kSides];
  float gctsSumHtGeV_[kSides];

  bool gtObjectValid_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gtObjectHwPt_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gtObjectHwEta_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gtObjectHwPhi_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gtObjectSpare_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  int gtObjectPtClusterSeed_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float gtObjectPtGeV_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float gtObjectEta_[kObjectTypes][kSides][kObjectsPerTypeAndSide];
  float gtObjectPhiRad_[kObjectTypes][kSides][kObjectsPerTypeAndSide];

  bool gtMETValid_;
  bool gtMHTValid_;
  bool gtNObjValid_;
  int gtMETHwPt_;
  int gtMETHwPhi_;
  int gtETHw_;
  int gtMHTHwPt_;
  int gtMHTHwPhi_;
  int gtHTHw_;
  int gtNObj_;
  float gtMETGeV_;
  float gtMETPhiRad_;
  float gtETGeV_;
  float gtMHTGeV_;
  float gtMHTPhiRad_;
  float gtHTGeV_;

  std::vector<float> genJetPt_;
  std::vector<float> genJetEta_;
  std::vector<float> genJetPhi_;
  std::vector<float> genJetMass_;
  std::vector<float> genMETPt_;
  std::vector<float> genMETPhi_;
  std::vector<float> genMETSumEt_;
};

MHHGCTSTrainingNtuplizer::MHHGCTSTrainingNtuplizer(const edm::ParameterSet& config)
    : hasExplicitGCTSInputs_(false),
      gctbPresentConfig_(config.getParameter<bool>("gctbPresent")),
      gctbSourceMaskConfig_(config.getParameter<unsigned int>("gctbSourceMask")),
      sourceEtLSBConfig_(config.getParameter<double>("sourceEtLSB")),
      gtEtLSBConfig_(config.getParameter<double>("gtEtLSB")),
      gtEtaLSBConfig_(config.getParameter<double>("gtEtaLSB")),
      gtPhiLSBConfig_(config.getParameter<double>("gtPhiLSB")),
      datasetConfig_(config.getParameter<std::string>("dataset")),
      sourceLFNConfig_(config.getParameter<std::string>("sourceLFN")),
      inputEDMConfig_(config.getParameter<std::string>("inputEDM")),
      cmsswReleaseConfig_(config.getParameter<std::string>("cmsswRelease")),
      globalTagConfig_(config.getParameter<std::string>("globalTag")),
      cmsswCommitConfig_(config.getParameter<std::string>("cmsswCommit")),
      analyzerCommitConfig_(config.getParameter<std::string>("analyzerCommit")),
      emulatorConfigurationHashConfig_(config.getParameter<std::string>("emulatorConfigurationHash")),
      ntupleConfigurationHashConfig_(config.getParameter<std::string>("ntupleConfigurationHash")),
      tree_(nullptr) {
  usesResource("TFileService");

  const auto hfLinks = config.getParameter<std::vector<edm::InputTag>>("hfLinks");
  const auto dmxLinks = config.getParameter<std::vector<edm::InputTag>>("dmxLinks");
  const auto mhhLinks = config.getParameter<std::vector<edm::InputTag>>("mhhLinks");
  const auto gctsInputLinks = config.getParameter<std::vector<edm::InputTag>>("gctsInputLinks");
  const auto gctsSideLinks = config.getParameter<std::vector<edm::InputTag>>("gctsSideLinks");
  const auto gtLinks = config.getParameter<std::vector<edm::InputTag>>("gtLinks");

  if (hfLinks.size() != hfTokens_.size() || dmxLinks.size() != dmxTokens_.size() ||
      mhhLinks.size() != mhhTokens_.size() || gctsSideLinks.size() != gctsSideTokens_.size() ||
      gtLinks.size() != gtTokens_.size()) {
    throw cms::Exception("MHHGCTSTrainingNtuplizer")
        << "Expected 6 HF, 32 DMX, 12 MHH, 6 GCTS-side, and 6 GT link tags; got " << hfLinks.size() << ", "
        << dmxLinks.size() << ", " << mhhLinks.size() << ", " << gctsSideLinks.size() << ", and "
        << gtLinks.size();
  }
  if (!gctsInputLinks.empty() && gctsInputLinks.size() != gctsInputTokens_.size()) {
    throw cms::Exception("MHHGCTSTrainingNtuplizer") << "gctsInputLinks must be empty or contain 24 tags";
  }
  hasExplicitGCTSInputs_ = !gctsInputLinks.empty();
  if (gctbPresentConfig_ && !hasExplicitGCTSInputs_) {
    throw cms::Exception("MHHGCTSTrainingNtuplizer")
        << "gctbPresent=True requires all 24 explicit GCTS input links";
  }

  for (unsigned int i = 0; i < hfTokens_.size(); ++i)
    hfTokens_[i] = consumes<std::vector<std::uint64_t>>(hfLinks[i]);
  for (unsigned int i = 0; i < dmxTokens_.size(); ++i)
    dmxTokens_[i] = consumes<std::vector<std::uint64_t>>(dmxLinks[i]);
  for (unsigned int i = 0; i < mhhTokens_.size(); ++i)
    mhhTokens_[i] = consumes<std::vector<std::uint64_t>>(mhhLinks[i]);
  if (hasExplicitGCTSInputs_) {
    for (unsigned int i = 0; i < gctsInputTokens_.size(); ++i)
      gctsInputTokens_[i] = consumes<std::vector<std::uint64_t>>(gctsInputLinks[i]);
  }
  for (unsigned int i = 0; i < gctsSideTokens_.size(); ++i)
    gctsSideTokens_[i] = consumes<std::vector<std::uint64_t>>(gctsSideLinks[i]);
  for (unsigned int i = 0; i < gtTokens_.size(); ++i)
    gtTokens_[i] = consumes<std::vector<std::uint64_t>>(gtLinks[i]);

  genJetToken_ = consumes<std::vector<reco::GenJet>>(config.getParameter<edm::InputTag>("genJets"));
  genMETToken_ = consumes<std::vector<reco::GenMET>>(config.getParameter<edm::InputTag>("genMET"));

  tree_ = fileService_->make<TTree>("trainingTree", "MHH and GCTS raw and decoded training data");
  tree_->Branch("run", &run_, "run/i");
  tree_->Branch("lumi", &lumi_, "lumi/i");
  tree_->Branch("event", &event_, "event/l");
  tree_->Branch("productPresenceMask", &productPresenceMask_, "productPresenceMask/i");
  tree_->Branch("gctbPresent", &gctbPresent_, "gctbPresent/O");
  tree_->Branch("gctbSourceMask", &gctbSourceMask_, "gctbSourceMask/i");
  tree_->Branch("gctsInputDerived", &gctsInputDerived_, "gctsInputDerived/O");
  tree_->Branch("genJetsPresent", &genJetsPresent_, "genJetsPresent/O");
  tree_->Branch("genMETPresent", &genMETPresent_, "genMETPresent/O");
  tree_->Branch("sourceEtLSB", &sourceEtLSB_, "sourceEtLSB/F");
  tree_->Branch("gtEtLSB", &gtEtLSB_, "gtEtLSB/F");
  tree_->Branch("gtEtaLSB", &gtEtaLSB_, "gtEtaLSB/F");
  tree_->Branch("gtPhiLSB", &gtPhiLSB_, "gtPhiLSB/F");
  tree_->Branch("dataset", &dataset_);
  tree_->Branch("sourceLFN", &sourceLFN_);
  tree_->Branch("inputEDM", &inputEDM_);
  tree_->Branch("cmsswRelease", &cmsswRelease_);
  tree_->Branch("globalTag", &globalTag_);
  tree_->Branch("cmsswCommit", &cmsswCommit_);
  tree_->Branch("analyzerCommit", &analyzerCommit_);
  tree_->Branch("emulatorConfigurationHash", &emulatorConfigurationHash_);
  tree_->Branch("ntupleConfigurationHash", &ntupleConfigurationHash_);

  tree_->Branch("hf_words", hfWords_, "hf_words[2][3][9]/l");
  tree_->Branch("dmx_words", dmxWords_, "dmx_words[2][16][9]/l");
  tree_->Branch("mhh_words", mhhWords_, "mhh_words[2][6][9]/l");
  tree_->Branch("gcts_input_words", gctsInputWords_, "gcts_input_words[2][12][9]/l");
  tree_->Branch("gcts_sum_words", gctsSideWords_, "gcts_sum_words[2][3][9]/l");
  tree_->Branch("gcts_gt_words", gtWords_, "gcts_gt_words[6][9]/l");

  tree_->Branch("mhhObjectRaw48", mhhObjectRaw48_, "mhhObjectRaw48[4][2][6]/l");
  tree_->Branch("mhhObjectValid", mhhObjectValid_, "mhhObjectValid[4][2][6]/O");
  tree_->Branch("mhhObjectHwEt", mhhObjectHwEt_, "mhhObjectHwEt[4][2][6]/I");
  tree_->Branch("mhhObjectHwEta", mhhObjectHwEta_, "mhhObjectHwEta[4][2][6]/I");
  tree_->Branch("mhhObjectHwPhi", mhhObjectHwPhi_, "mhhObjectHwPhi[4][2][6]/I");
  tree_->Branch("mhhObjectAux", mhhObjectAux_, "mhhObjectAux[4][2][6]/I");
  tree_->Branch("mhhObjectSourceFlag", mhhObjectSourceFlag_, "mhhObjectSourceFlag[4][2][6]/I");
  tree_->Branch("mhhObjectPtGeV", mhhObjectPtGeV_, "mhhObjectPtGeV[4][2][6]/F");
  tree_->Branch("mhhObjectPhiRad", mhhObjectPhiRad_, "mhhObjectPhiRad[4][2][6]/F");

  tree_->Branch("gctsObjectRaw48", gctsObjectRaw48_, "gctsObjectRaw48[4][2][6]/l");
  tree_->Branch("gctsObjectValid", gctsObjectValid_, "gctsObjectValid[4][2][6]/O");
  tree_->Branch("gctsObjectHwEt", gctsObjectHwEt_, "gctsObjectHwEt[4][2][6]/I");
  tree_->Branch("gctsObjectHwEta", gctsObjectHwEta_, "gctsObjectHwEta[4][2][6]/I");
  tree_->Branch("gctsObjectHwPhi", gctsObjectHwPhi_, "gctsObjectHwPhi[4][2][6]/I");
  tree_->Branch("gctsObjectAux", gctsObjectAux_, "gctsObjectAux[4][2][6]/I");
  tree_->Branch("gctsObjectIsBarrel", gctsObjectIsBarrel_, "gctsObjectIsBarrel[4][2][6]/I");
  tree_->Branch("gctsObjectPtGeV", gctsObjectPtGeV_, "gctsObjectPtGeV[4][2][6]/F");
  tree_->Branch("gctsObjectEta", gctsObjectEta_, "gctsObjectEta[4][2][6]/F");
  tree_->Branch("gctsObjectPhiRad", gctsObjectPhiRad_, "gctsObjectPhiRad[4][2][6]/F");

  tree_->Branch("mhhSumEx", mhhSumEx_, "mhhSumEx[2]/I");
  tree_->Branch("mhhSumEy", mhhSumEy_, "mhhSumEy[2]/I");
  tree_->Branch("mhhSumHtx", mhhSumHtx_, "mhhSumHtx[2]/I");
  tree_->Branch("mhhSumHty", mhhSumHty_, "mhhSumHty[2]/I");
  tree_->Branch("mhhSumEt", mhhSumEt_, "mhhSumEt[2]/I");
  tree_->Branch("mhhSumHt", mhhSumHt_, "mhhSumHt[2]/I");
  tree_->Branch("mhhSumNObj", mhhSumNObj_, "mhhSumNObj[2]/I");
  tree_->Branch("mhhSumExGeV", mhhSumExGeV_, "mhhSumExGeV[2]/F");
  tree_->Branch("mhhSumEyGeV", mhhSumEyGeV_, "mhhSumEyGeV[2]/F");
  tree_->Branch("mhhSumHtxGeV", mhhSumHtxGeV_, "mhhSumHtxGeV[2]/F");
  tree_->Branch("mhhSumHtyGeV", mhhSumHtyGeV_, "mhhSumHtyGeV[2]/F");
  tree_->Branch("mhhSumEtGeV", mhhSumEtGeV_, "mhhSumEtGeV[2]/F");
  tree_->Branch("mhhSumHtGeV", mhhSumHtGeV_, "mhhSumHtGeV[2]/F");

  tree_->Branch("gctsSumEx", gctsSumEx_, "gctsSumEx[2]/I");
  tree_->Branch("gctsSumEy", gctsSumEy_, "gctsSumEy[2]/I");
  tree_->Branch("gctsSumHtx", gctsSumHtx_, "gctsSumHtx[2]/I");
  tree_->Branch("gctsSumHty", gctsSumHty_, "gctsSumHty[2]/I");
  tree_->Branch("gctsSumEt", gctsSumEt_, "gctsSumEt[2]/I");
  tree_->Branch("gctsSumHt", gctsSumHt_, "gctsSumHt[2]/I");
  tree_->Branch("gctsSumNObj", gctsSumNObj_, "gctsSumNObj[2]/I");
  tree_->Branch("gctsSumExGeV", gctsSumExGeV_, "gctsSumExGeV[2]/F");
  tree_->Branch("gctsSumEyGeV", gctsSumEyGeV_, "gctsSumEyGeV[2]/F");
  tree_->Branch("gctsSumHtxGeV", gctsSumHtxGeV_, "gctsSumHtxGeV[2]/F");
  tree_->Branch("gctsSumHtyGeV", gctsSumHtyGeV_, "gctsSumHtyGeV[2]/F");
  tree_->Branch("gctsSumEtGeV", gctsSumEtGeV_, "gctsSumEtGeV[2]/F");
  tree_->Branch("gctsSumHtGeV", gctsSumHtGeV_, "gctsSumHtGeV[2]/F");

  tree_->Branch("gtObjectValid", gtObjectValid_, "gtObjectValid[4][2][6]/O");
  tree_->Branch("gtObjectHwPt", gtObjectHwPt_, "gtObjectHwPt[4][2][6]/I");
  tree_->Branch("gtObjectHwEta", gtObjectHwEta_, "gtObjectHwEta[4][2][6]/I");
  tree_->Branch("gtObjectHwPhi", gtObjectHwPhi_, "gtObjectHwPhi[4][2][6]/I");
  tree_->Branch("gtObjectSpare", gtObjectSpare_, "gtObjectSpare[4][2][6]/I");
  tree_->Branch(
      "gtObjectPtClusterSeed", gtObjectPtClusterSeed_, "gtObjectPtClusterSeed[4][2][6]/I");
  tree_->Branch("gtObjectPtGeV", gtObjectPtGeV_, "gtObjectPtGeV[4][2][6]/F");
  tree_->Branch("gtObjectEta", gtObjectEta_, "gtObjectEta[4][2][6]/F");
  tree_->Branch("gtObjectPhiRad", gtObjectPhiRad_, "gtObjectPhiRad[4][2][6]/F");

  tree_->Branch("gtMETValid", &gtMETValid_, "gtMETValid/O");
  tree_->Branch("gtMHTValid", &gtMHTValid_, "gtMHTValid/O");
  tree_->Branch("gtNObjValid", &gtNObjValid_, "gtNObjValid/O");
  tree_->Branch("gtMETHwPt", &gtMETHwPt_, "gtMETHwPt/I");
  tree_->Branch("gtMETHwPhi", &gtMETHwPhi_, "gtMETHwPhi/I");
  tree_->Branch("gtETHw", &gtETHw_, "gtETHw/I");
  tree_->Branch("gtMHTHwPt", &gtMHTHwPt_, "gtMHTHwPt/I");
  tree_->Branch("gtMHTHwPhi", &gtMHTHwPhi_, "gtMHTHwPhi/I");
  tree_->Branch("gtHTHw", &gtHTHw_, "gtHTHw/I");
  tree_->Branch("gtNObj", &gtNObj_, "gtNObj/I");
  tree_->Branch("gtMETGeV", &gtMETGeV_, "gtMETGeV/F");
  tree_->Branch("gtMETPhiRad", &gtMETPhiRad_, "gtMETPhiRad/F");
  tree_->Branch("gtETGeV", &gtETGeV_, "gtETGeV/F");
  tree_->Branch("gtMHTGeV", &gtMHTGeV_, "gtMHTGeV/F");
  tree_->Branch("gtMHTPhiRad", &gtMHTPhiRad_, "gtMHTPhiRad/F");
  tree_->Branch("gtHTGeV", &gtHTGeV_, "gtHTGeV/F");

  tree_->Branch("genJetPt", &genJetPt_);
  tree_->Branch("genJetEta", &genJetEta_);
  tree_->Branch("genJetPhi", &genJetPhi_);
  tree_->Branch("genJetMass", &genJetMass_);
  tree_->Branch("genMETPt", &genMETPt_);
  tree_->Branch("genMETPhi", &genMETPhi_);
  tree_->Branch("genMETSumEt", &genMETSumEt_);
}

void MHHGCTSTrainingNtuplizer::readLink(
    const edm::Event& event,
    const edm::EDGetTokenT<std::vector<std::uint64_t>>& token,
    unsigned long long destination[kWords],
    const std::string& label) const {
  edm::Handle<std::vector<std::uint64_t>> handle;
  event.getByToken(token, handle);
  if (!handle.isValid() || handle->size() != kWords) {
    throw cms::Exception("MHHGCTSTrainingNtuplizer")
        << label << " is missing or has " << (handle.isValid() ? handle->size() : 0) << " words; expected 9";
  }
  for (unsigned int word = 0; word < kWords; ++word)
    destination[word] = static_cast<unsigned long long>((*handle)[word]);
}

void MHHGCTSTrainingNtuplizer::resetEvent() {
  productPresenceMask_ = 0;
  gctbPresent_ = gctbPresentConfig_;
  gctbSourceMask_ = gctbSourceMaskConfig_;
  gctsInputDerived_ = !hasExplicitGCTSInputs_;
  genJetsPresent_ = false;
  genMETPresent_ = false;
  sourceEtLSB_ = sourceEtLSBConfig_;
  gtEtLSB_ = gtEtLSBConfig_;
  gtEtaLSB_ = gtEtaLSBConfig_;
  gtPhiLSB_ = gtPhiLSBConfig_;
  dataset_ = datasetConfig_;
  sourceLFN_ = sourceLFNConfig_;
  inputEDM_ = inputEDMConfig_;
  cmsswRelease_ = cmsswReleaseConfig_;
  globalTag_ = globalTagConfig_;
  cmsswCommit_ = cmsswCommitConfig_;
  analyzerCommit_ = analyzerCommitConfig_;
  emulatorConfigurationHash_ = emulatorConfigurationHashConfig_;
  ntupleConfigurationHash_ = ntupleConfigurationHashConfig_;

  std::memset(hfWords_, 0, sizeof(hfWords_));
  std::memset(dmxWords_, 0, sizeof(dmxWords_));
  std::memset(mhhWords_, 0, sizeof(mhhWords_));
  std::memset(gctsInputWords_, 0, sizeof(gctsInputWords_));
  std::memset(gctsSideWords_, 0, sizeof(gctsSideWords_));
  std::memset(gtWords_, 0, sizeof(gtWords_));
  std::memset(mhhObjectRaw48_, 0, sizeof(mhhObjectRaw48_));
  std::memset(mhhObjectValid_, 0, sizeof(mhhObjectValid_));
  std::memset(mhhObjectHwEt_, 0, sizeof(mhhObjectHwEt_));
  std::memset(mhhObjectHwEta_, 0, sizeof(mhhObjectHwEta_));
  std::memset(mhhObjectHwPhi_, 0, sizeof(mhhObjectHwPhi_));
  std::memset(mhhObjectAux_, 0, sizeof(mhhObjectAux_));
  std::memset(mhhObjectSourceFlag_, 0, sizeof(mhhObjectSourceFlag_));
  std::memset(mhhObjectPtGeV_, 0, sizeof(mhhObjectPtGeV_));
  std::memset(mhhObjectPhiRad_, 0, sizeof(mhhObjectPhiRad_));
  std::memset(gctsObjectRaw48_, 0, sizeof(gctsObjectRaw48_));
  std::memset(gctsObjectValid_, 0, sizeof(gctsObjectValid_));
  std::memset(gctsObjectHwEt_, 0, sizeof(gctsObjectHwEt_));
  std::memset(gctsObjectHwEta_, 0, sizeof(gctsObjectHwEta_));
  std::memset(gctsObjectHwPhi_, 0, sizeof(gctsObjectHwPhi_));
  std::memset(gctsObjectAux_, 0, sizeof(gctsObjectAux_));
  std::memset(gctsObjectIsBarrel_, 0, sizeof(gctsObjectIsBarrel_));
  std::memset(gctsObjectPtGeV_, 0, sizeof(gctsObjectPtGeV_));
  std::memset(gctsObjectEta_, 0, sizeof(gctsObjectEta_));
  std::memset(gctsObjectPhiRad_, 0, sizeof(gctsObjectPhiRad_));
  std::memset(mhhSumEx_, 0, sizeof(mhhSumEx_));
  std::memset(mhhSumEy_, 0, sizeof(mhhSumEy_));
  std::memset(mhhSumHtx_, 0, sizeof(mhhSumHtx_));
  std::memset(mhhSumHty_, 0, sizeof(mhhSumHty_));
  std::memset(mhhSumEt_, 0, sizeof(mhhSumEt_));
  std::memset(mhhSumHt_, 0, sizeof(mhhSumHt_));
  std::memset(mhhSumNObj_, 0, sizeof(mhhSumNObj_));
  std::memset(mhhSumExGeV_, 0, sizeof(mhhSumExGeV_));
  std::memset(mhhSumEyGeV_, 0, sizeof(mhhSumEyGeV_));
  std::memset(mhhSumHtxGeV_, 0, sizeof(mhhSumHtxGeV_));
  std::memset(mhhSumHtyGeV_, 0, sizeof(mhhSumHtyGeV_));
  std::memset(mhhSumEtGeV_, 0, sizeof(mhhSumEtGeV_));
  std::memset(mhhSumHtGeV_, 0, sizeof(mhhSumHtGeV_));
  std::memset(gctsSumEx_, 0, sizeof(gctsSumEx_));
  std::memset(gctsSumEy_, 0, sizeof(gctsSumEy_));
  std::memset(gctsSumHtx_, 0, sizeof(gctsSumHtx_));
  std::memset(gctsSumHty_, 0, sizeof(gctsSumHty_));
  std::memset(gctsSumEt_, 0, sizeof(gctsSumEt_));
  std::memset(gctsSumHt_, 0, sizeof(gctsSumHt_));
  std::memset(gctsSumNObj_, 0, sizeof(gctsSumNObj_));
  std::memset(gctsSumExGeV_, 0, sizeof(gctsSumExGeV_));
  std::memset(gctsSumEyGeV_, 0, sizeof(gctsSumEyGeV_));
  std::memset(gctsSumHtxGeV_, 0, sizeof(gctsSumHtxGeV_));
  std::memset(gctsSumHtyGeV_, 0, sizeof(gctsSumHtyGeV_));
  std::memset(gctsSumEtGeV_, 0, sizeof(gctsSumEtGeV_));
  std::memset(gctsSumHtGeV_, 0, sizeof(gctsSumHtGeV_));
  std::memset(gtObjectValid_, 0, sizeof(gtObjectValid_));
  std::memset(gtObjectHwPt_, 0, sizeof(gtObjectHwPt_));
  std::memset(gtObjectHwEta_, 0, sizeof(gtObjectHwEta_));
  std::memset(gtObjectHwPhi_, 0, sizeof(gtObjectHwPhi_));
  std::memset(gtObjectSpare_, 0, sizeof(gtObjectSpare_));
  std::memset(gtObjectPtClusterSeed_, 0, sizeof(gtObjectPtClusterSeed_));
  std::memset(gtObjectPtGeV_, 0, sizeof(gtObjectPtGeV_));
  std::memset(gtObjectEta_, 0, sizeof(gtObjectEta_));
  std::memset(gtObjectPhiRad_, 0, sizeof(gtObjectPhiRad_));
  gtMETValid_ = gtMHTValid_ = gtNObjValid_ = false;
  gtMETHwPt_ = gtMETHwPhi_ = gtETHw_ = 0;
  gtMHTHwPt_ = gtMHTHwPhi_ = gtHTHw_ = gtNObj_ = 0;
  gtMETGeV_ = gtMETPhiRad_ = gtETGeV_ = 0;
  gtMHTGeV_ = gtMHTPhiRad_ = gtHTGeV_ = 0;
  genJetPt_.clear();
  genJetEta_.clear();
  genJetPhi_.clear();
  genJetMass_.clear();
  genMETPt_.clear();
  genMETPhi_.clear();
  genMETSumEt_.clear();
}

void MHHGCTSTrainingNtuplizer::buildGCTSInputs(const edm::Event& event) {
  if (hasExplicitGCTSInputs_) {
    for (unsigned int side = 0; side < kSides; ++side) {
      for (unsigned int link = 0; link < kGCTSInputLinks; ++link) {
        readLink(event,
                 gctsInputTokens_[side * kGCTSInputLinks + link],
                 gctsInputWords_[side][link],
                 "GCTS input link");
      }
    }
    return;
  }

  for (unsigned int side = 0; side < kSides; ++side) {
    for (unsigned int link = 0; link < 3; ++link) {
      for (unsigned int word = 0; word < kWords; ++word)
        gctsInputWords_[side][link][word] = mhhWords_[side][link][word];
    }
  }
}

void MHHGCTSTrainingNtuplizer::decodeMHHObjects() {
  for (unsigned int side = 0; side < kSides; ++side) {
    for (unsigned int slot = 0; slot < 12; ++slot) {
      const unsigned int type = slot < 6 ? kEG : kEGIso;
      const unsigned int rank = slot % 6;
      const std::uint64_t raw = extractBits(mhhWords_[side][0], slot * 48, 48);
      mhhObjectRaw48_[type][side][rank] = raw;
      mhhObjectHwEt_[type][side][rank] = raw & 0xFFF;
      mhhObjectHwEta_[type][side][rank] = (raw >> 12) & 0x7F;
      mhhObjectHwPhi_[type][side][rank] = (raw >> 19) & 0x1FF;
      mhhObjectAux_[type][side][rank] = 0;
      // In the MHH packet this bit identifies the adapter source: 0=HF, 1=HGCAL.
      // It only becomes a barrel/endcap classification after GCTS merges its inputs.
      mhhObjectSourceFlag_[type][side][rank] = (raw >> 47) & 1;
      mhhObjectValid_[type][side][rank] = mhhObjectHwEt_[type][side][rank] != 0;
      mhhObjectPtGeV_[type][side][rank] = mhhObjectHwEt_[type][side][rank] * sourceEtLSB_;
      mhhObjectPhiRad_[type][side][rank] = mhhObjectHwPhi_[type][side][rank] * (2.0 * kPi / 360.0);
    }
    for (unsigned int slot = 0; slot < 12; ++slot) {
      const unsigned int type = slot < 6 ? kJet : kTau;
      const unsigned int rank = slot % 6;
      const std::uint64_t raw = extractBits(mhhWords_[side][1], slot * 48, 48);
      mhhObjectRaw48_[type][side][rank] = raw;
      mhhObjectHwEt_[type][side][rank] = raw & 0xFFF;
      mhhObjectHwEta_[type][side][rank] = (raw >> 12) & 0x3F;
      mhhObjectHwPhi_[type][side][rank] = (raw >> 18) & 0x1FF;
      mhhObjectAux_[type][side][rank] = (raw >> 27) & 0xF;
      mhhObjectSourceFlag_[type][side][rank] = (raw >> 47) & 1;
      mhhObjectValid_[type][side][rank] = mhhObjectHwEt_[type][side][rank] != 0;
      mhhObjectPtGeV_[type][side][rank] = mhhObjectHwEt_[type][side][rank] * sourceEtLSB_;
      mhhObjectPhiRad_[type][side][rank] = mhhObjectHwPhi_[type][side][rank] * (2.0 * kPi / 24.0);
    }
  }
}

void MHHGCTSTrainingNtuplizer::decodeMHHSums() {
  for (unsigned int side = 0; side < kSides; ++side) {
    const std::uint64_t word0 = mhhWords_[side][2][0];
    const std::uint64_t word1 = mhhWords_[side][2][1];
    mhhSumEx_[side] = signExtend(word0, 12);
    mhhSumEy_[side] = signExtend(word0 >> 12, 12);
    mhhSumHtx_[side] = signExtend(word0 >> 24, 12);
    mhhSumHty_[side] = signExtend(word0 >> 36, 12);
    mhhSumEt_[side] = word1 & 0xFFF;
    mhhSumHt_[side] = (word1 >> 12) & 0xFFF;
    mhhSumNObj_[side] = (word1 >> 24) & 0xFFF;
    mhhSumExGeV_[side] = mhhSumEx_[side] * sourceEtLSB_;
    mhhSumEyGeV_[side] = mhhSumEy_[side] * sourceEtLSB_;
    mhhSumHtxGeV_[side] = mhhSumHtx_[side] * sourceEtLSB_;
    mhhSumHtyGeV_[side] = mhhSumHty_[side] * sourceEtLSB_;
    mhhSumEtGeV_[side] = mhhSumEt_[side] * sourceEtLSB_;
    mhhSumHtGeV_[side] = mhhSumHt_[side] * sourceEtLSB_;
  }
}

void MHHGCTSTrainingNtuplizer::decodeGCTSObjects() {
  for (unsigned int side = 0; side < kSides; ++side) {
    for (unsigned int slot = 0; slot < 12; ++slot) {
      const unsigned int type = slot < 6 ? kEG : kEGIso;
      const unsigned int rank = slot % 6;
      const std::uint64_t raw = extractBits(gctsSideWords_[side][0], slot * 48, 48);
      gctsObjectRaw48_[type][side][rank] = raw;
      gctsObjectHwEt_[type][side][rank] = raw & 0xFFF;
      gctsObjectHwEta_[type][side][rank] = ((raw >> 12) & 0x7F) | (((raw >> 28) & 1) << 7);
      gctsObjectHwPhi_[type][side][rank] = (raw >> 19) & 0x1FF;
      gctsObjectAux_[type][side][rank] = 0;
      gctsObjectIsBarrel_[type][side][rank] = (raw >> 47) & 1;
      gctsObjectValid_[type][side][rank] = gctsObjectHwEt_[type][side][rank] != 0;
      gctsObjectPtGeV_[type][side][rank] = gctsObjectHwEt_[type][side][rank] * sourceEtLSB_;
      const float etaSign = side == 0 ? 1.0 : -1.0;
      gctsObjectEta_[type][side][rank] = etaSign * gctsObjectHwEta_[type][side][rank] * 23.0 * gtEtaLSB_;
      gctsObjectPhiRad_[type][side][rank] = gctsObjectHwPhi_[type][side][rank] * (2.0 * kPi / 360.0);
    }
    for (unsigned int slot = 0; slot < 12; ++slot) {
      const unsigned int type = slot < 6 ? kJet : kTau;
      const unsigned int rank = slot % 6;
      const std::uint64_t raw = extractBits(gctsSideWords_[side][1], slot * 48, 48);
      gctsObjectRaw48_[type][side][rank] = raw;
      gctsObjectHwEt_[type][side][rank] = raw & 0xFFF;
      gctsObjectHwEta_[type][side][rank] = (raw >> 12) & 0x3F;
      gctsObjectHwPhi_[type][side][rank] = (raw >> 18) & 0x1FF;
      gctsObjectAux_[type][side][rank] = (raw >> 27) & 0xF;
      gctsObjectIsBarrel_[type][side][rank] = (raw >> 47) & 1;
      gctsObjectValid_[type][side][rank] = gctsObjectHwEt_[type][side][rank] != 0;
      gctsObjectPtGeV_[type][side][rank] = gctsObjectHwEt_[type][side][rank] * sourceEtLSB_;
      const float etaSign = side == 0 ? 1.0 : -1.0;
      gctsObjectEta_[type][side][rank] = etaSign * gctsObjectHwEta_[type][side][rank] * 23.0 * gtEtaLSB_;
      gctsObjectPhiRad_[type][side][rank] = gctsObjectHwPhi_[type][side][rank] * (2.0 * kPi / 24.0);
    }
  }
}

void MHHGCTSTrainingNtuplizer::decodeGCTSSideSums() {
  for (unsigned int side = 0; side < kSides; ++side) {
    const std::uint64_t word0 = gctsSideWords_[side][2][0];
    const std::uint64_t word1 = gctsSideWords_[side][2][1];
    gctsSumEx_[side] = signExtend(word0, 16);
    gctsSumEy_[side] = signExtend(word0 >> 16, 16);
    gctsSumHtx_[side] = signExtend(word0 >> 32, 16);
    gctsSumHty_[side] = signExtend(word0 >> 48, 16);
    gctsSumEt_[side] = word1 & 0xFFFF;
    gctsSumHt_[side] = (word1 >> 16) & 0xFFFF;
    gctsSumNObj_[side] = (word1 >> 32) & 0xFFFF;
    gctsSumExGeV_[side] = gctsSumEx_[side] * sourceEtLSB_;
    gctsSumEyGeV_[side] = gctsSumEy_[side] * sourceEtLSB_;
    gctsSumHtxGeV_[side] = gctsSumHtx_[side] * sourceEtLSB_;
    gctsSumHtyGeV_[side] = gctsSumHty_[side] * sourceEtLSB_;
    gctsSumEtGeV_[side] = gctsSumEt_[side] * sourceEtLSB_;
    gctsSumHtGeV_[side] = gctsSumHt_[side] * sourceEtLSB_;
  }
}

void MHHGCTSTrainingNtuplizer::decodeGTObject(
    unsigned int type, unsigned int side, unsigned int rank, std::uint64_t word) {
  gtObjectValid_[type][side][rank] = word & 1;
  gtObjectHwPt_[type][side][rank] = (word >> 1) & 0xFFFF;
  gtObjectHwPhi_[type][side][rank] = signExtend(word >> 17, 13);
  // The current TO_GT firmware stores an unsigned eta magnitude.  The eta
  // sign is implicit in the positive/negative packet slot, not in bit 43.
  gtObjectHwEta_[type][side][rank] = (word >> 30) & 0x3FFF;
  gtObjectSpare_[type][side][rank] = (word >> 44) & 0xFFFFF;
  gtObjectPtClusterSeed_[type][side][rank] = gtObjectSpare_[type][side][rank] & 0xF;
  gtObjectPtGeV_[type][side][rank] = gtObjectHwPt_[type][side][rank] * gtEtLSB_;
  const float etaSign = side == 0 ? 1.0f : -1.0f;
  gtObjectEta_[type][side][rank] = etaSign * gtObjectHwEta_[type][side][rank] * gtEtaLSB_;
  gtObjectPhiRad_[type][side][rank] = gtObjectHwPhi_[type][side][rank] * gtPhiLSB_;
}

void MHHGCTSTrainingNtuplizer::decodeGTObjects() {
  for (unsigned int rank = 0; rank < 6; ++rank)
    decodeGTObject(kEG, 0, rank, gtWords_[0][rank]);
  for (unsigned int rank = 0; rank < 3; ++rank)
    decodeGTObject(kEG, 1, rank, gtWords_[0][rank + 6]);
  for (unsigned int rank = 3; rank < 6; ++rank)
    decodeGTObject(kEG, 1, rank, gtWords_[1][rank - 3]);
  for (unsigned int rank = 0; rank < 6; ++rank)
    decodeGTObject(kEGIso, 0, rank, gtWords_[1][rank + 3]);
  for (unsigned int rank = 0; rank < 6; ++rank)
    decodeGTObject(kEGIso, 1, rank, gtWords_[2][rank]);
  for (unsigned int rank = 0; rank < 3; ++rank)
    decodeGTObject(kJet, 0, rank, gtWords_[2][rank + 6]);
  for (unsigned int rank = 3; rank < 6; ++rank)
    decodeGTObject(kJet, 0, rank, gtWords_[3][rank - 3]);
  for (unsigned int rank = 0; rank < 6; ++rank)
    decodeGTObject(kJet, 1, rank, gtWords_[3][rank + 3]);
  for (unsigned int rank = 0; rank < 6; ++rank)
    decodeGTObject(kTau, 0, rank, gtWords_[4][rank]);
  for (unsigned int rank = 0; rank < 3; ++rank)
    decodeGTObject(kTau, 1, rank, gtWords_[4][rank + 6]);
  for (unsigned int rank = 3; rank < 6; ++rank)
    decodeGTObject(kTau, 1, rank, gtWords_[5][rank - 3]);
}

void MHHGCTSTrainingNtuplizer::decodeGTSums() {
  const std::uint64_t metWord = gtWords_[5][3];
  const std::uint64_t mhtWord = gtWords_[5][4];
  const std::uint64_t nObjWord = gtWords_[5][5];
  gtMETValid_ = metWord & 1;
  gtMHTValid_ = mhtWord & 1;
  gtNObjValid_ = nObjWord & 1;
  gtMETHwPt_ = (metWord >> 1) & 0xFFFF;
  gtMETHwPhi_ = signExtend(metWord >> 17, 13);
  gtETHw_ = (metWord >> 30) & 0xFFFF;
  gtMHTHwPt_ = (mhtWord >> 1) & 0xFFFF;
  gtMHTHwPhi_ = signExtend(mhtWord >> 17, 13);
  gtHTHw_ = (mhtWord >> 30) & 0xFFFF;
  gtNObj_ = (nObjWord >> 1) & 0xFFFF;
  gtMETGeV_ = gtMETHwPt_ * gtEtLSB_;
  gtMETPhiRad_ = gtMETHwPhi_ * gtPhiLSB_;
  gtETGeV_ = gtETHw_ * gtEtLSB_;
  gtMHTGeV_ = gtMHTHwPt_ * gtEtLSB_;
  gtMHTPhiRad_ = gtMHTHwPhi_ * gtPhiLSB_;
  gtHTGeV_ = gtHTHw_ * gtEtLSB_;
}

void MHHGCTSTrainingNtuplizer::analyze(const edm::Event& event, const edm::EventSetup&) {
  resetEvent();
  run_ = event.id().run();
  lumi_ = event.luminosityBlock();
  event_ = event.id().event();

  for (unsigned int side = 0; side < kSides; ++side) {
    for (unsigned int link = 0; link < kHFLinks; ++link)
      readLink(event, hfTokens_[side * kHFLinks + link], hfWords_[side][link], "HF link");
    productPresenceMask_ |= (1U << side);
    for (unsigned int link = 0; link < kDMXLinks; ++link)
      readLink(event, dmxTokens_[side * kDMXLinks + link], dmxWords_[side][link], "DMX link");
    productPresenceMask_ |= (1U << (2 + side));
    for (unsigned int link = 0; link < kMHHLinks; ++link)
      readLink(event, mhhTokens_[side * kMHHLinks + link], mhhWords_[side][link], "MHH link");
    productPresenceMask_ |= (1U << (4 + side));
    for (unsigned int link = 0; link < kGCTSSideLinks; ++link)
      readLink(event,
               gctsSideTokens_[side * kGCTSSideLinks + link],
               gctsSideWords_[side][link],
               "GCTS side link");
  }
  productPresenceMask_ |= (1U << 6);
  for (unsigned int link = 0; link < kGTLinks; ++link)
    readLink(event, gtTokens_[link], gtWords_[link], "GT link");
  productPresenceMask_ |= (1U << 7);

  buildGCTSInputs(event);
  decodeMHHObjects();
  decodeMHHSums();
  decodeGCTSObjects();
  decodeGCTSSideSums();
  decodeGTObjects();
  decodeGTSums();

  edm::Handle<std::vector<reco::GenJet>> genJets;
  event.getByToken(genJetToken_, genJets);
  if (genJets.isValid()) {
    genJetsPresent_ = true;
    productPresenceMask_ |= (1U << 8);
    for (const auto& jet : *genJets) {
      genJetPt_.push_back(jet.pt());
      genJetEta_.push_back(jet.eta());
      genJetPhi_.push_back(jet.phi());
      genJetMass_.push_back(jet.mass());
    }
  }

  edm::Handle<std::vector<reco::GenMET>> genMET;
  event.getByToken(genMETToken_, genMET);
  if (genMET.isValid()) {
    genMETPresent_ = true;
    productPresenceMask_ |= (1U << 9);
    for (const auto& met : *genMET) {
      genMETPt_.push_back(met.pt());
      genMETPhi_.push_back(met.phi());
      genMETSumEt_.push_back(met.sumEt());
    }
  }

  tree_->Fill();
}

DEFINE_FWK_MODULE(MHHGCTSTrainingNtuplizer);

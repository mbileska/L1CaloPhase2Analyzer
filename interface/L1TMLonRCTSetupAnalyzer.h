#ifndef L1TMLonRCTSetupAnalyzer_H
#define L1TMLonRCTSetupAnalyzer_H


// system include files
#include <memory>
#include <unistd.h>


#include <iostream>
#include <fstream>
#include <vector>

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TH2F.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
//#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

// GCT and RCT data formats
#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctCollections.h"
#include "DataFormats/TauReco/interface/PFTau.h"
#include "DataFormats/TauReco/interface/PFTauDiscriminator.h"
#include "DataFormats/L1Trigger/interface/L1JetParticle.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/L1THGCal/interface/HGCalTower.h"

#include "DataFormats/TauReco/interface/BaseTau.h"
#include "DataFormats/TauReco/interface/PFTauFwd.h"
#include "DataFormats/TauReco/interface/PFTauTagInfo.h"

#include <memory>
#include <math.h>
#include <vector>
#include <list>
#include <TLorentzVector.h>

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CondFormats/L1TObjects/interface/L1CaloHcalScale.h"
#include "CondFormats/DataRecord/interface/L1CaloHcalScaleRcd.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "L1Trigger/L1TGlobal/interface/TriggerMenuFwd.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloRegion.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloCrystalCluster.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloPFCluster.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/Phase2L1CaloJet.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedL1CaloJet.h"
#include "DataFormats/L1THGCal/interface/HGCalTower.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/RCT_output.h"

#ifdef __MAKECINT__
// #pragma extra_include "TLorentzVector.h";
#pragma link C++ class std::vector<TLorentzVector>;
#endif

//
// class declaration
//
using std::vector;

class L1TMLonRCTSetupAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {

 public:
  
  // Constructor
  L1TMLonRCTSetupAnalyzer(const edm::ParameterSet& ps);
  
  // Destructor
  virtual ~L1TMLonRCTSetupAnalyzer();

  edm::Service<TFileService> tfs_;

  // Vectors of vectors, where each element is a per-card vector
  // Cluster branches SLR3
  std::vector<std::vector<int>> *SLR3_cluster_seed_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_et5x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_et2x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_spike = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_satur = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_brems = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR3_cluster_spare = new std::vector<std::vector<int>>;
  // Cluster branches SLR2
  std::vector<std::vector<int>> *SLR2_cluster_seed_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_et5x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_et2x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_spike = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_satur = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_brems = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR2_cluster_spare = new std::vector<std::vector<int>>;
  // Cluster branches SLR1
  std::vector<std::vector<int>> *SLR1_cluster_seed_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_et5x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_et2x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_spike = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_satur = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_brems = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR1_cluster_spare = new std::vector<std::vector<int>>;
  // Cluster branches SLR0
  std::vector<std::vector<int>> *SLR0_cluster_seed_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_energy = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_et5x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_et2x5 = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_spike = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_satur = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_brems = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *SLR0_cluster_spare = new std::vector<std::vector<int>>;
  // ECAL unclustered energy tower branches SLR3
  std::vector<std::vector<int>> *ECALUnclusteredSLR3_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR3_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR3_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR3_tower_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR3_tower_spike = new std::vector<std::vector<int>>;
  // ECAL unclustered energy tower branches SLR2
  std::vector<std::vector<int>> *ECALUnclusteredSLR2_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR2_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR2_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR2_tower_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR2_tower_spike = new std::vector<std::vector<int>>;
  // ECAL unclustered energy tower branches SLR1
  std::vector<std::vector<int>> *ECALUnclusteredSLR1_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR1_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR1_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR1_tower_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR1_tower_spike = new std::vector<std::vector<int>>;
  // ECAL unclustered energy tower branches SLR0
  std::vector<std::vector<int>> *ECALUnclusteredSLR0_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR0_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR0_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR0_tower_timing = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *ECALUnclusteredSLR0_tower_spike = new std::vector<std::vector<int>>;
  // HCAL tower branches link 8
  std::vector<std::vector<int>> *HCAL8_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL8_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL8_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL8_tower_fb = new std::vector<std::vector<int>>;
  // HCAL tower branches link 7
  std::vector<std::vector<int>> *HCAL7_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL7_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL7_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL7_tower_fb = new std::vector<std::vector<int>>;
  // HCAL tower branches link 6
  std::vector<std::vector<int>> *HCAL6_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL6_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL6_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL6_tower_fb = new std::vector<std::vector<int>>;
  // HCAL tower branches link 5
  std::vector<std::vector<int>> *HCAL5_tower_et = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL5_tower_eta = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL5_tower_phi = new std::vector<std::vector<int>>;
  std::vector<std::vector<int>> *HCAL5_tower_fb = new std::vector<std::vector<int>>;

  // Per-card vectors (clusters)
  std::vector<int> *RCT_cluster_seed_energy = new std::vector<int>;
  std::vector<int> *RCT_cluster_energy = new std::vector<int>;
  std::vector<int> *RCT_cluster_eta = new std::vector<int>;
  std::vector<int> *RCT_cluster_phi = new std::vector<int>;
  std::vector<int> *RCT_cluster_et5x5 = new std::vector<int>;
  std::vector<int> *RCT_cluster_et2x5 = new std::vector<int>;
  std::vector<int> *RCT_cluster_timing = new std::vector<int>;
  std::vector<int> *RCT_cluster_spike = new std::vector<int>;
  std::vector<int> *RCT_cluster_satur = new std::vector<int>;
  std::vector<int> *RCT_cluster_brems = new std::vector<int>;
  std::vector<int> *RCT_cluster_spare = new std::vector<int>;

  // Per-card vectors (ECAL towers)
  std::vector<int> *RCT_ECAL_tower_et = new std::vector<int>;
  std::vector<int> *RCT_ECAL_tower_eta = new std::vector<int>;
  std::vector<int> *RCT_ECAL_tower_phi = new std::vector<int>;
  std::vector<int> *RCT_ECAL_tower_timing = new std::vector<int>;
  std::vector<int> *RCT_ECAL_tower_spike = new std::vector<int>;

  // Per-card vectors (HCAL towers)
  std::vector<int> *RCT_HCAL_tower_et = new std::vector<int>;
  std::vector<int> *RCT_HCAL_tower_eta = new std::vector<int>;
  std::vector<int> *RCT_HCAL_tower_phi = new std::vector<int>;
  std::vector<int> *RCT_HCAL_tower_fb = new std::vector<int>;

  TTree* linkTree;

  int run, lumi, event;

 protected:
  // Analyze
  void analyze(const edm::Event& evt, const edm::EventSetup& es);
  
  // BeginJob
  void beginJob(const edm::EventSetup &es);
  
  // EndJob
  void endJob(void);

  
 private:
  // ----------member data ---------------------------

  edm::ESGetToken<CaloTPGTranscoder, CaloTPGRecord> decoderToken_;

  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeometryToken_;
  edm::ESGetToken<HcalTopology, HcalRecNumberingRecord> hbTopologyToken_;

  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> EGammaSLR3Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> EGammaSLR2Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> EGammaSLR1Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> EGammaSLR0Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> ECALUnclusteredSLR3Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> ECALUnclusteredSLR2Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> ECALUnclusteredSLR1Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> ECALUnclusteredSLR0Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> HCAL8Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> HCAL7Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> HCAL6Src_;
  edm::EDGetTokenT<l1tp2::rctOutputLinkCollection> HCAL5Src_;

  std::string folderName_;

};

void getEGammaClusters(
  ap_uint<576> Data,
  int SLR,
  std::vector<int>* RCT_seed_energy,
  std::vector<int>* RCT_energy,
  std::vector<int>* RCT_eta,
  std::vector<int>* RCT_phi,
  std::vector<int>* RCT_et5x5,
  std::vector<int>* RCT_et2x5,
  std::vector<int>* RCT_timing,
  std::vector<int>* RCT_spike,
  std::vector<int>* RCT_satur,
  std::vector<int>* RCT_brems,
  std::vector<int>* RCT_spare
);

void getECALUnclusteredEnergy(
  ap_uint<576> Data,
  int SLR,
  std::vector<int>* RCT_et,
  std::vector<int>* RCT_eta,
  std::vector<int>* RCT_phi,
  std::vector<int>* RCT_timing,
  std::vector<int>* RCT_spike
);

void getHCALTowers(
  ap_uint<576> Data,
  bool secondhalfstarts,
  int nLink,
  std::vector<int>* RCT_et,
  std::vector<int>* RCT_eta,
  std::vector<int>* RCT_phi,
  std::vector<int>* RCT_fb
);

#endif

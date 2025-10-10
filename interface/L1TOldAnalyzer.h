#ifndef L1TOldAnalyzer_H
#define L1TOldAnalyzer_H


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
#pragma link C++ class std::vector<TLorentzVector>;
#endif

//
// class declaration
//
using std::vector;

class L1TOldAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {

 public:
  
  // Constructor
  L1TOldAnalyzer(const edm::ParameterSet& ps);
  
  // Destructor
  virtual ~L1TOldAnalyzer();

  edm::Service<TFileService> tfs_;

  // Cluster branches
  std::vector<float> *cluster_pt = new std::vector<float>;
  std::vector<float> *cluster_eta = new std::vector<float>;
  std::vector<float> *cluster_phi = new std::vector<float>;

  // Tower branches
  std::vector<float> *tower_ecal_et = new std::vector<float>;
  std::vector<float> *tower_hcal_et = new std::vector<float>;
  std::vector<float> *tower_eta = new std::vector<float>;
  std::vector<float> *tower_phi = new std::vector<float>;

  TTree* outputTree;

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

  edm::EDGetTokenT<l1tp2::CaloCrystalClusterCollection> rctClustersSrc_;
  edm::EDGetTokenT<l1tp2::CaloTowerCollection> rctTowersSrc_;

  std::string folderName_;

};

#endif

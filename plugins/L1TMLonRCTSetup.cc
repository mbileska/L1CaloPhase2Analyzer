/*
 *  \file L1TMLonRCTSetup.cc
 *  Authors S. Kwan, P. Das, I. Ojalvo, R. Simeon
 */

// system include files
#include <ap_int.h>
#include <array>
#include <cmath>
// #include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <TLorentzVector.h>
#ifdef __MAKECINT__
#pragma link C++ class vector<TLorentzVector>+;
#endif

// user include files
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/EcalAlgo/interface/EcalBarrelGeometry.h"
#include "Geometry/HcalTowerAlgo/interface/HcalTrigTowerGeometry.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/L1THGCal/interface/HGCalTower.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/RCT_output.h"


// ECAL TPs
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"

// HCAL TPs
#include "DataFormats/HcalDigi/interface/HcalTriggerPrimitiveDigi.h"

// Output tower collection
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloCrystalCluster.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloTower.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloPFCluster.h"
#include "DataFormats/L1Trigger/interface/EGamma.h"

#include "L1Trigger/L1CaloTrigger/interface/ParametricCalibration.h"
#include "L1Trigger/L1TCalorimeter/interface/CaloTools.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1Trigger/L1CaloPhase2Analyzer/interface/L1TMLonRCTSetup.h"
#include "DataFormats/Math/interface/deltaR.h"


// ECAL propagation
#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "CommonTools/BaseParticlePropagator/interface/RawParticle.h"

//////////////////////////////////////////////////////////////////////////////////////

using namespace edm;

L1TMLonRCTSetup::L1TMLonRCTSetup( const ParameterSet & cfg ) :
  decoderToken_(esConsumes<CaloTPGTranscoder, CaloTPGRecord>(edm::ESInputTag("", ""))),
  caloGeometryToken_(esConsumes<CaloGeometry, CaloGeometryRecord>(edm::ESInputTag("", ""))),
  hbTopologyToken_(esConsumes<HcalTopology, HcalRecNumberingRecord>(edm::ESInputTag("", ""))),
  EGammaSLR3Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("EGammaSLR3"))),
  EGammaSLR2Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("EGammaSLR2"))),
  EGammaSLR1Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("EGammaSLR1"))),
  EGammaSLR0Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("EGammaSLR0"))),
  HCAL8Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("HCAL8"))),
  HCAL7Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("HCAL7"))),
  HCAL6Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("HCAL6"))),
  HCAL5Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("HCAL5")))
{
    folderName_          = cfg.getUntrackedParameter<std::string>("folderName");

    linkTree = tfs_->make<TTree>("linkTree", "RCT Output Link Tree");

    linkTree->Branch("run",    &run,     "run/I");
    linkTree->Branch("lumi",   &lumi,    "lumi/I");
    linkTree->Branch("event",  &event,   "event/I");
    
    ////putting bufsize at 32000 and changing split level to 0 so that the branch isn't split into multiple branches

    // Cluster branches SLR3
    linkTree->Branch("SLR3_cluster_seed_energy", "vector<vector<int>>", &SLR3_cluster_seed_energy, 32000, 0);
    linkTree->Branch("SLR3_cluster_energy", "vector<vector<int>>", &SLR3_cluster_energy, 32000, 0);
    linkTree->Branch("SLR3_cluster_eta", "vector<vector<int>>", &SLR3_cluster_eta, 32000, 0);
    linkTree->Branch("SLR3_cluster_phi", "vector<vector<int>>", &SLR3_cluster_phi, 32000, 0);
    linkTree->Branch("SLR3_cluster_et5x5", "vector<vector<int>>", &SLR3_cluster_et5x5, 32000, 0);
    linkTree->Branch("SLR3_cluster_et2x5", "vector<vector<int>>", &SLR3_cluster_et2x5, 32000, 0);
    linkTree->Branch("SLR3_cluster_timing", "vector<vector<int>>", &SLR3_cluster_timing, 32000, 0);
    linkTree->Branch("SLR3_cluster_spike", "vector<vector<int>>", &SLR3_cluster_spike, 32000, 0);
    linkTree->Branch("SLR3_cluster_satur", "vector<vector<int>>", &SLR3_cluster_satur, 32000, 0);
    linkTree->Branch("SLR3_cluster_brems", "vector<vector<int>>", &SLR3_cluster_brems, 32000, 0);
    linkTree->Branch("SLR3_cluster_spare", "vector<vector<int>>", &SLR3_cluster_spare, 32000, 0);

    // Cluster branches SLR2
    linkTree->Branch("SLR2_cluster_seed_energy", "vector<vector<int>>", &SLR2_cluster_seed_energy, 32000, 0);
    linkTree->Branch("SLR2_cluster_energy", "vector<vector<int>>", &SLR2_cluster_energy, 32000, 0);
    linkTree->Branch("SLR2_cluster_eta", "vector<vector<int>>", &SLR2_cluster_eta, 32000, 0);
    linkTree->Branch("SLR2_cluster_phi", "vector<vector<int>>", &SLR2_cluster_phi, 32000, 0);
    linkTree->Branch("SLR2_cluster_et5x5", "vector<vector<int>>", &SLR2_cluster_et5x5, 32000, 0);
    linkTree->Branch("SLR2_cluster_et2x5", "vector<vector<int>>", &SLR2_cluster_et2x5, 32000, 0);
    linkTree->Branch("SLR2_cluster_timing", "vector<vector<int>>", &SLR2_cluster_timing, 32000, 0);
    linkTree->Branch("SLR2_cluster_spike", "vector<vector<int>>", &SLR2_cluster_spike, 32000, 0);
    linkTree->Branch("SLR2_cluster_satur", "vector<vector<int>>", &SLR2_cluster_satur, 32000, 0);
    linkTree->Branch("SLR2_cluster_brems", "vector<vector<int>>", &SLR2_cluster_brems, 32000, 0);
    linkTree->Branch("SLR2_cluster_spare", "vector<vector<int>>", &SLR2_cluster_spare, 32000, 0);

    // Cluster branches SLR1
    linkTree->Branch("SLR1_cluster_seed_energy", "vector<vector<int>>", &SLR1_cluster_seed_energy, 32000, 0);
    linkTree->Branch("SLR1_cluster_energy", "vector<vector<int>>", &SLR1_cluster_energy, 32000, 0);
    linkTree->Branch("SLR1_cluster_eta", "vector<vector<int>>", &SLR1_cluster_eta, 32000, 0);
    linkTree->Branch("SLR1_cluster_phi", "vector<vector<int>>", &SLR1_cluster_phi, 32000, 0);
    linkTree->Branch("SLR1_cluster_et5x5", "vector<vector<int>>", &SLR1_cluster_et5x5, 32000, 0);
    linkTree->Branch("SLR1_cluster_et2x5", "vector<vector<int>>", &SLR1_cluster_et2x5, 32000, 0);
    linkTree->Branch("SLR1_cluster_timing", "vector<vector<int>>", &SLR1_cluster_timing, 32000, 0);
    linkTree->Branch("SLR1_cluster_spike", "vector<vector<int>>", &SLR1_cluster_spike, 32000, 0);
    linkTree->Branch("SLR1_cluster_satur", "vector<vector<int>>", &SLR1_cluster_satur, 32000, 0);
    linkTree->Branch("SLR1_cluster_brems", "vector<vector<int>>", &SLR1_cluster_brems, 32000, 0);
    linkTree->Branch("SLR1_cluster_spare", "vector<vector<int>>", &SLR1_cluster_spare, 32000, 0);

    // Cluster branches SLR0
    linkTree->Branch("SLR0_cluster_seed_energy", "vector<vector<int>>", &SLR0_cluster_seed_energy, 32000, 0);
    linkTree->Branch("SLR0_cluster_energy", "vector<vector<int>>", &SLR0_cluster_energy, 32000, 0);
    linkTree->Branch("SLR0_cluster_eta", "vector<vector<int>>", &SLR0_cluster_eta, 32000, 0);
    linkTree->Branch("SLR0_cluster_phi", "vector<vector<int>>", &SLR0_cluster_phi, 32000, 0);
    linkTree->Branch("SLR0_cluster_et5x5", "vector<vector<int>>", &SLR0_cluster_et5x5, 32000, 0);
    linkTree->Branch("SLR0_cluster_et2x5", "vector<vector<int>>", &SLR0_cluster_et2x5, 32000, 0);
    linkTree->Branch("SLR0_cluster_timing", "vector<vector<int>>", &SLR0_cluster_timing, 32000, 0);
    linkTree->Branch("SLR0_cluster_spike", "vector<vector<int>>", &SLR0_cluster_spike, 32000, 0);
    linkTree->Branch("SLR0_cluster_satur", "vector<vector<int>>", &SLR0_cluster_satur, 32000, 0);
    linkTree->Branch("SLR0_cluster_brems", "vector<vector<int>>", &SLR0_cluster_brems, 32000, 0);
    linkTree->Branch("SLR0_cluster_spare", "vector<vector<int>>", &SLR0_cluster_spare, 32000, 0);

    // Tower branches FIXME: Change to HCAL branches
    linkTree->Branch("tower_et", "vector<vector<int>>", &tower_et, 32000, 0);
    linkTree->Branch("tower_eta", "vector<vector<int>>", &tower_eta, 32000, 0);
    linkTree->Branch("tower_phi", "vector<vector<int>>", &tower_phi, 32000, 0);
    linkTree->Branch("tower_hoe", "vector<vector<int>>", &hoe, 32000, 0);
    linkTree->Branch("tower_fb", "vector<vector<int>>", &fb, 32000, 0);

  }

void L1TMLonRCTSetup::beginJob( const EventSetup & es) {
}

void L1TMLonRCTSetup::analyze( const Event& evt, const EventSetup& es )
 {

  run = evt.id().run();
  lumi = evt.id().luminosityBlock();
  event = evt.id().event();

  // Set up handles to read in links
  edm::Handle<l1tp2::rctOutputLinkCollection> EGammaSLR3_link;
  edm::Handle<l1tp2::rctOutputLinkCollection> EGammaSLR2_link;
  edm::Handle<l1tp2::rctOutputLinkCollection> EGammaSLR1_link;
  edm::Handle<l1tp2::rctOutputLinkCollection> EGammaSLR0_link;
  // FIXME: Add HCAL input links

  // Clear cluster vectors
  // SLR3
  SLR3_cluster_seed_energy->clear();
  SLR3_cluster_energy->clear();
  SLR3_cluster_eta->clear();
  SLR3_cluster_phi->clear();
  SLR3_cluster_et5x5->clear();
  SLR3_cluster_et2x5->clear();
  SLR3_cluster_timing->clear();
  SLR3_cluster_spike->clear();
  SLR3_cluster_satur->clear();
  SLR3_cluster_brems->clear();
  SLR3_cluster_spare->clear();
  // SLR2
  SLR2_cluster_seed_energy->clear();
  SLR2_cluster_energy->clear();
  SLR2_cluster_eta->clear();
  SLR2_cluster_phi->clear();
  SLR2_cluster_et5x5->clear();
  SLR2_cluster_et2x5->clear();
  SLR2_cluster_timing->clear();
  SLR2_cluster_spike->clear();
  SLR2_cluster_satur->clear();
  SLR2_cluster_brems->clear();
  SLR2_cluster_spare->clear();
  // SLR1
  SLR1_cluster_seed_energy->clear();
  SLR1_cluster_energy->clear();
  SLR1_cluster_eta->clear();
  SLR1_cluster_phi->clear();
  SLR1_cluster_et5x5->clear();
  SLR1_cluster_et2x5->clear();
  SLR1_cluster_timing->clear();
  SLR1_cluster_spike->clear();
  SLR1_cluster_satur->clear();
  SLR1_cluster_brems->clear();
  SLR1_cluster_spare->clear();
  // SLR0
  SLR0_cluster_seed_energy->clear();
  SLR0_cluster_energy->clear();
  SLR0_cluster_eta->clear();
  SLR0_cluster_phi->clear();
  SLR0_cluster_et5x5->clear();
  SLR0_cluster_et2x5->clear();
  SLR0_cluster_timing->clear();
  SLR0_cluster_spike->clear();
  SLR0_cluster_satur->clear();
  SLR0_cluster_brems->clear();
  SLR0_cluster_spare->clear();

  // FIXME: Clear HCAL vectors

  // Read out SLR3 EGamma clusters
  if(evt.getByToken(EGammaSLR3Src_, EGammaSLR3_link)){
    for(const auto & link : *EGammaSLR3_link){

      getEGammaClusters(
        link.data(),
        RCT_cluster_seed_energy,
        RCT_cluster_energy,
        RCT_cluster_eta,
        RCT_cluster_phi,
        RCT_cluster_et5x5,
        RCT_cluster_et2x5,
        RCT_cluster_timing,
        RCT_cluster_spike,
        RCT_cluster_satur,
        RCT_cluster_brems,
        RCT_cluster_spare
      );
      SLR3_cluster_seed_energy->push_back(*RCT_cluster_seed_energy);
      SLR3_cluster_energy->push_back(*RCT_cluster_energy);
      SLR3_cluster_eta->push_back(*RCT_cluster_eta);
      SLR3_cluster_phi->push_back(*RCT_cluster_phi);
      SLR3_cluster_et5x5->push_back(*RCT_cluster_et5x5);
      SLR3_cluster_et2x5->push_back(*RCT_cluster_et2x5);
      SLR3_cluster_timing->push_back(*RCT_cluster_timing);
      SLR3_cluster_spike->push_back(*RCT_cluster_spike);
      SLR3_cluster_satur->push_back(*RCT_cluster_satur);
      SLR3_cluster_brems->push_back(*RCT_cluster_brems);
      SLR3_cluster_spare->push_back(*RCT_cluster_spare);
    }
  }
  // Read out SLR2 EGamma clusters
  if(evt.getByToken(EGammaSLR2Src_, EGammaSLR2_link)){
    for(const auto & link : *EGammaSLR2_link){

      getEGammaClusters(
        link.data(),
        RCT_cluster_seed_energy,
        RCT_cluster_energy,
        RCT_cluster_eta,
        RCT_cluster_phi,
        RCT_cluster_et5x5,
        RCT_cluster_et2x5,
        RCT_cluster_timing,
        RCT_cluster_spike,
        RCT_cluster_satur,
        RCT_cluster_brems,
        RCT_cluster_spare
      );
      SLR2_cluster_seed_energy->push_back(*RCT_cluster_seed_energy);
      SLR2_cluster_energy->push_back(*RCT_cluster_energy);
      SLR2_cluster_eta->push_back(*RCT_cluster_eta);
      SLR2_cluster_phi->push_back(*RCT_cluster_phi);
      SLR2_cluster_et5x5->push_back(*RCT_cluster_et5x5);
      SLR2_cluster_et2x5->push_back(*RCT_cluster_et2x5);
      SLR2_cluster_timing->push_back(*RCT_cluster_timing);
      SLR2_cluster_spike->push_back(*RCT_cluster_spike);
      SLR2_cluster_satur->push_back(*RCT_cluster_satur);
      SLR2_cluster_brems->push_back(*RCT_cluster_brems);
      SLR2_cluster_spare->push_back(*RCT_cluster_spare);
    }
  }
  // Read out SLR1 EGamma clusters
  if(evt.getByToken(EGammaSLR1Src_, EGammaSLR1_link)){
    for(const auto & link : *EGammaSLR1_link){

      getEGammaClusters(
        link.data(),
        RCT_cluster_seed_energy,
        RCT_cluster_energy,
        RCT_cluster_eta,
        RCT_cluster_phi,
        RCT_cluster_et5x5,
        RCT_cluster_et2x5,
        RCT_cluster_timing,
        RCT_cluster_spike,
        RCT_cluster_satur,
        RCT_cluster_brems,
        RCT_cluster_spare
      );
      SLR1_cluster_seed_energy->push_back(*RCT_cluster_seed_energy);
      SLR1_cluster_energy->push_back(*RCT_cluster_energy);
      SLR1_cluster_eta->push_back(*RCT_cluster_eta);
      SLR1_cluster_phi->push_back(*RCT_cluster_phi);
      SLR1_cluster_et5x5->push_back(*RCT_cluster_et5x5);
      SLR1_cluster_et2x5->push_back(*RCT_cluster_et2x5);
      SLR1_cluster_timing->push_back(*RCT_cluster_timing);
      SLR1_cluster_spike->push_back(*RCT_cluster_spike);
      SLR1_cluster_satur->push_back(*RCT_cluster_satur);
      SLR1_cluster_brems->push_back(*RCT_cluster_brems);
      SLR1_cluster_spare->push_back(*RCT_cluster_spare);
    }
  }
  // Read out SLR0 EGamma clusters
  if(evt.getByToken(EGammaSLR0Src_, EGammaSLR0_link)){
    for(const auto & link : *EGammaSLR0_link){

      getEGammaClusters(
        link.data(),
        RCT_cluster_seed_energy,
        RCT_cluster_energy,
        RCT_cluster_eta,
        RCT_cluster_phi,
        RCT_cluster_et5x5,
        RCT_cluster_et2x5,
        RCT_cluster_timing,
        RCT_cluster_spike,
        RCT_cluster_satur,
        RCT_cluster_brems,
        RCT_cluster_spare
      );
      SLR0_cluster_seed_energy->push_back(*RCT_cluster_seed_energy);
      SLR0_cluster_energy->push_back(*RCT_cluster_energy);
      SLR0_cluster_eta->push_back(*RCT_cluster_eta);
      SLR0_cluster_phi->push_back(*RCT_cluster_phi);
      SLR0_cluster_et5x5->push_back(*RCT_cluster_et5x5);
      SLR0_cluster_et2x5->push_back(*RCT_cluster_et2x5);
      SLR0_cluster_timing->push_back(*RCT_cluster_timing);
      SLR0_cluster_spike->push_back(*RCT_cluster_spike);
      SLR0_cluster_satur->push_back(*RCT_cluster_satur);
      SLR0_cluster_brems->push_back(*RCT_cluster_brems);
      SLR0_cluster_spare->push_back(*RCT_cluster_spare);
    }
  }

  // FIXME: Write out HCAL TPs
  if(evt.getByToken(link1Src_, linkOut1_int576)){
    for(const auto & link : *linkOut1_int576){
      linkOut1->push_back((int)link.data());

      getIP3OutputTowers(
        link.data(),
        1,
        RCT_tower_et,
        RCT_tower_eta,
        RCT_tower_phi,
        RCT_hoe,
        RCT_fb
      );
      tower_et->push_back(*RCT_tower_et);
      tower_eta->push_back(*RCT_tower_eta);
      tower_phi->push_back(*RCT_tower_phi);
      hoe->push_back(*RCT_hoe);
      fb->push_back(*RCT_fb);
    }
  }

  if(evt.getByToken(link2Src_, linkOut2_int576)){
    for(const auto & link : *linkOut2_int576){
      linkOut2->push_back((int)link.data());

      getIP3OutputTowers(
        link.data(),
        2,
        RCT_tower_et,
        RCT_tower_eta,
        RCT_tower_phi,
        RCT_hoe,
        RCT_fb
      );
      tower_et->push_back(*RCT_tower_et);
      tower_eta->push_back(*RCT_tower_eta);
      tower_phi->push_back(*RCT_tower_phi);
      hoe->push_back(*RCT_hoe);
      fb->push_back(*RCT_fb);
    }
  }

  if(evt.getByToken(link3Src_, linkOut3_int576)){
    for(const auto & link : *linkOut3_int576){
      linkOut3->push_back((int)link.data());

      getIP3OutputTowers(
        link.data(),
        3,
        RCT_tower_et,
        RCT_tower_eta,
        RCT_tower_phi,
        RCT_hoe,
        RCT_fb
      );
      tower_et->push_back(*RCT_tower_et);
      tower_eta->push_back(*RCT_tower_eta);
      tower_phi->push_back(*RCT_tower_phi);
      hoe->push_back(*RCT_hoe);
      fb->push_back(*RCT_fb);
    }
  }

  linkTree->Fill();
 
 }


void L1TMLonRCTSetup::endJob() {
}

L1TMLonRCTSetup::~L1TMLonRCTSetup(){
}

//////////////////////////// Utility functions ///////////////////////////////
void getEGammaClusters(
  ap_uint<576> Data,
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
) {
  
  RCT_seed_energy->clear();
  RCT_energy->clear();
  RCT_eta->clear();
  RCT_phi->clear();
  RCT_et5x5->clear();
  RCT_et2x5->clear();
  RCT_timing->clear();
  RCT_spike->clear();
  RCT_satur->clear();
  RCT_brems->clear();
  RCT_spare->clear();

  for(int i=0; i<9; i++){
    int start = i*64;
    
    int this_seed_energy = (int)Data.range(start+9,start);
    RCT_seed_energy->push_back(this_seed_pt);
    int this_energy = (int)Data.range(start+21,start+10);
    RCT_energy->push_back(this_pt);
    int this_eta = (int)Data.range(start+26,start+22);
    RCT_eta->push_back(this_eta);
    int this_phi = (int)Data.range(start+31,start+27);
    RCT_phi->push_back(this_phi);
    int this_et5x5 = (int)Data.range(start+41,start+32);
    RCT_et5x5->push_back(this_et5x5);
    int this_wps = (int)Data.range(start+51,start+42);
    RCT_et2x5->push_back(this_wps);
    int this_timing = (int)Data.range(start+56,start+52);
    RCT_timing->push_back(this_timing);
    int this_spike = (int)Data.range(start+57,start+57);
    RCT_spare->push_back(this_spike);
    int this_satur = (int)Data.range(start+58,start+58);
    RCT_spare->push_back(this_satur);
    int this_brems = (int)Data.range(start+60,start+59);
    RCT_brems->push_back(this_brems);
    int this_spare = (int)Data.range(start+63,start+61);
    RCT_spare->push_back(this_spare);
  }

}

// FIXME: Replace with a function that gets HCAL TP info
void getIP3OutputTowers(
  ap_uint<576> Data,
  int whichLink,
  std::vector<int>* RCT_et,
  std::vector<int>* RCT_eta,
  std::vector<int>* RCT_phi,
  std::vector<int>* RCT_hoe,
  std::vector<int>* RCT_fb
) {

  int this_et;
  int this_hoe;
  int this_fb;

  RCT_et->clear();
  RCT_eta->clear();
  RCT_phi->clear();
  RCT_hoe->clear();
  RCT_fb->clear();

  for(int i=0; i<17; i++) {
    // Lower iPhi in this link
    int this_phi = (whichLink - 1)*2;
    int start = i*16;

    this_et = (int)Data.range(start+9,start);
    RCT_et->push_back(this_et);
    RCT_eta->push_back(i);
    RCT_phi->push_back(this_phi);
    this_hoe = (int)Data.range(start+13,start+10);
    RCT_hoe->push_back(this_hoe);
    this_fb = (int)Data.range(start+15,start+14);
    RCT_fb->push_back(this_fb);

    // Higher iPhi in this link
    this_phi = (whichLink - 1)*2 + 1;
    start = i*16 + 272;
    
    this_et = (int)Data.range(start+9,start);
    RCT_et->push_back(this_et);
    RCT_eta->push_back(i);
    RCT_phi->push_back(this_phi);
    this_hoe = (int)Data.range(start+13,start+10);
    RCT_hoe->push_back(this_hoe);
    this_fb = (int)Data.range(start+15,start+14);
    RCT_fb->push_back(this_fb);
  }

}

DEFINE_FWK_MODULE(L1TMLonRCTSetup);

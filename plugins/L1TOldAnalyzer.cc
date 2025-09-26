/*
 *  \file L1TOldAnalyzer.cc
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

#include "L1Trigger/L1CaloPhase2Analyzer/interface/L1TOldAnalyzer.h"
#include "DataFormats/Math/interface/deltaR.h"


// ECAL propagation
#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "CommonTools/BaseParticlePropagator/interface/RawParticle.h"

///////////////////////////////////////////////////////////////////////////////////////

using namespace edm;

L1TOldAnalyzer::L1TOldAnalyzer( const ParameterSet & cfg ) :
  decoderToken_(esConsumes<CaloTPGTranscoder, CaloTPGRecord>(edm::ESInputTag("", ""))),
  caloGeometryToken_(esConsumes<CaloGeometry, CaloGeometryRecord>(edm::ESInputTag("", ""))),
  hbTopologyToken_(esConsumes<HcalTopology, HcalRecNumberingRecord>(edm::ESInputTag("", ""))),
  rctClustersSrc_(consumes<l1tp2::CaloCrystalClusterCollection>(cfg.getParameter<edm::InputTag>("rctClusters"))),
  rctTowersSrc_(consumes<l1tp2::CaloTowerCollection>(cfg.getParameter<edm::InputTag>("rctTowers")))
{
    folderName_          = cfg.getUntrackedParameter<std::string>("folderName");

    outputTree = tfs_->make<TTree>("outputTree", "RCT Output Tree");

    outputTree->Branch("run",    &run,     "run/I");
    outputTree->Branch("lumi",   &lumi,    "lumi/I");
    outputTree->Branch("event",  &event,   "event/I");

    // Cluster branches
    outputTree->Branch("cluster_pt", "vector<float>", &cluster_pt, 32000, 0);
    outputTree->Branch("cluster_eta", "vector<float>", &cluster_eta, 32000, 0);
    outputTree->Branch("cluster_phi", "vector<float>", &cluster_phi, 32000, 0);

    // Tower branches
    outputTree->Branch("tower_ecal_et", "vector<float>", &tower_ecal_et, 32000, 0);
    outputTree->Branch("tower_hcal_et", "vector<float>", &tower_hcal_et, 32000, 0);
    outputTree->Branch("tower_eta", "vector<float>", &tower_eta, 32000, 0);
    outputTree->Branch("tower_phi", "vector<float>", &tower_phi, 32000, 0);

  }

void L1TOldAnalyzer::beginJob( const EventSetup & es) {
}

void L1TOldAnalyzer::analyze( const Event& evt, const EventSetup& es )
 {

  run = evt.id().run();
  lumi = evt.id().luminosityBlock();
  event = evt.id().event();

  edm::Handle<l1tp2::CaloCrystalClusterCollection> rctClusters;
  edm::Handle<l1tp2::CaloTowerCollection> rctTowers;

  // Clear cluster vectors
  cluster_pt->clear();
  cluster_eta->clear();
  cluster_phi->clear();

  // Clear tower vectors
  tower_ecal_et->clear();
  tower_hcal_et->clear();
  tower_eta->clear();
  tower_phi->clear();

  if(evt.getByToken(rctClustersSrc_, rctClusters)){
    for(const auto & c : *rctClusters){
      cluster_pt->push_back(c.calibratedPt());
      cluster_eta->push_back(c.p4().eta());
      cluster_phi->push_back(c.p4().phi());
    }
  }

  if(evt.getByToken(rctTowersSrc_, rctTowers)){
    for(const auto & t : *rctTowers){
      tower_ecal_et->push_back(t.ecalTowerEt());
      tower_hcal_et->push_back(t.hcalTowerEt());
      tower_eta->push_back(t.towerEta());
      tower_phi->push_back(t.towerPhi());
    }
  }

  outputTree->Fill();
 
 }


void L1TOldAnalyzer::endJob() {
}

L1TOldAnalyzer::~L1TOldAnalyzer(){
}

DEFINE_FWK_MODULE(L1TOldAnalyzer);

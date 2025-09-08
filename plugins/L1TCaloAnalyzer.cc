/*
 *  \file L1TCaloAnalyzer.cc
 *  Authors S. Kwan, P. Das, I. Ojalvo
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

#include "L1Trigger/L1CaloPhase2Analyzer/interface/L1TCaloAnalyzer.h"
#include "DataFormats/Math/interface/deltaR.h"


// ECAL propagation
#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "CommonTools/BaseParticlePropagator/interface/RawParticle.h"

//////////////////////////////////////////////////////////////////////////////////////

using namespace edm;

L1TCaloAnalyzer::L1TCaloAnalyzer( const ParameterSet & cfg ) :
  decoderToken_(esConsumes<CaloTPGTranscoder, CaloTPGRecord>(edm::ESInputTag("", ""))),
  caloGeometryToken_(esConsumes<CaloGeometry, CaloGeometryRecord>(edm::ESInputTag("", ""))),
  hbTopologyToken_(esConsumes<HcalTopology, HcalRecNumberingRecord>(edm::ESInputTag("", ""))),
  link0Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("LinkOut0"))),
  link1Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("LinkOut1"))),
  link2Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("LinkOut2"))),
  link3Src_(consumes<l1tp2::rctOutputLinkCollection>(cfg.getParameter<edm::InputTag>("LinkOut3")))
{
    folderName_          = cfg.getUntrackedParameter<std::string>("folderName");

    linkTree = tfs_->make<TTree>("linkTree", "RCT Output Link Tree");

    linkTree->Branch("run",    &run,     "run/I");
    linkTree->Branch("lumi",   &lumi,    "lumi/I");
    linkTree->Branch("event",  &event,   "event/I");
    
    ////putting bufsize at 32000 and changing split level to 0 so that the branch isn't split into multiple branches

    linkTree->Branch("linkOut0", "vector<int>", &linkOut0, 32000, 0);
    linkTree->Branch("linkOut1", "vector<int>", &linkOut1, 32000, 0);
    linkTree->Branch("linkOut2", "vector<int>", &linkOut2, 32000, 0);
    linkTree->Branch("linkOut3", "vector<int>", &linkOut3, 32000, 0);

  }

void L1TCaloAnalyzer::beginJob( const EventSetup & es) {
}

void L1TCaloAnalyzer::analyze( const Event& evt, const EventSetup& es )
 {

  run = evt.id().run();
  lumi = evt.id().luminosityBlock();
  event = evt.id().event();

  edm::Handle<l1tp2::rctOutputLinkCollection> linkOut0_int576;
  edm::Handle<l1tp2::rctOutputLinkCollection> linkOut1_int576;
  edm::Handle<l1tp2::rctOutputLinkCollection> linkOut2_int576;
  edm::Handle<l1tp2::rctOutputLinkCollection> linkOut3_int576;

  linkOut0->clear();
  linkOut1->clear();
  linkOut2->clear();
  linkOut3->clear();

  if(evt.getByToken(link0Src_, linkOut0_int576)){
    for(const auto & link : *link0_int576){
      linkOut0->push_back((int)link);
    }
  }

  if(evt.getByToken(link1Src_, linkOut1_int576)){
    for(const auto & link : *link1_int576){
      linkOut1->push_back((int)link);
    }
  }

  if(evt.getByToken(link2Src_, linkOut2_int576)){
    for(const auto & link : *link2_int576){
      linkOut2->push_back((int)link);
    }
  }

  if(evt.getByToken(link3Src_, linkOut3_int576)){
    for(const auto & link : *link3_int576){
      linkOut3->push_back((int)link);
    }
  }

  linkTree->Fill();
 
 }


void L1TCaloAnalyzer::endJob() {
}

L1TCaloAnalyzer::~L1TCaloAnalyzer(){
}

DEFINE_FWK_MODULE(L1TCaloAnalyzer);

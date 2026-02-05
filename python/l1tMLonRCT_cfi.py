import FWCore.ParameterSet.Config as cms


l1NtupleProducer = cms.EDAnalyzer("L1TMLonRCTSetupAnalyzer",
                                  folderName = cms.untracked.string("firstFolder"),
                                  EGammaSLR3 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "EGammaSLR3"),
                                  EGammaSLR2 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "EGammaSLR2"),
                                  EGammaSLR1 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "EGammaSLR1"),
                                  EGammaSLR0 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "EGammaSLR0"),
                                  HCAL8 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "HCAL8"),
                                  HCAL7 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "HCAL7"),
                                  HCAL6 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "HCAL6"),
                                  HCAL5 = cms.InputTag("l1tPhase2MLonRCTSetupProducer", "HCAL5"),
)

import FWCore.ParameterSet.Config as cms


l1NtupleProducer = cms.EDAnalyzer("L1TCaloAnalyzer",
                                  folderName = cms.untracked.string("firstFolder"),
                                  link0 = cms.InputTag("l1tPhase2RCTEmulatorProducer", "LinkOut0"),
                                  link1 = cms.InputTag("l1tPhase2RCTEmulatorProducer", "LinkOut1"),
                                  link2 = cms.InputTag("l1tPhase2RCTEmulatorProducer", "LinkOut2"),
                                  link3 = cms.InputTag("l1tPhase2RCTEmulatorProducer", "LinkOut3"),
)

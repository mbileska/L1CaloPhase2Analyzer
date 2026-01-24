import FWCore.ParameterSet.Config as cms


l1NtupleOldProducer = cms.EDAnalyzer("L1TOldAnalyzer",
                                  folderName = cms.untracked.string("firstFolder"),
                                  rctClusters = cms.InputTag("l1tPhase2L1CaloEGammaEmulator", "RCTClusters"),
                                  rctTowers = cms.InputTag("l1tPhase2L1CaloEGammaEmulator", "RCTTowers"),
)

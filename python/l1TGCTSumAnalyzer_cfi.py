import FWCore.ParameterSet.Config as cms

l1TGCTSumAnalyzer = cms.EDAnalyzer(
    "L1TGCTSumAnalyzer",
    folderName = cms.untracked.string("GCTSumAnalyzer"),
    LinkOut0 = cms.InputTag("phase2L1GCTSumEmulator", "LinkOut0"),
    LinkOut1 = cms.InputTag("phase2L1GCTSumEmulator", "LinkOut1"),
    LinkOut2 = cms.InputTag("phase2L1GCTSumEmulator", "LinkOut2"),
    LinkOut3 = cms.InputTag("phase2L1GCTSumEmulator", "LinkOut3"),
    LinkOut4 = cms.InputTag("phase2L1GCTSumEmulator", "LinkOut4"),
    LinkOut5 = cms.InputTag("phase2L1GCTSumEmulator", "LinkOut5"),
)
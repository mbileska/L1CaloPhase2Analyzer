import FWCore.ParameterSet.Config as cms

l1TMHHAnalyzer = cms.EDAnalyzer(
    "L1TMHHAnalyzer",
    debug=cms.untracked.bool(False),
    inputDumpFile=cms.untracked.string("mhh_input.txt"),
    outputDumpFile=cms.untracked.string("mhh_cmssw_output.txt"),
    inputLinks=cms.VInputTag(),
    outputLinks=cms.VInputTag(),
)

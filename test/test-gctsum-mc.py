import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

from Configuration.StandardSequences.Eras import eras


options = VarParsing.VarParsing("analysis")
options.outputFile = "gctsum_mc.root"
options.maxEvents = 1000
options.inputFiles = [
    "file:///hdfs/store/user/rsimeon/MCFiles/001ebf5f-b83c-43fc-997f-c2e5ecf1f9dd.root",
]
options.register(
    "sumInputDumpFile",
    "gctsum_mc_sum_input.txt",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Text dump of the packed GCT Sum input links",
)
options.register(
    "gtOutputDumpFile",
    "gctsum_mc_gt_output.txt",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Text dump of the CMSSW GT-link outputs",
)
options.parseArguments()


process = cms.Process("GCTSUMMC", eras.Phase2C17I13M9)

process.load("Configuration.StandardSequences.Services_cff")
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("SimGeneral.MixingModule.mixNoPU_cfi")
process.load("Configuration.Geometry.GeometryExtendedRun4D110Reco_cff")
process.load("Configuration.Geometry.GeometryExtendedRun4D110_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.SimL1Emulator_cff")
process.load("Configuration.StandardSequences.EndOfProcess_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, "131X_mcRun4_realistic_v6", "")

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents)
)

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(*options.inputFiles),
    inputCommands=cms.untracked.vstring(
        "keep *",
        "drop l1tTkPrimaryVertexs_*_*_*",
    ),
)

process.MessageLogger.cerr.threshold = cms.untracked.string("INFO")
process.MessageLogger.cerr.FwkReport = cms.untracked.PSet(
    reportEvery=cms.untracked.int32(100)
)

process.load("SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff")
process.load("CalibCalorimetry.CaloTPG.CaloTPGTranscoder_cfi")

process.load("L1Trigger.L1CaloTrigger.l1tPhase2L1CaloEGammaEmulator_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2CaloJetEmulator_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2L1GCTSumEmulator_cfi")
process.load("L1Trigger.L1CaloPhase2Analyzer.l1TGCTSumAnalyzer_cfi")

process.l1tPhase2CaloJetEmulator.useHgcalTowers = cms.bool(False)

process.gctSumCandidateInputProducer = cms.EDProducer(
    "GCTSumCandidateInputProducer",
    gctEGammas=cms.InputTag("l1tPhase2L1CaloEGammaEmulator", "GCTEGammas"),
    gctJets=cms.InputTag("l1tPhase2CaloJetEmulator", "GCTJet"),
    debug=cms.bool(False),
)

packedInputTags = cms.VInputTag(
    *[
        cms.InputTag("gctSumCandidateInputProducer", f"LinkIn{i}")
        for i in range(24)
    ]
)

process.phase2L1GCTSumEmulator.debug = cms.bool(False)
process.phase2L1GCTSumEmulator.inputLinks = packedInputTags

process.l1TGCTSumAnalyzer.debug = cms.untracked.bool(False)
process.l1TGCTSumAnalyzer.sumInputDumpFile = cms.untracked.string(options.sumInputDumpFile)
process.l1TGCTSumAnalyzer.cmsswGtOutputDumpFile = cms.untracked.string(options.gtOutputDumpFile)
process.l1TGCTSumAnalyzer.inputLinks = packedInputTags
process.l1TGCTSumAnalyzer.outputLinks = cms.VInputTag(
    *[
        cms.InputTag("phase2L1GCTSumEmulator", f"LinkOut{i}")
        for i in range(6)
    ]
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string(options.outputFile)
)

process.GCTSUM = cms.Path(
    process.l1tPhase2L1CaloEGammaEmulator *
    process.l1tPhase2CaloJetEmulator *
    process.gctSumCandidateInputProducer *
    process.phase2L1GCTSumEmulator *
    process.l1TGCTSumAnalyzer
)

process.schedule = cms.Schedule(process.GCTSUM)

process.options.numberOfThreads = cms.untracked.uint32(1)
process.options.numberOfStreams = cms.untracked.uint32(1)
process.options.wantSummary = cms.untracked.bool(True)

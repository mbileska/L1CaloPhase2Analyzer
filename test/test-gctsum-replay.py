import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import re
from Configuration.StandardSequences.Eras import eras


def count_events(path, words_per_event=9, expected_columns=25):
    rows = 0
    wordcnt_re = re.compile(r"^(0x)?[0-9a-fA-F]+$")

    with open(path, "r", encoding="utf-8") as handle:
        for line_number, line in enumerate(handle, start=1):
            stripped = line.strip()

            if not stripped or stripped.startswith("#"):
                continue

            tokens = stripped.split()

            # Skip column header, e.g.
            # WordCnt POS_0 POS_1 ... NEG_11
            if not wordcnt_re.match(tokens[0]):
                continue

            # A real GCTSUM input row should be:
            # WordCnt + 24 input links = 25 columns
            if len(tokens) != expected_columns:
                raise RuntimeError(
                    f"Malformed data row in {path} at line {line_number}: "
                    f"expected {expected_columns} columns, got {len(tokens)}"
                )

            rows += 1

    if rows % words_per_event != 0:
        raise RuntimeError(
            f"Input file {path} has {rows} data rows, "
            f"which is not divisible by {words_per_event}"
        )

    return rows // words_per_event

options = VarParsing.VarParsing("analysis")

options.register(
    "inputFile",
    "",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Path to the GCT Sum input vector file",
)

options.outputFile = "gctsum_replay.root"

options.register(
    "sumInputDumpFile",
    "gctsum_replay_sum_input.txt",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Text dump of the replayed GCT Sum emulator inputs",
)
options.register(
    "gtOutputDumpFile",
    "gctsum_replay_gt_output.txt",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "Text dump of the CMSSW GT-link outputs",
)
options.register(
    "posOffset",
    0,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.int,
    "Column offset for the 12 positive-eta input links",
)
options.register(
    "negOffset",
    12,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.int,
    "Column offset for the 12 negative-eta input links",
)
options.register(
    "eventOffset",
    0,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.int,
    "Event offset applied before replaying the file-backed events",
)
options.register(
    "wrapAround",
    False,
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.bool,
    "Wrap around to the first replay event instead of failing at end-of-file",
)
options.parseArguments()

if not options.inputFile:
    raise RuntimeError("Pass inputFile=/path/to/GCTSUM_TV_*.txt or another compatible GCT Sum vector file")

default_max_events = count_events(options.inputFile)

process = cms.Process("GCTSUMREPLAY", eras.Phase2C17I13M9)

process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.Geometry.GeometryExtendedRun4D110Reco_cff")
process.load("Configuration.Geometry.GeometryExtendedRun4D110_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.EndOfProcess_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, "131X_mcRun4_realistic_v6", "")

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents if options.maxEvents >= 0 else default_max_events)
)

process.source = cms.Source(
    "EmptySource",
    firstRun=cms.untracked.uint32(1),
    firstEvent=cms.untracked.uint32(1),
)

process.MessageLogger.cerr.threshold = cms.untracked.string("INFO")
process.MessageLogger.cerr.FwkReport = cms.untracked.PSet(
    reportEvery=cms.untracked.int32(100)
)

process.load("L1Trigger.L1CaloTrigger.l1tGCTSumFileInputProducer_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2L1GCTSumEmulator_cfi")
process.load("L1Trigger.L1CaloPhase2Analyzer.l1TGCTSumAnalyzer_cfi")

process.gctSumFileInputProducer.inputFile = cms.string(options.inputFile)
process.gctSumFileInputProducer.posOffset = cms.uint32(options.posOffset)
process.gctSumFileInputProducer.negOffset = cms.uint32(options.negOffset)
process.gctSumFileInputProducer.eventOffset = cms.uint32(options.eventOffset)
process.gctSumFileInputProducer.wrapAround = cms.bool(options.wrapAround)
process.gctSumFileInputProducer.debug = cms.bool(False)

replayInputTags = cms.VInputTag(
    *[
        cms.InputTag("gctSumFileInputProducer", f"LinkIn{i}")
        for i in range(24)
    ]
)

process.phase2L1GCTSumEmulator.debug = cms.bool(False)
process.phase2L1GCTSumEmulator.inputLinks = replayInputTags

process.l1TGCTSumAnalyzer.debug = cms.untracked.bool(False)
process.l1TGCTSumAnalyzer.sumInputDumpFile = cms.untracked.string(options.sumInputDumpFile)
process.l1TGCTSumAnalyzer.cmsswGtOutputDumpFile = cms.untracked.string(options.gtOutputDumpFile)
process.l1TGCTSumAnalyzer.inputLinks = replayInputTags
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
    process.gctSumFileInputProducer *
    process.phase2L1GCTSumEmulator *
    process.l1TGCTSumAnalyzer
)

process.schedule = cms.Schedule(process.GCTSUM)

process.options.numberOfThreads = cms.untracked.uint32(1)
process.options.numberOfStreams = cms.untracked.uint32(1)
process.options.wantSummary = cms.untracked.bool(True)

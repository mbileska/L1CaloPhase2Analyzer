# Analyzer for the distributed-trigger proof of concept

This branch pairs with `mbileska/cmssw:dist-trigger-poc`. It is separate from
the earlier `cmssw20-mhh-gcts-workflow`, whose campaign scripts intentionally
use zero-filled GCTB sources. The CMSSW branch owns the full detector-stage
configuration, GCTB-to-GCTS adapter, stage TTree, Condor submission script,
and detailed [decision log](https://github.com/mbileska/cmssw/blob/dist-trigger-poc/L1Trigger/L1CaloTrigger/test/DIST_TRIGGER_POC.md).

## What changed here

`python/mhhGCTSFullCaloTrainingNtuplizer_cfi.py` clones the existing ABI-v2
training analyzer with all 24 GCTS source tags explicit: for each eta side,
three MHH links followed by three links from each of GCTB cards 1, 2, and 3.
It selects the full GCTS side/GT outputs and records `gctbPresent=true`,
`gctbSourceMask=7`, and `gctsInputDerived=false`. The original smoke cfi is
unchanged and continues to encode its zero-filled barrel inputs honestly.

`test/check_mhh_gcts_training_ntuple.py --expect-full-gctb` checks that every
event has the full-barrel metadata, that MHH outputs 0..2 match the first
three GCTS inputs on each side, and that at least one GCTB input word is
nonzero across the sample. It is mutually exclusive with the existing
`--expect-zero-gctb` mode. The CMSSW branch also checks four-source to GCTS
sum conservation and the synchronized RCT/GCTB stage tree.

## Data contract

The analyzer writes one `mhhGCTSTrainingNtuplizer/trainingTree` entry per EDM
event with raw 9-word links, decoded object/sum integer codes, physical-unit
views, generator truth, and provenance. The paired CMSSW stage tree adds RCT
and GCTB Pre/Post frames and decoded RCT tower, GCTB supertower, EG, jet, and
tau ET/pT. Both trees are written to one ROOT file by
`runFullCaloMHHGCTSTraining_cfg.py` in the CMSSW branch.

The GCTB adapter remains a documented proof-of-concept translation: iso-EG
slots are zero, barrel sums are derived from supertowers and selected jets,
and detector-wide object phi alignment is not yet established. Raw local
coordinates and links are retained for validation and redecoding. Do not
interpret these training fields as final certified trigger quantities until
the CMSSW branch's runtime and geometry gates pass.

## Workflow

Check out this branch at `src/L1Trigger/L1CaloPhase2Analyzer` and the paired
CMSSW branch at `src/L1Trigger/L1CaloTrigger` in a
`CMSSW_20_0_0_patch2` area. Build and run the 10-event Condor gate described
in the paired decision log. Its runner uses the full-calorimeter preset and
the new checker mode. The old `submit_mhh_gcts_campaign.sh` remains the
endcap-only smoke workflow and must not be used for full-barrel production.

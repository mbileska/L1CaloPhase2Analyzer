# CMSSW 20 MHH-to-GCTS Phase2Fall26 MC workflow

This page records the earlier zero-GCTB smoke campaign. For the
`dist-trigger-poc` full-calorimeter branch, use
[DIST_TRIGGER_POC_ANALYZER.md](DIST_TRIGGER_POC_ANALYZER.md) and the paired
CMSSW branch's decision log and Condor scripts.

This document freezes the first successful end-to-end software integration
run and defines the gates for scaling it into a training-data production.  It
is intentionally explicit about what is real, what is provisional, and what
is still zero-filled.

## What succeeded

On 2026-11-03, the clean MHH/GCTS chain processed ten events from centrally
produced CMS simulation:

```text
/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/Phase2Fall26DIGI-DIGI_pilot_200X_mcRun4_realistic_v1-v3/GEN-SIM-DIGI-RAW
```

The selected LFN was:

```text
/store/mc/Phase2Fall26DIGI/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/GEN-SIM-DIGI-RAW/DIGI_pilot_200X_mcRun4_realistic_v1-v3/2560000/a01895fa-67ee-4f7d-9c71-c68cb648e3c5.root
```

The working environment was:

- CMSSW `20_0_0_patch2`;
- local CMSSW branch `emulator-gcts-mhh-cmssw20`;
- clean-emulator source branch `mbileska/cmssw:emulator-gcts-mhh-clean`;
- XRootD input through `root://cmsxrootd.fnal.gov/`;
- a local Wisconsin AVX2 worker (`qis1.hep.wisc.edu` in the reference run);
- process name `MHHGCTS`.

The `cmsRun` exit status was zero.  The output
`mhh_gctsum_dy_10events_edm_numEvent10.root` contains one run, one luminosity
block, and ten `edm::Event` records with event numbers:

```text
1003 1001 1004 1002 1005 1006 1007 1009 1010 1008
```

### What “ten events” means

It means ten simulated proton-proton collision records from one
`GEN-SIM-DIGI-RAW` input file were read by CMSSW and passed through the
software chain.  The result is an **EDM ROOT file**, not yet a flat training
ntuple.  Each event contains raw nine-word link products such as
`vector<unsigned long>` plus selected generator truth.  It is not ten input
files, ten datasets, collision data, or ten synthetic firmware vectors.

The successful EDM retained:

```text
HF adapters:   l1tPhase2L1HFMHHLinkProducer{Positive,Negative}:LinkIn0..2
DMX adapters:  l1tPhase2L1DMXEmulator{Positive,Negative}:LinkIn0..15
MHH:           l1tPhase2L1MHHFromHFAndDMX{Positive,Negative}:LinkOut0..5
GCTS sides:    l1tPhase2L1GCTSumFromMHHSmoke:SumLinkOut0..5
GCTS to GT:    l1tPhase2L1GCTSumFromMHHSmoke:LinkOut0..5
truth:         ak4GenJetsNoNu and genMetTrue
```

The HCAL trigger primitives and HGCAL tower/multicluster products were real
stored products from the simulated input.  The HF and DMX adapters that turn
them into MHH link words are provisional and are not yet subsystem-certified.

## Critical limitation: GCTB is not connected

The current smoke configuration passes the real MC-derived MHH triplet to
GCTS, but supplies zero words for all three GCTB triplets on both eta sides:

```text
real HCAL/HGCAL simulation -> provisional HF/DMX -> MHH --+
                                                          +-> GCTS SUM -> TO_GT
GCTB triplet 1 = zero ------------------------------------+
GCTB triplet 2 = zero ------------------------------------+
GCTB triplet 3 = zero ------------------------------------+
```

Zero is a physically meaningful value, so a training dataset must never imply
that these placeholders are measured empty barrel links.  Until real GCTB is
connected, every ntuple event must carry at least:

```text
gctbPresent = false
gctbSourceMask = 0
```

The current result is therefore an **MHH-to-GCTS software smoke test**, not a
complete barrel+endcap trigger integration.

## Why the earlier attempts failed

The failures had several independent causes:

1. Phase2Fall26 uses CMSSW-20-era persisted dictionaries.  CMSSW 15 rejected
   the input with a `Phase2TrackerRecHit1D` checksum mismatch.  The solution
   was a selective port of the clean MHH/GCTS code to `CMSSW_20_0_0_patch2`.
2. The Wisconsin login nodes used during debugging could not execute the
   CMSSW-20 `libActsCore.so` build and raised `SIGILL`.  A tmux session does not
   change the CPU.  Running on an AVX2-capable Condor worker fixed this.
3. Direct DAVS opened the file but later failed in `Davix::copyBytes`.
   XRootD through CMS AAA worked.
4. The first CMSSW configuration omitted the HCAL TPG transcoder and then the
   trigger-tower geometry.  The successful configuration contains both:

   ```python
   process.load("CalibCalorimetry.CaloTPG.CaloTPGTranscoder_cfi")
   process.HcalTrigTowerGeometryESProducer = cms.ESProducer(
       "HcalTrigTowerGeometryESProducer"
   )
   ```

5. Off-site Condor workers could not see the custom AFS CMSSW area.  Jobs must
   be restricted to modern `hep.wisc.edu` workers unless the CMSSW area is
   packaged and transferred.
6. Returning Condor output directly into AFS failed with a transfer permission
   error.  `/scratch/$USER/<unique-production>` works as `initialdir`; small
   provenance files can be copied to backed-up AFS afterward.
7. `VarParsing("analysis")` decorated the requested output name with
   `_numEvent10`.  The wrapper incorrectly checked only the undecorated name.
   Production wrappers must discover the actual output by a constrained glob.

The generic ROOT footer that suggested filing a ROOT bug was not evidence of
file corruption.  The stack trace ended in the `libActsCore` initializer on an
old CPU, while the same file was read successfully on an AVX2 worker.

## Gate 1: freeze the exact CMSSW implementation

Before producing more events, commit and push all of the following on a
reviewable CMSSW-20 branch:

- the four producer plugins;
- the sums-ABI-v2 headers used by those plugins;
- `l1tPhase2L1MHHGCTSumSmoke_cff.py`;
- the exact `runMHHGCTSumDY_cfg.py` that succeeded;
- the two firmware-equivalence tests;
- the CMSSW-20 ntuplizer and its output checker.

Record the state of the successful local area before changing it:

```bash
CMSSW_SRC="$HOME/CMSSW_20_0_0_patch2_MHHGCTS/src"
FREEZE_DIR="$HOME/mhhgcts_results/20261103_dy_10event_smoke"

mkdir -p "$FREEZE_DIR"
git -C "$CMSSW_SRC" rev-parse HEAD > "$FREEZE_DIR/CMSSW_GIT_HEAD.txt"
git -C "$CMSSW_SRC" status --short > "$FREEZE_DIR/CMSSW_GIT_STATUS.txt"
git -C "$CMSSW_SRC" diff > "$FREEZE_DIR/CMSSW_GIT_DIFF.patch"
cp -p "$CMSSW_SRC/dy_mhh_gcts_test/runMHHGCTSumDY_cfg.py" "$FREEZE_DIR/"
(
  cd "$FREEZE_DIR"
  find . -maxdepth 1 -type f ! -name SHA256SUMS -print0 \
    | sort -z \
    | xargs -0 sha256sum \
    > SHA256SUMS
)
```

Do not try to scale from an undocumented dirty CMSSW area: a later result
would not be reproducible.

## Gate 2: validate the ten-event reference file

Run FWLite only under the CMSSW-20 runtime on an AVX2 worker.  The validator
`test/validate_mhh_gcts_edm.py` checks every event, not only the three events
that were visually inspected.  It checks:

- all required HF, DMX, MHH, side-GCTS, and final-GCTS products exist;
- every raw link contains exactly nine 64-bit words;
- MHH links 3--5 and all reserved sum words are zero;
- in placeholder mode, each GCTS side sum equals the corresponding MHH sum;
- final MET/MHT magnitudes, phi, ET/HT, and object count exactly match sums ABI
  v2, including the integer CORDIC and saturation;
- all ten reference event IDs are present in the frozen order;
- the sample contains nonzero MHH and GT payloads.

Command:

```bash
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd "$HOME/CMSSW_20_0_0_patch2_MHHGCTS/src"
eval "$(scram runtime -sh)"

python3 /path/to/L1CaloPhase2Analyzer/test/validate_mhh_gcts_edm.py \
  "$HOME/mhhgcts_results/20261103_dy_10event_smoke/mhh_gctsum_dy_10events_edm_numEvent10.root" \
  --expected-events 10 \
  --expected-event-ids 1003,1001,1004,1002,1005,1006,1007,1009,1010,1008 \
  --expect-zero-gctb \
  --require-nonzero
```

On an old login node this may raise `SIGILL`; that is why this command belongs
inside an AVX2-restricted Condor validation job or on a known compatible host.
Do not scale until it prints both `PASS` lines.

## Gate 3: build and validate the CMSSW-20 training ntuplizer

The existing `L1TGCTSumAnalyzer` predates this CMSSW-20 integration and must
not be reused unchanged.  The `cmssw20-mhh-gcts-workflow` branch provides the
ABI-v2-aware `MHHGCTSTrainingNtuplizer`, its CMSSW configuration, a TTree
checker, and a Wisconsin Condor submission helper.

The CMSSW-20 plugin BuildFile compiles only this ntuplizer.  Legacy RCT
analyzers in the same source directory include interfaces removed from this
release; they remain available on their original CMSSW-15-era branches and
are deliberately excluded from this build.

Use fixed-shape unsigned 64-bit raw branches:

```text
hf_words[2][3][9]
dmx_words[2][16][9]
mhh_words[2][6][9]
gcts_input_words[2][12][9]
gcts_sum_words[2][3][9]
gcts_gt_words[6][9]
```

The first side index is positive eta and the second is negative eta.  The
object arrays use `[type][side][rank]`, where type is
`[EG, EGIso, Jet, Tau]`, side is `[positive eta, negative eta]`, and rank is
`0..5`.  The implementation also stores:

```text
run, lumi, event, productPresenceMask
dataset, source LFN, input EDM, CMSSW release, GlobalTag
CMSSW commit, Analyzer commit, emulator/ntuple configuration hashes
gctbPresent, gctbSourceMask, gctsInputDerived
decoded MHH objects and source sums (integer codes and GeV/radian views)
decoded GCTS side objects and sums (integer codes and GeV/radian views)
decoded final GT objects plus MET, MHT, ET, HT and NObj
generator jets and generator MET
```

The packed-object decoder follows the frozen interface:

- source energy LSB: `0.5 GeV`;
- GT energy LSB: `1/32 GeV`;
- GT phi LSB: `pi/4096`;
- MHH gamma phi: 360-bin view; MHH jet/tau phi: 24-bin view;
- `mhhObjectSourceFlag` is the MHH adapter-source bit (`0=HF`, `1=HGCAL`),
  not a barrel/endcap flag;
- `gctsObjectIsBarrel` is the classification after GCTS merges its inputs;
- `gctsObjectEta` applies the packet-side sign and the firmware conversion
  `hwEta * 23 * gtEtaLSB`;
- GT object eta is an unsigned packed magnitude.  `gtObjectEta` applies the
  sign from the positive/negative packet slot; the configurable magnitude LSB
  is stored as `gtEtaLSB`.

The baseline non-sum object coordinate conversion has not been newly
physics-certified by the sums-ABI-v2 work, so retain both the raw word and
integer code.  Never train solely on the floating view without freezing and
reviewing these scale parameters.

The frozen reference EDM contains real MC-derived HF and DMX adapter links but
does **not** retain the original HCAL trigger primitives or HGCAL
tower/multicluster collections.  Therefore this first ntuple stores every
available packed stage, not the direct detector-subsystem collections.  To use
those collections as distributed-training features, extend the first-stage
EDM output and ntuplizer explicitly; do not reconstruct or label them as raw
detector inputs from the adapter words.

The two outputs serve different purposes:

- `*_edm.root`: exact CMSSW replay/debug/provenance and raw products;
- `*_training.root`: a simple TTree or RNTuple readable by uproot/Awkward and
  PyTorch data loaders.

Do not replace the EDM with the ntuple.  Retain at least a prescaled replay
EDM sample so unexpected training examples can be reproduced.

### Install, build and submit the ten-event conversion

On `login.hep.wisc.edu`, after entering the existing CMSSW-20 area:

```bash
set -euo pipefail

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd "$HOME/CMSSW_20_0_0_patch2_MHHGCTS/src"
eval "$(scram runtime -sh)"

ANALYZER="$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer"
if test -d "$ANALYZER/.git"; then
  git -C "$ANALYZER" fetch origin
  git -C "$ANALYZER" switch cmssw20-mhh-gcts-workflow
  git -C "$ANALYZER" pull --ff-only origin cmssw20-mhh-gcts-workflow
else
  git clone --branch cmssw20-mhh-gcts-workflow \
    https://github.com/mbileska/L1CaloPhase2Analyzer.git \
    "$ANALYZER"
fi

cd "$CMSSW_BASE/src"
scram b -j 8

cd "$ANALYZER"
./test/submit_mhh_gcts_training_ntuple.sh \
  --input-edm "$HOME/mhhgcts_results/20261103_dy_10event_smoke/mhh_gctsum_dy_10events_edm_numEvent10.root" \
  --dataset '/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/Phase2Fall26DIGI-DIGI_pilot_200X_mcRun4_realistic_v1-v3/GEN-SIM-DIGI-RAW' \
  --source-lfn '/store/mc/Phase2Fall26DIGI/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/GEN-SIM-DIGI-RAW/DIGI_pilot_200X_mcRun4_realistic_v1-v3/2560000/a01895fa-67ee-4f7d-9c71-c68cb648e3c5.root' \
  --max-events 10
```

The helper records the exact commits and configuration hashes, creates a
unique `/scratch/$USER/mhhgcts_training_<timestamp>` directory, and submits
one AVX2-restricted job.  It deliberately does **not** launch a campaign.
Monitor and validate it from the same login scheduler:

```bash
TRAIN_DIR="$(cat "$HOME/latest_mhhgcts_training_dir.txt")"
condor_q "$USER"
tail -100 "$TRAIN_DIR/logs/condor.log"
tail -100 "$TRAIN_DIR/logs/job.out"
cat "$TRAIN_DIR/logs/job.err"

test -f "$TRAIN_DIR/SUCCESS"
cat "$TRAIN_DIR/training_validation.txt"
ls -lh "$TRAIN_DIR/mhh_gcts_training.root"
sha256sum -c "$TRAIN_DIR/SHA256SUMS"
```

The gate passes only when the checker reports the expected ten entries,
required schema, self-consistent unit conversions and explicit zero-GCTB
metadata.

## Gate 4: one complete input file

After the ten-event validator and ntuplizer pass, process the entire first LFN
with `maxEvents=-1`.  Compare the EDM and training-ntuple entry counts, run the
ABI validator over every event, inspect missing-product counters, and measure
runtime/output size.  Those measurements determine memory, disk, and job
splitting for the dataset production.

The committed campaign helper implements Gates 4 and 5.  Start with two
complete input files:

```bash
cd "$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer"
./test/submit_mhh_gcts_campaign.sh \
  --dataset '/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/Phase2Fall26DIGI-DIGI_pilot_200X_mcRun4_realistic_v1-v3/GEN-SIM-DIGI-RAW' \
  --max-events -1 \
  --limit-files 2
```

Every successful shard produces two complementary files:

- `mhh_gcts_NNNN_edm.root`: replayable CMSSW EDM containing the HCAL trigger
  primitives and HGCAL tower/multicluster inputs, retained HF/DMX adapter
  links, MHH, GCTS-side and final-GT products plus generator truth;
- `mhh_gcts_NNNN_training.root`: flat decoded training tree containing the
  raw HF, DMX, MHH, reconstructed GCTS input, GCTS-side and final-GT words,
  decoded objects/sums, generator truth and provenance.

The helper validates the ABI-v2 EDM, validates every packed-to-decoded record,
requires matching EDM/ntuple event counts, and writes per-shard manifests and
checksums.  Separate ROOT files for each individual module would duplicate the
same event metadata and complicate alignment, so stages are branches/products
inside these two synchronized files instead.

The campaign EDM uses `test/run-mhh-gcts-campaign-edm.py` as a thin overlay on
the proven integration configuration.  It explicitly keeps
`simHcalTriggerPrimitiveDigis`, `l1tHGCalTowerProducer`, and
`l1tHGCalBackEndLayer2Producer`; every shard checks that all three labels are
present before validation can pass.

After both full-file jobs pass, submit the complete DAS file list by changing
only `--limit-files 0` and using a new campaign tag.

When the queue is empty, summarize the pilot without loading the ROOT event
data on the login node:

```bash
CAMPAIGN_DIR="$(cat "$HOME/latest_mhhgcts_campaign_dir.txt")"
python3 "$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer/test/check_mhh_gcts_campaign.py" \
  "$CAMPAIGN_DIR"
```

This requires every success marker, manifest, ROOT pair, validation record and
checksum list, verifies that stderr is empty, and writes
`campaign_manifest.csv`.

## Gate 5: one input file per Condor job

The following creates the production input list on `login.hep.wisc.edu`:

```bash
set -euo pipefail

export DATASET='/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/Phase2Fall26DIGI-DIGI_pilot_200X_mcRun4_realistic_v1-v3/GEN-SIM-DIGI-RAW'
export PROD_TAG="mhhgcts_phase2fall26_$(date +%Y%m%d_%H%M%S)"
export PROD_DIR="/scratch/$USER/$PROD_TAG"

mkdir -p "$PROD_DIR/logs"
dasgoclient --query="file dataset=${DATASET}" | sort -u > "$PROD_DIR/lfns.txt"
test -s "$PROD_DIR/lfns.txt"
awk '{printf "%d %s\n", NR-1, $0}' "$PROD_DIR/lfns.txt" > "$PROD_DIR/jobs.txt"
wc -l "$PROD_DIR/lfns.txt"
```

Create `$PROD_DIR/run_one_file.sh` with this content and make it executable:

```bash
#!/usr/bin/env bash
set -euo pipefail

job_index="$1"
input_lfn="$2"
max_events="${3:--1}"
dataset='/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/Phase2Fall26DIGI-DIGI_pilot_200X_mcRun4_realistic_v1-v3/GEN-SIM-DIGI-RAW'
cmssw_src="/afs/hep.wisc.edu/user/mbileska/CMSSW_20_0_0_patch2_MHHGCTS/src"
cfg="$cmssw_src/dy_mhh_gcts_test/runMHHGCTSumDY_cfg.py"
validator="$cmssw_src/L1Trigger/L1CaloPhase2Analyzer/test/validate_mhh_gcts_edm.py"
analyzer_dir="$cmssw_src/L1Trigger/L1CaloPhase2Analyzer"
ntuple_cfg="$analyzer_dir/test/run-mhh-gcts-training-ntuple.py"
ntuple_checker="$analyzer_dir/test/check_mhh_gcts_training_ntuple.py"
input_pfn="root://cmsxrootd.fnal.gov/${input_lfn}"

test -d "$cmssw_src" || { echo "Missing CMSSW area: $cmssw_src"; exit 2; }
test -f "$cfg" || { echo "Missing integration configuration: $cfg"; exit 3; }
test -f "$validator" || { echo "Missing ABI-v2 validator: $validator"; exit 4; }
test -f "$ntuple_cfg" || { echo "Missing training ntuple configuration: $ntuple_cfg"; exit 4; }
test -f "$ntuple_checker" || { echo "Missing training ntuple checker: $ntuple_checker"; exit 4; }

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd "$cmssw_src"
eval "$(scram runtime -sh)"
cd "$_CONDOR_SCRATCH_DIR"

printf '%s\n' "$input_lfn" > "input_${job_index}.lfn"
git -C "$cmssw_src" rev-parse HEAD > "cmssw_head_${job_index}.txt"
git -C "$cmssw_src" status --short > "cmssw_status_${job_index}.txt"

set +e
cmsRun "$cfg" \
  inputFiles="$input_pfn" \
  maxEvents="$max_events" \
  outputFile="file:mhh_gcts_${job_index}_edm.root" \
  2>&1 | tee "cmsrun_${job_index}.log"
cmsrun_status=${PIPESTATUS[0]}
set -e
test "$cmsrun_status" -eq 0 || exit "$cmsrun_status"

edm_output="$(find . -maxdepth 1 -type f \
  -name "mhh_gcts_${job_index}_edm*.root" -size +100k -print -quit)"
test -n "$edm_output" || exit 20

edmFileUtil -e "$edm_output" > "events_${job_index}.txt"
edmDumpEventContent "$edm_output" > "content_${job_index}.txt"
python3 "$validator" "$edm_output" --expect-zero-gctb --require-nonzero \
  > "validation_${job_index}.txt"

cmssw_commit="$(git -C "$cmssw_src" rev-parse HEAD)"
analyzer_commit="$(git -C "$analyzer_dir" rev-parse HEAD)"
emulator_cfg_hash="$(sha256sum "$cfg" | awk '{print $1}')"
ntuple_cfg_hash="$(sha256sum "$ntuple_cfg" | awk '{print $1}')"

cmsRun "$ntuple_cfg" \
  inputFiles="file:$PWD/${edm_output#./}" \
  maxEvents="$max_events" \
  ntupleFile="mhh_gcts_${job_index}_training.root" \
  dataset="$dataset" \
  sourceLFN="$input_lfn" \
  inputEDM="$edm_output" \
  cmsswRelease="CMSSW_20_0_0_patch2" \
  globalTag="200X_mcRun4_realistic_v1" \
  cmsswCommit="$cmssw_commit" \
  analyzerCommit="$analyzer_commit" \
  emulatorConfigurationHash="$emulator_cfg_hash" \
  ntupleConfigurationHash="$ntuple_cfg_hash" \
  2>&1 | tee "ntuple_${job_index}.log"

checker_args=(--expect-zero-gctb --require-truth --require-nonzero)
if (( max_events >= 0 )); then
  checker_args+=(--expected-events "$max_events")
fi
python3 "$ntuple_checker" "mhh_gcts_${job_index}_training.root" \
  "${checker_args[@]}" > "ntuple_validation_${job_index}.txt"

sha256sum "$edm_output" \
  "mhh_gcts_${job_index}_training.root" \
  "cmsrun_${job_index}.log" \
  "ntuple_${job_index}.log" \
  "events_${job_index}.txt" \
  "content_${job_index}.txt" \
  "validation_${job_index}.txt" \
  "ntuple_validation_${job_index}.txt" \
  > "SHA256SUMS_${job_index}"
```

Create `$PROD_DIR/submit.sub`:

```text
universe = vanilla
initialdir = /scratch/mbileska/REPLACE_WITH_PRODUCTION_TAG
executable = run_one_file.sh
arguments = $(job_index) $(input_lfn) -1

output = logs/job_$(job_index).out
error  = logs/job_$(job_index).err
log    = logs/condor.log

should_transfer_files = YES
when_to_transfer_output = ON_EXIT
use_x509userproxy = True

request_cpus = 1
request_memory = 4000MB
request_disk = 10000MB

requirements = (TARGET.has_avx2 =?= True) && regexp("hep[.]wisc[.]edu$", TARGET.Machine)

queue job_index,input_lfn from jobs.txt
```

Replace the literal `initialdir`, then submit from the same Wisconsin access
point on which `/scratch/$USER/$PROD_TAG` was created:

```bash
voms-proxy-init --voms cms --valid 192:00
voms-proxy-info -timeleft

cd "$PROD_DIR"
sed -i "s|/scratch/mbileska/REPLACE_WITH_PRODUCTION_TAG|$PROD_DIR|" submit.sub
condor_submit submit.sub | tee submission.txt
condor_q "$USER"
```

This dataset had ten files when the reference run was made, so this layout
would create ten independent jobs.  Query DAS again for every production;
never hard-code the old file count.

### Staged submission

Do not submit a much larger campaign immediately.  Use these stages:

1. ten events from one file;
2. all events from one file;
3. two files/jobs;
4. all files in this DY dataset;
5. broader QCD, minimum-bias/pileup, high-pT, and signal samples.

At each stage require zero held/failed jobs, validator success for every
shard, matching EDM/ntuple event counts, and complete manifests.

## Gate 6: collect without losing provenance

`/scratch` is temporary and not backed up.  Large ROOT files should go to the
approved Wisconsin user-data storage after quota and path are confirmed; do
not fill AFS with a training campaign.  Keep the small configuration,
manifests, logs, checksums, file list, validation summaries, and git state in a
backed-up result directory.

Create a campaign manifest containing one row per shard:

```text
dataset,input_lfn,output_edm,output_training,event_count,cmssw_release,
global_tag,cmssw_commit,cfg_sha256,edm_sha256,training_sha256,job_id,status,
gctb_present
```

Split train/validation/test by input file or physics sample, not by randomly
mixing events from the same file after production.  That makes leakage and
reproduction easier to control.

## Definition of “correct enough to scale”

A campaign is allowed past the reference stage only when all of these are
true:

- the clean MHH and GCTS firmware-equivalence tests pass;
- the exact CMSSW-20 code and configuration are committed;
- the ten-event file passes `validate_mhh_gcts_edm.py` for all ten events;
- one complete input file passes the same validator;
- every output records that GCTB is absent while it is zero-filled;
- EDM and training-ntuple event counts agree;
- no required product is missing and no event is silently skipped;
- every output and configuration has a checksum and manifest entry;
- output storage and retention are defined before submission.

Connecting real GCTB inputs is a separate required gate before the dataset can
be described as full barrel+MHH GCTS training data.

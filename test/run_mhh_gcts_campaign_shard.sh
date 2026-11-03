#!/usr/bin/env bash
set -euo pipefail

if (($# != 7)); then
  echo "Usage: $0 JOB_INDEX INPUT_LFN MAX_EVENTS DATASET CMSSW_SRC GLOBAL_TAG EMULATOR_CFG" >&2
  exit 2
fi

job_index="$1"
input_lfn="$2"
max_events="$3"
dataset="$4"
cmssw_src="$5"
global_tag="$6"
emulator_cfg="$7"

printf -v shard "%04d" "$job_index"
analyzer_dir="$cmssw_src/L1Trigger/L1CaloPhase2Analyzer"
edm_validator="$analyzer_dir/test/validate_mhh_gcts_edm.py"
ntuple_cfg="$analyzer_dir/test/run-mhh-gcts-training-ntuple.py"
ntuple_checker="$analyzer_dir/test/check_mhh_gcts_training_ntuple.py"
input_pfn="root://cmsxrootd.fnal.gov/${input_lfn}"

for path in "$cmssw_src" "$analyzer_dir"; do
  test -d "$path" || { echo "Missing directory: $path" >&2; exit 3; }
done
for path in "$emulator_cfg" "$edm_validator" "$ntuple_cfg" "$ntuple_checker"; do
  test -f "$path" || { echo "Missing required file: $path" >&2; exit 4; }
done

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd "$cmssw_src"
eval "$(scram runtime -sh)"
cd "${_CONDOR_SCRATCH_DIR:-$PWD}"

grep -qm1 avx2 /proc/cpuinfo || { echo "Worker does not advertise AVX2" >&2; exit 10; }
voms-proxy-info -exists -valid 0:30 || { echo "CMS proxy is missing or nearly expired" >&2; exit 11; }

edm_output="mhh_gcts_${shard}_edm.root"
training_output="mhh_gcts_${shard}_training.root"

{
  printf 'job_index=%s\n' "$job_index"
  printf 'input_lfn=%s\n' "$input_lfn"
  printf 'input_pfn=%s\n' "$input_pfn"
  printf 'dataset=%s\n' "$dataset"
  printf 'max_events=%s\n' "$max_events"
  printf 'host=%s\n' "$(hostname -f)"
  printf 'started_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  printf 'cmssw_base=%s\n' "$CMSSW_BASE"
  printf 'scram_arch=%s\n' "$SCRAM_ARCH"
  printf 'global_tag=%s\n' "$global_tag"
  printf 'cmssw_commit=%s\n' "$(git -C "$cmssw_src" rev-parse HEAD)"
  printf 'analyzer_commit=%s\n' "$(git -C "$analyzer_dir" rev-parse HEAD)"
  printf 'emulator_cfg_sha256=%s\n' "$(sha256sum "$emulator_cfg" | awk '{print $1}')"
  printf 'ntuple_cfg_sha256=%s\n' "$(sha256sum "$ntuple_cfg" | awk '{print $1}')"
} > "provenance_${shard}.txt"

git -C "$cmssw_src" status --short > "cmssw_status_${shard}.txt"
git -C "$analyzer_dir" status --short > "analyzer_status_${shard}.txt"

echo "Running emulator chain for shard $shard"
cmsRun "$emulator_cfg" \
  inputFiles="$input_pfn" \
  maxEvents="$max_events" \
  outputFile="file:$edm_output" \
  2>&1 | tee "emulator_${shard}.log"

# VarParsing may decorate the EDM filename when maxEvents is non-default.
actual_edm="$(find . -maxdepth 1 -type f -name "mhh_gcts_${shard}_edm*.root" -size +100k -print -quit)"
test -n "$actual_edm" || { echo "No nontrivial replay EDM was produced" >&2; exit 20; }
if test "${actual_edm#./}" != "$edm_output"; then
  mv "$actual_edm" "$edm_output"
fi

edmFileUtil -e "$edm_output" > "edm_events_${shard}.txt"
edmDumpEventContent "$edm_output" > "edm_content_${shard}.txt"
python3 "$edm_validator" "$edm_output" \
  --expect-zero-gctb \
  --require-nonzero \
  | tee "edm_validation_${shard}.txt"

cmssw_commit="$(git -C "$cmssw_src" rev-parse HEAD)"
analyzer_commit="$(git -C "$analyzer_dir" rev-parse HEAD)"
emulator_cfg_hash="$(sha256sum "$emulator_cfg" | awk '{print $1}')"
ntuple_cfg_hash="$(sha256sum "$ntuple_cfg" | awk '{print $1}')"

echo "Writing decoded training ROOT for shard $shard"
cmsRun "$ntuple_cfg" \
  inputFiles="file:$PWD/$edm_output" \
  maxEvents=-1 \
  ntupleFile="$training_output" \
  dataset="$dataset" \
  sourceLFN="$input_lfn" \
  inputEDM="$edm_output" \
  cmsswRelease="CMSSW_20_0_0_patch2" \
  globalTag="$global_tag" \
  cmsswCommit="$cmssw_commit" \
  analyzerCommit="$analyzer_commit" \
  emulatorConfigurationHash="$emulator_cfg_hash" \
  ntupleConfigurationHash="$ntuple_cfg_hash" \
  2>&1 | tee "ntuple_${shard}.log"

python3 "$ntuple_checker" "$training_output" \
  --expected-edm "$edm_output" \
  --expect-zero-gctb \
  --require-truth \
  --require-nonzero \
  | tee "ntuple_validation_${shard}.txt"

event_count="$(python3 - "$training_output" <<'PY'
import sys
import ROOT

root_file = ROOT.TFile.Open(sys.argv[1])
tree = root_file.Get("mhhGCTSTrainingNtuplizer/trainingTree")
if not tree:
    raise SystemExit("missing training tree")
print(int(tree.GetEntries()))
PY
)"

sha256sum \
  "$edm_output" \
  "$training_output" \
  "emulator_${shard}.log" \
  "ntuple_${shard}.log" \
  "edm_validation_${shard}.txt" \
  "ntuple_validation_${shard}.txt" \
  > "SHA256SUMS_${shard}"

{
  printf 'job_index=%s\n' "$job_index"
  printf 'status=SUCCESS\n'
  printf 'event_count=%s\n' "$event_count"
  printf 'dataset=%s\n' "$dataset"
  printf 'input_lfn=%s\n' "$input_lfn"
  printf 'output_edm=%s\n' "$edm_output"
  printf 'output_training=%s\n' "$training_output"
  printf 'edm_bytes=%s\n' "$(stat -c %s "$edm_output")"
  printf 'training_bytes=%s\n' "$(stat -c %s "$training_output")"
  printf 'gctb_present=false\n'
  printf 'finished_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "manifest_${shard}.txt"

touch "SUCCESS_${shard}"
echo "SUCCESS: shard $shard produced replay EDM and decoded training ROOT for $event_count events"

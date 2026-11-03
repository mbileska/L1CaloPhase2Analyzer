#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: submit_mhh_gcts_training_ntuple.sh [options]

Submit an AVX2-restricted Wisconsin Condor job that converts an existing
MHH/GCTS replay EDM into the decoded training TTree and validates it.

Options:
  --input-edm PATH       Existing emulator EDM (required)
  --source-lfn LFN       Original Phase2Fall26 MC LFN (required)
  --dataset DATASET      Original dataset name (required)
  --max-events N         Events to convert (default: 10)
  --global-tag TAG       Emulator GlobalTag (default: 200X_mcRun4_realistic_v1)
  --emulator-cfg PATH    First-stage cmsRun configuration
  --tag NAME             Scratch production tag
  -h, --help             Show this help

Run this from the CMSSW_20_0_0_patch2 runtime on login.hep.wisc.edu.
USAGE
}

input_edm=""
source_lfn=""
dataset=""
max_events=10
global_tag="200X_mcRun4_realistic_v1"
emulator_cfg=""
production_tag="mhhgcts_training_$(date +%Y%m%d_%H%M%S)"

while (($#)); do
  case "$1" in
    --input-edm) input_edm="$2"; shift 2 ;;
    --source-lfn) source_lfn="$2"; shift 2 ;;
    --dataset) dataset="$2"; shift 2 ;;
    --max-events) max_events="$2"; shift 2 ;;
    --global-tag) global_tag="$2"; shift 2 ;;
    --emulator-cfg) emulator_cfg="$2"; shift 2 ;;
    --tag) production_tag="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

: "${CMSSW_BASE:?Run cmsenv (or eval \"\$(scram runtime -sh)\") in the CMSSW 20 area first}"
: "${SCRAM_ARCH:?SCRAM_ARCH is not set}"

for required in input_edm source_lfn dataset; do
  test -n "${!required}" || { echo "Missing --${required//_/-}" >&2; exit 2; }
done
test -s "$input_edm" || { echo "Input EDM is absent or empty: $input_edm" >&2; exit 3; }
[[ "$max_events" =~ ^-?[0-9]+$ ]] || { echo "Invalid --max-events: $max_events" >&2; exit 3; }

analyzer_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmssw_src="$CMSSW_BASE/src"
ntuple_cfg="$analyzer_dir/test/run-mhh-gcts-training-ntuple.py"
checker="$analyzer_dir/test/check_mhh_gcts_training_ntuple.py"
if test -z "$emulator_cfg"; then
  emulator_cfg="$cmssw_src/dy_mhh_gcts_test/runMHHGCTSumDY_cfg.py"
fi

test -f "$ntuple_cfg" || { echo "Missing ntuple configuration: $ntuple_cfg" >&2; exit 4; }
test -f "$checker" || { echo "Missing ntuple checker: $checker" >&2; exit 4; }
test -f "$emulator_cfg" || { echo "Missing emulator configuration: $emulator_cfg" >&2; exit 4; }
command -v condor_submit >/dev/null || { echo "condor_submit is not available" >&2; exit 5; }

work_dir="/scratch/$USER/$production_tag"
mkdir -p "$work_dir/logs"
chmod 700 "$work_dir"

cmssw_commit="$(git -C "$cmssw_src" rev-parse HEAD)"
analyzer_commit="$(git -C "$analyzer_dir" rev-parse HEAD)"
emulator_cfg_hash="$(sha256sum "$emulator_cfg" | awk '{print $1}')"
ntuple_cfg_hash="$(sha256sum "$ntuple_cfg" | awk '{print $1}')"

cat > "$work_dir/run_training.sh" <<RUNNER
#!/usr/bin/env bash
set -euo pipefail

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd '$cmssw_src'
eval "\$(scram runtime -sh)"
cd "\${_CONDOR_SCRATCH_DIR:-\$PWD}"

grep -qm1 avx2 /proc/cpuinfo || { echo 'Worker does not advertise AVX2' >&2; exit 10; }
uname -a | tee runtime.txt
grep -m1 'model name' /proc/cpuinfo | tee -a runtime.txt
printf 'CMSSW_BASE=%s\nSCRAM_ARCH=%s\n' "\$CMSSW_BASE" "\$SCRAM_ARCH" | tee -a runtime.txt

cmsRun '$ntuple_cfg' \
  inputFiles='file:$input_edm' \
  maxEvents='$max_events' \
  ntupleFile='mhh_gcts_training.root' \
  dataset='$dataset' \
  sourceLFN='$source_lfn' \
  inputEDM='$input_edm' \
  cmsswRelease='CMSSW_20_0_0_patch2' \
  globalTag='$global_tag' \
  cmsswCommit='$cmssw_commit' \
  analyzerCommit='$analyzer_commit' \
  emulatorConfigurationHash='$emulator_cfg_hash' \
  ntupleConfigurationHash='$ntuple_cfg_hash' \
  2>&1 | tee training_cmsrun.log

checker_args=(--expect-zero-gctb --require-truth --require-nonzero)
if (( $max_events >= 0 )); then
  checker_args+=(--expected-events '$max_events')
fi
python3 '$checker' mhh_gcts_training.root "\${checker_args[@]}" \
  | tee training_validation.txt

sha256sum mhh_gcts_training.root training_cmsrun.log training_validation.txt \
  runtime.txt > SHA256SUMS
touch SUCCESS
RUNNER
chmod 700 "$work_dir/run_training.sh"

cat > "$work_dir/submit.sub" <<SUBMIT
universe = vanilla
initialdir = $work_dir
executable = run_training.sh

output = logs/job.out
error = logs/job.err
log = logs/condor.log

should_transfer_files = YES
when_to_transfer_output = ON_EXIT

request_cpus = 1
request_memory = 4000MB
request_disk = 4000MB

requirements = (TARGET.has_avx2 =?= True) && regexp("hep[.]wisc[.]edu\$", TARGET.Machine)
on_exit_hold = (ExitBySignal == True) || (ExitCode != 0)

queue 1
SUBMIT

cat > "$work_dir/PROVENANCE.txt" <<PROVENANCE
dataset=$dataset
source_lfn=$source_lfn
input_edm=$input_edm
max_events=$max_events
global_tag=$global_tag
cmssw_base=$CMSSW_BASE
scram_arch=$SCRAM_ARCH
cmssw_commit=$cmssw_commit
analyzer_commit=$analyzer_commit
emulator_cfg=$emulator_cfg
emulator_cfg_sha256=$emulator_cfg_hash
ntuple_cfg=$ntuple_cfg
ntuple_cfg_sha256=$ntuple_cfg_hash
PROVENANCE

printf '%s\n' "$work_dir" > "$HOME/latest_mhhgcts_training_dir.txt"
(
  cd "$work_dir"
  condor_submit submit.sub | tee submission.txt
)

echo
echo "Submitted training-ntuple smoke test."
echo "Work directory: $work_dir"
echo "Monitor from this same login host with:"
echo "  condor_q \"\$USER\""
echo "  tail -f '$work_dir/logs/condor.log'"
echo "After completion:"
echo "  test -f '$work_dir/SUCCESS' && cat '$work_dir/training_validation.txt'"

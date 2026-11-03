#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: submit_mhh_gcts_campaign.sh [options]

Submit one AVX2-restricted Wisconsin Condor shard per Phase-2 MC input file.
Each shard writes a replay EDM with all packed stages and a decoded training
ROOT file with raw links, physical views, truth and provenance.

Options:
  --dataset DATASET       DAS dataset (required)
  --max-events N         Events per input file; -1 means all (default: -1)
  --limit-files N        Submit only the first N files; 0 means all (default: 2)
  --tag NAME             Campaign directory name
  --global-tag TAG       GlobalTag metadata (default: 200X_mcRun4_realistic_v1)
  --request-memory MB    Condor memory request (default: 4000)
  --request-disk MB      Condor disk request (default: 12000)
  --emulator-cfg PATH    First-stage cmsRun configuration
  -h, --help             Show this help

Run this from the CMSSW_20_0_0_patch2 runtime on login.hep.wisc.edu.
USAGE
}

dataset=""
max_events=-1
limit_files=2
tag="mhhgcts_campaign_$(date +%Y%m%d_%H%M%S)"
global_tag="200X_mcRun4_realistic_v1"
request_memory=4000
request_disk=12000
emulator_cfg=""

while (($#)); do
  case "$1" in
    --dataset) dataset="$2"; shift 2 ;;
    --max-events) max_events="$2"; shift 2 ;;
    --limit-files) limit_files="$2"; shift 2 ;;
    --tag) tag="$2"; shift 2 ;;
    --global-tag) global_tag="$2"; shift 2 ;;
    --request-memory) request_memory="$2"; shift 2 ;;
    --request-disk) request_disk="$2"; shift 2 ;;
    --emulator-cfg) emulator_cfg="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

: "${CMSSW_BASE:?Initialize the CMSSW 20 runtime first}"
: "${SCRAM_ARCH:?SCRAM_ARCH is not set}"
test -n "$dataset" || { echo "Missing --dataset" >&2; exit 2; }
[[ "$max_events" =~ ^-?[0-9]+$ ]] || { echo "Invalid --max-events: $max_events" >&2; exit 2; }
[[ "$limit_files" =~ ^[0-9]+$ ]] || { echo "Invalid --limit-files: $limit_files" >&2; exit 2; }
[[ "$request_memory" =~ ^[0-9]+$ ]] || { echo "Invalid --request-memory" >&2; exit 2; }
[[ "$request_disk" =~ ^[0-9]+$ ]] || { echo "Invalid --request-disk" >&2; exit 2; }

command -v dasgoclient >/dev/null || { echo "dasgoclient is unavailable" >&2; exit 3; }
command -v condor_submit >/dev/null || { echo "condor_submit is unavailable" >&2; exit 3; }
voms-proxy-info -exists -valid 1:00 || {
  echo "Create a CMS proxy valid for at least one hour before submitting" >&2
  exit 4
}

analyzer_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmssw_src="$CMSSW_BASE/src"
runner="$analyzer_dir/test/run_mhh_gcts_campaign_shard.sh"
campaign_cfg="$analyzer_dir/test/run-mhh-gcts-campaign-edm.py"
if test -z "$emulator_cfg"; then
  emulator_cfg="$cmssw_src/dy_mhh_gcts_test/runMHHGCTSumDY_cfg.py"
fi
for path in "$runner" "$emulator_cfg" "$campaign_cfg"; do
  test -f "$path" || { echo "Missing required file: $path" >&2; exit 5; }
done

campaign_dir="/scratch/$USER/$tag"
test ! -e "$campaign_dir" || { echo "Campaign directory already exists: $campaign_dir" >&2; exit 6; }
mkdir -p "$campaign_dir/logs" "$campaign_dir/frozen_configs"
chmod 700 "$campaign_dir" "$campaign_dir/logs"

echo "Querying DAS for input files..."
dasgoclient --query="file dataset=${dataset}" | sort -u > "$campaign_dir/all_lfns.txt"
test -s "$campaign_dir/all_lfns.txt" || { echo "DAS returned no files" >&2; exit 7; }

if (( limit_files > 0 )); then
  awk -v limit="$limit_files" 'NR <= limit' "$campaign_dir/all_lfns.txt" > "$campaign_dir/lfns.txt"
else
  cp "$campaign_dir/all_lfns.txt" "$campaign_dir/lfns.txt"
fi
test -s "$campaign_dir/lfns.txt"
awk '{printf "%d %s\n", NR-1, $0}' "$campaign_dir/lfns.txt" > "$campaign_dir/jobs.txt"
job_count="$(wc -l < "$campaign_dir/jobs.txt")"

cp "$runner" "$campaign_dir/frozen_configs/"
cp "$emulator_cfg" "$campaign_dir/frozen_configs/"
cp "$campaign_cfg" "$campaign_dir/frozen_configs/"
cp "$analyzer_dir/test/run-mhh-gcts-training-ntuple.py" "$campaign_dir/frozen_configs/"
cp "$analyzer_dir/test/check_mhh_gcts_training_ntuple.py" "$campaign_dir/frozen_configs/"
cp "$analyzer_dir/test/validate_mhh_gcts_edm.py" "$campaign_dir/frozen_configs/"
cp "$analyzer_dir/test/check_mhh_gcts_campaign.py" "$campaign_dir/frozen_configs/"

cat > "$campaign_dir/submit.sub" <<SUBMIT
universe = vanilla
initialdir = $campaign_dir
executable = $runner
arguments = \$(job_index) \$(input_lfn) $max_events $dataset $cmssw_src $global_tag $emulator_cfg

output = logs/job_\$(job_index).out
error = logs/job_\$(job_index).err
log = logs/condor.log

should_transfer_files = YES
when_to_transfer_output = ON_EXIT
use_x509userproxy = True

request_cpus = 1
request_memory = ${request_memory}MB
request_disk = ${request_disk}MB

requirements = (TARGET.has_avx2 =?= True) && regexp("hep[.]wisc[.]edu\$", TARGET.Machine)
on_exit_hold = (ExitBySignal == True) || (ExitCode != 0)

queue job_index,input_lfn from jobs.txt
SUBMIT

{
  printf 'tag=%s\n' "$tag"
  printf 'created_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  printf 'dataset=%s\n' "$dataset"
  printf 'job_count=%s\n' "$job_count"
  printf 'max_events_per_file=%s\n' "$max_events"
  printf 'limit_files=%s\n' "$limit_files"
  printf 'cmssw_base=%s\n' "$CMSSW_BASE"
  printf 'scram_arch=%s\n' "$SCRAM_ARCH"
  printf 'global_tag=%s\n' "$global_tag"
  printf 'cmssw_commit=%s\n' "$(git -C "$cmssw_src" rev-parse HEAD)"
  printf 'analyzer_commit=%s\n' "$(git -C "$analyzer_dir" rev-parse HEAD)"
  printf 'emulator_cfg=%s\n' "$emulator_cfg"
  printf 'emulator_cfg_sha256=%s\n' "$(sha256sum "$emulator_cfg" | awk '{print $1}')"
  printf 'campaign_cfg_sha256=%s\n' \
    "$(sha256sum "$campaign_cfg" | awk '{print $1}')"
  printf 'proxy_timeleft=%s\n' "$(voms-proxy-info -timeleft)"
} > "$campaign_dir/CAMPAIGN_PROVENANCE.txt"

(
  cd "$campaign_dir"
  condor_submit submit.sub | tee submission.txt
)
printf '%s\n' "$campaign_dir" > "$HOME/latest_mhhgcts_campaign_dir.txt"

echo
echo "Submitted $job_count MHH/GCTS campaign shards."
echo "Campaign directory: $campaign_dir"
echo "Each successful shard writes:"
echo "  mhh_gcts_NNNN_edm.root       (replay EDM with packed pipeline stages)"
echo "  mhh_gcts_NNNN_training.root  (decoded flat training tree)"
echo "Monitor from this same login host with: condor_q \"\$USER\""

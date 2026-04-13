#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./runComparison.bash [--skip-cmsrun]

Runs the GCT Sum CMSSW analyzer, replays the SUM_IP and TO_GT_IP firmware C++
models, and compares the CMSSW GT-link output against the firmware replay.

Options:
  --skip-cmsrun  Reuse existing gctsum_mc_sum_input.txt and
                 gctsum_mc_gt_output.txt from this analyzer directory.

Environment overrides:
  CMSSW_DIR        CMSSW release directory. Defaults to this package's release.
  INPUT_ROOT       Input ROOT file passed to test/test-gctsum-mc.py.
  MAX_EVENTS       Number of events for cmsRun. Defaults to 100.
  APXROOT          apx-fs-r2-gctsum_wip directory. Defaults to
                   $HOME/apx-fs-r2-gctsum_wip.
  XFDIR            xF13P directory. Defaults to $APXROOT/xF13P.
  HLS_INCLUDE_DIR  HLS include directory.
EOF
}

RUN_CMSRUN=1
case "${1:-}" in
  "")
    ;;
  "--skip-cmsrun")
    RUN_CMSRUN=0
    ;;
  "-h"|"--help")
    usage
    exit 0
    ;;
  *)
    echo "Unknown option: $1" >&2
    usage >&2
    exit 2
    ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_CMSSW_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"

CMSSW_DIR="${CMSSW_DIR:-$DEFAULT_CMSSW_DIR}"
INPUT_ROOT="${INPUT_ROOT:-file:///hdfs/store/user/rsimeon/MCFiles/001ebf5f-b83c-43fc-997f-c2e5ecf1f9dd.root}"
MAX_EVENTS="${MAX_EVENTS:-100}"

APXROOT="${APXROOT:-$HOME/apx-fs-r2-gctsum_wip}"
XFDIR="${XFDIR:-$APXROOT/xF13P}"
HLS_INCLUDE_DIR="${HLS_INCLUDE_DIR:-/cvmfs/cms.cern.ch/el9_amd64_gcc12/external/hls/2019.08-e6beae7d560007d8bb20c2cf88bfde9a/include}"

if ! command -v cmsenv >/dev/null 2>&1; then
  # Define cmsenv when the script is run from a plain shell.
  set +u
  source /cvmfs/cms.cern.ch/cmsset_default.sh
  set -u
fi

if [[ ! -d "$CMSSW_DIR/src" ]]; then
  echo "CMSSW_DIR does not look like a CMSSW release: $CMSSW_DIR" >&2
  exit 1
fi

if [[ ! -d "$XFDIR" ]]; then
  echo "XFDIR does not exist: $XFDIR" >&2
  echo "Set APXROOT or XFDIR before running this script." >&2
  exit 1
fi

if [[ ! -d "$HLS_INCLUDE_DIR" ]]; then
  echo "HLS_INCLUDE_DIR does not exist: $HLS_INCLUDE_DIR" >&2
  exit 1
fi

cd "$CMSSW_DIR/src"
cmsenv
scram b -j 8

if [[ "$RUN_CMSRUN" -eq 1 ]]; then
  cd "$CMSSW_DIR/src/L1Trigger/L1CaloPhase2Analyzer"
  cmsRun test/test-gctsum-mc.py \
    inputFiles="$INPUT_ROOT" \
    outputFile=gctsum_mc.root \
    sumInputDumpFile=gctsum_mc_sum_input.txt \
    gtOutputDumpFile=gctsum_mc_gt_output.txt \
    maxEvents="$MAX_EVENTS"
fi

cd "$CMSSW_DIR/src"
cmsenv

export ANALYZER_DIR="$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer"

export CMSSW_SUM_IN32="$ANALYZER_DIR/gctsum_mc_sum_input.txt"
export CMSSW_GT_OUT="$ANALYZER_DIR/gctsum_mc_gt_output.txt"

export SUM_IN24="$XFDIR/testing/out/gctsum_mc_sum_input_24links.txt"
export SUMIP_OUT="$XFDIR/testing/out/gctsum_mc_sumip_chain_cpp.txt"
export TOGT_OUT="$XFDIR/testing/out/gctsum_mc_togtip_chain_cpp.txt"

for required_file in "$CMSSW_SUM_IN32" "$CMSSW_GT_OUT"; do
  if [[ ! -f "$required_file" ]]; then
    echo "Required CMSSW analyzer output is missing: $required_file" >&2
    echo "Run without --skip-cmsrun, or provide the expected dump file." >&2
    exit 1
  fi
done

mkdir -p "$XFDIR/testing/build" "$XFDIR/testing/out"

awk '
BEGIN {
  printf "WordCnt"
  for (i = 0; i < 12; ++i) printf "    POS_%d", i
  for (i = 0; i < 12; ++i) printf "    NEG_%d", i
  printf "\n#BeginData\n"
}
$1 ~ /^[0-9a-fA-F]+$/ {
  printf "%s", $1
  for (i = 2; i <= 13; ++i)  printf "    %s", $i
  for (i = 18; i <= 29; ++i) printf "    %s", $i
  printf "\n"
}
' "$CMSSW_SUM_IN32" > "$SUM_IN24"

cd "$APXROOT"

g++ -O2 -std=c++17 \
  -I"$HLS_INCLUDE_DIR" \
  -I"$XFDIR/testing" \
  -I"$XFDIR/AlgoSRC/SUM_IP" \
  -I"$XFDIR/AlgoSRC/TO_GT_IP" \
  "$XFDIR/testing/sumip_vector_replay.cpp" \
  "$XFDIR/AlgoSRC/SUM_IP/algo_top.cpp" \
  "$XFDIR/AlgoSRC/SUM_IP/bitonicSort32.cpp" \
  -o "$XFDIR/testing/build/sumip_vector_replay"

g++ -O2 -std=c++17 \
  -I"$HLS_INCLUDE_DIR" \
  -I"$XFDIR/testing" \
  -I"$XFDIR/AlgoSRC/SUM_IP" \
  -I"$XFDIR/AlgoSRC/TO_GT_IP" \
  "$XFDIR/testing/togtip_vector_replay.cpp" \
  "$XFDIR/AlgoSRC/TO_GT_IP/algo_top.cpp" \
  -o "$XFDIR/testing/build/togtip_vector_replay"

g++ -O2 -std=c++17 \
  -I"$HLS_INCLUDE_DIR" \
  -I"$XFDIR/testing" \
  "$XFDIR/testing/compare_vector_tables.cpp" \
  -o "$XFDIR/testing/build/compare_vector_tables"

"$XFDIR/testing/build/sumip_vector_replay" \
  "$SUM_IN24" \
  "$SUMIP_OUT"

"$XFDIR/testing/build/togtip_vector_replay" \
  "$SUMIP_OUT" \
  "$TOGT_OUT"

"$XFDIR/testing/build/compare_vector_tables" \
  "$CMSSW_GT_OUT" \
  "$TOGT_OUT" \
  6

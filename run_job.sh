#!/bin/bash
period=$1
version="1"
base_dir="$PWD"

job_dir="$base_dir/data/$version/$period"
echo "[JOB] Starting job for period: $period"
echo "[JOB] Working directory: $job_dir"

cd "$job_dir" || { echo "[JOB-ERROR] Failed to enter $job_dir"; exit 1; }
[[ -x ./job ]] || { echo "[JOB-ERROR] job script not executable"; exit 1; }

./job
EXIT_CODE=$?
echo "[JOB] Finished with exit code $EXIT_CODE"
exit $EXIT_CODE

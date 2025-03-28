#!/bin/bash

# Define the list of years to process
years=("2016" "2017" "2018" "2022")

# Get absolute path to data directory
base_dir=$(ls data)
subdir=$(ls "$base_dir" | head -n 1)

# Function to execute the job in a specific directory
run_job() {
    local year=$1
    local dir="${base_dir}/${subdir}/${year}"
    
    echo "Starting job for year: $year (Directory: $dir)"
    
    # Check if the directory exists
    if [[ ! -d "$dir" ]]; then
        echo "Directory $dir does not exist. Skipping year $year..."
        return 1
    fi
    
    # Check if the job script exists and is executable
    if [[ ! -x "${dir}/job" ]]; then
        echo "Job script 'job' is missing or not executable in $dir. Skipping year $year..."
        return 1
    fi
    
    # Submit the job via bsub with absolute paths
    bsub -J "job_${year}" \
         -o "${dir}/job_%J.out" \
         -e "${dir}/job_%J.err" \
         "cd ${dir} && ./job"
}

# Run jobs for each year
echo "Submitting jobs for years: ${years[@]}..."
for year in "${years[@]}"; do
    run_job "$year"
done

echo "All jobs submitted! Use 'bjobs' to check their status."
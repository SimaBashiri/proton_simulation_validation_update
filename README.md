# Proton Simulation Validation (CMSSW_15_1_X)

CMS validation package for proton simulation studies.

## Quick Start

```bash
# 1. Setup CMSSW environment
cmsrel CMSSW_15_1_X_2025-03-24-2300
cd CMSSW_15_1_X_2025-03-24-2300/src
dir=$PWD
scram b
cmsenv

# 2. Get validation code
git clone https://github.com/SimaBashiri/proton_simulation_validation_update.git
cd proton_simulation_validation_update
mv Validations ${dir}

# 3. Run validation (replace <my_version> with your preferred name)
./submit <my_version>

The results can be found in data/<my_version> directory.

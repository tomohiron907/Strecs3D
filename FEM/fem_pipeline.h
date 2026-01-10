#ifndef FEM_PIPELINE_H
#define FEM_PIPELINE_H

#include <string>
#include "FEMProgressCallback.h"

/**
 * Run complete FEM analysis pipeline
 *
 * This function performs the following steps:
 * 1. Load simulation configuration from JSON file
 * 2. Convert STEP file to INP format
 * 3. Run CalculiX analysis
 * 4. Convert results from FRD to VTU format
 *
 * @param config_file Path to the simulation configuration JSON file
 * @param progressCallback Optional callback for progress reporting (nullptr = no reporting)
 * @return Path to the generated VTU file on success, empty string on failure
 */
std::string runFEMAnalysis(const std::string& config_file, FEMProgressCallback* progressCallback = nullptr);

#endif // FEM_PIPELINE_H

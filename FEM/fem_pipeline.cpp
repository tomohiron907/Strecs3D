#include "fem_pipeline.h"
#include "frd2vtu.h"
#include "step2inp.h"
#include "simulation_config.h"
#include "../utils/tempPathUtility.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <limits.h>

// --- 【修正1】 プラットフォームごとのヘッダー切り替え ---
#if defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(_WIN32)
    #include <windows.h>
    // WindowsでのPATH_MAX対応（定義されていない場合の予備）
    #ifndef PATH_MAX
    #define PATH_MAX MAX_PATH
    #endif
#endif
// ----------------------------------------------------

// Helper to run command and capture output
int runCommand(const std::string& cmd, FEMProgressCallback* callback) {
    std::string full_cmd = cmd + " 2>&1"; // Capture stderr too
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
        return -1;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        // Remove trailing newline
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        if (callback) {
            callback->log(line);
        } else {
            std::cout << line << std::endl;
        }
    }
    
    return pclose(pipe);
}

std::string runFEMAnalysis(const std::string& config_file, FEMProgressCallback* progressCallback) {
    // Helper lambda to report progress safely
    auto reportProgress = [&](int progress, const std::string& msg) {
        if (progressCallback) {
            progressCallback->reportProgress(progress, msg);
            progressCallback->log(">>> " + msg); // Also log progress messages
        }
    };

    // Helper lambda to log
    auto log = [&](const std::string& msg) {
        if (progressCallback) {
            progressCallback->log(msg);
        } else {
            std::cout << msg << std::endl;
        }
    };

    // Helper lambda to check cancellation
    auto checkCancellation = [&]() -> bool {
        return progressCallback && progressCallback->isCancelled();
    };

    reportProgress(0, "Loading configuration...");

    // Load simulation configuration from JSON
    SimulationConfig config;
    try {
        config = SimulationConfig::fromJsonFile(config_file);
        log("Loaded configuration from: " + config_file);
    } catch (const std::exception& e) {
        std::string err = "Error: Failed to load config: " + std::string(e.what());
        std::cerr << err << std::endl;
        log(err);
        return "";
    }

    std::string step_file = config.step_file;

    // Create constraint conditions from config
    std::vector<ConstraintProperties> constraints;
    for (const auto& fixed_face : config.constraints.fixed_faces) {
        constraints.push_back(createConstraintCondition(fixed_face.surface_id));
    }

    // Create load conditions from config
    std::vector<LoadProperties> loads;
    for (const auto& load : config.loads.applied_loads) {
        std::vector<double> direction = {load.direction.x, load.direction.y, load.direction.z};
        loads.push_back(createLoadCondition(load.surface_id, load.magnitude, direction));
    }

    if (constraints.empty() && loads.empty()) {
        std::string err = "Error: No boundary conditions specified.";
        std::cerr << err << std::endl;
        log(err);
        return "";
    }

    // Get temp/FEM directory for intermediate files
    std::filesystem::path fem_temp_dir = TempPathUtility::getTempSubDirPath("FEM");

    // Create directory if it doesn't exist
    if (!std::filesystem::exists(fem_temp_dir)) {
        std::filesystem::create_directories(fem_temp_dir);
    }

    // Get base filename for subsequent operations
    std::filesystem::path path(step_file);
    std::string base_name = path.stem().string();

    // All intermediate files will be created in temp/FEM directory
    std::filesystem::path inp_file_path = fem_temp_dir / (base_name + ".inp");
    std::filesystem::path frd_file_path = fem_temp_dir / (base_name + ".frd");
    std::filesystem::path vtu_file_path = fem_temp_dir / (base_name + ".vtu");

    std::string inp_file = inp_file_path.string();
    std::string frd_file = frd_file_path.string();
    std::string vtu_file = vtu_file_path.string();

    if (checkCancellation()) return "";

    // Step 1: Convert STEP to INP (5% -> 45%)
    reportProgress(5, "Loading STEP file...");
    log("Step 1: Converting STEP to INP...");

    reportProgress(10, "Generating mesh...");
    reportProgress(20, "Processing geometry...");
    reportProgress(25, "Creating mesh elements...");

    int result = convertStepToInp(step_file, constraints, loads, inp_file);

    if (result != 0) {
        std::string err = "Error: STEP to INP conversion failed";
        std::cerr << err << std::endl;
        log(err);
        return "";
    }

    reportProgress(35, "Writing boundary conditions...");
    reportProgress(40, "Finalizing INP file...");
    reportProgress(45, "STEP to INP conversion completed");

    if (checkCancellation()) return "";

    // Step 2: Run CalculiX analysis (45% -> 90%)
    reportProgress(47, "Preparing CalculiX environment...");
    // CalculiX needs to run in the temp/FEM directory to output files there
    log("Step 2: Running CalculiX analysis...");
    std::string original_dir = std::filesystem::current_path().string();
    std::filesystem::current_path(fem_temp_dir);

    reportProgress(50, "Starting CalculiX solver...");

    // Get path to bundled ccx executable (bin/ccx in same directory as executable)
    std::filesystem::path ccx_path;
    bool pathFound = false;

    // --- 【修正2 & 3】 OSごとの実行パス取得処理 ---
#if defined(__APPLE__)
    // macOS Implementation
    char exe_path[PATH_MAX];
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
        ccx_path = exe_dir / "bin" / "ccx"; // Mac: 拡張子なし
        pathFound = true;
    }
#elif defined(_WIN32)
    // Windows Implementation
    char exe_path[PATH_MAX];
    // GetModuleFileNameAはANSI版のパスを取得します
    if (GetModuleFileNameA(NULL, exe_path, PATH_MAX) != 0) {
        std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
        ccx_path = exe_dir / "bin" / "ccx.exe"; // Windows: .exeをつける
        pathFound = true;
    }
#else
    // Linux / Other (Fallback)
    // Linuxの場合は /proc/self/exe を読むのが一般的ですが、ここでは簡易的にデフォルトへ
    pathFound = false;
#endif

    if (pathFound) {
        log("Looking for CCX at: " + ccx_path.string());
    } else {
        log("Warning: Could not determine executable path. Using default path.");
        #if defined(_WIN32)
            ccx_path = "ccx.exe";
        #else
            ccx_path = "ccx";
        #endif
    }
    // ----------------------------------------------------

    // Windowsではパスに空白が含まれる可能性があるので引用符で囲む
    std::string ccx_command;
#if defined(_WIN32)
    ccx_command = "\"" + ccx_path.string() + "\" " + base_name;
#else
    ccx_command = ccx_path.string() + " " + base_name;
#endif
    
    log("Executing command: " + ccx_command);

    reportProgress(55, "CalculiX: Running FEM analysis...");
    
    // Execute command and capture output
    result = runCommand(ccx_command, progressCallback);
    
    reportProgress(85, "CalculiX: Processing results...");

    // Return to original directory
    std::filesystem::current_path(original_dir);

    if (result != 0) {
        std::string err = "Error: CalculiX analysis failed with code " + std::to_string(result);
        std::cerr << err << std::endl;
        log(err);
        return "";
    }

    reportProgress(88, "Verifying analysis results...");
    reportProgress(90, "CalculiX analysis completed");

    if (checkCancellation()) return "";

    // Check if FRD file was generated
    if (!std::filesystem::exists(frd_file)) {
        std::string err = "Error: FRD file was not generated: " + frd_file;
        std::cerr << err << std::endl;
        log(err);
        return "";
    }

    // Step 3: Convert FRD to VTU (90% -> 99%)
    reportProgress(91, "Reading FRD result file...");
    log("Step 3: Converting FRD to VTU...");

    reportProgress(93, "Converting to VTU format...");
    result = convertFrdToVtu(frd_file, vtu_file);

    if (result == EXIT_SUCCESS) {
        reportProgress(97, "Writing VTU file...");
        reportProgress(99, "FRD to VTU conversion completed");
        log("Analysis pipeline completed successfully!");
        log("Generated files:");
        log("  - INP file: " + inp_file);
        log("  - FRD file: " + frd_file);
        log("  - VTU file: " + vtu_file);

        // Verify VTU file exists and return its path
        if (std::filesystem::exists(vtu_file)) {
            reportProgress(100, "Analysis completed successfully");
            return vtu_file;
        } else {
            std::string err = "Error: VTU file was not generated: " + vtu_file;
            std::cerr << err << std::endl;
            log(err);
            return "";
        }
    } else {
        std::string err = "Error: FRD to VTU conversion failed";
        std::cerr << err << std::endl;
        log(err);
        return "";
    }
}
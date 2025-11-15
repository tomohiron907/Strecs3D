#include "fem_pipeline.h"
#include "frd2vtu.h"
#include "step2inp.h"
#include "simulation_config.h"
#include "../utils/tempPathUtility.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>

int runFEMAnalysis(const std::string& config_file) {
    // Load simulation configuration from JSON
    SimulationConfig config;
    try {
        config = SimulationConfig::fromJsonFile(config_file);
        std::cout << "Loaded configuration from: " << config_file << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "エラー: 設定ファイルの読み込みに失敗しました: " << e.what() << std::endl;
        return EXIT_FAILURE;
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
        std::cerr << "エラー: 設定ファイルに境界条件が指定されていません。" << std::endl;
        return EXIT_FAILURE;
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

    // Step 1: Convert STEP to INP
    std::cout << "Step 1: Converting STEP to INP..." << std::endl;
    int result = convertStepToInp(step_file, constraints, loads, inp_file);
    if (result != 0) {
        std::cerr << "エラー: STEP to INP conversion failed" << std::endl;
        return result;
    }

    // Step 2: Run CalculiX analysis
    // CalculiX needs to run in the temp/FEM directory to output files there
    std::cout << "Step 2: Running CalculiX analysis..." << std::endl;
    std::string original_dir = std::filesystem::current_path().string();
    std::filesystem::current_path(fem_temp_dir);

    std::string ccx_command = "ccx_2.22 " + base_name;
    result = std::system(ccx_command.c_str());

    // Return to original directory
    std::filesystem::current_path(original_dir);

    if (result != 0) {
        std::cerr << "エラー: CalculiX analysis failed" << std::endl;
        return result;
    }

    // Check if FRD file was generated
    if (!std::filesystem::exists(frd_file)) {
        std::cerr << "エラー: FRD file was not generated: " << frd_file << std::endl;
        return EXIT_FAILURE;
    }

    // Step 3: Convert FRD to VTU
    std::cout << "Step 3: Converting FRD to VTU..." << std::endl;
    result = convertFrdToVtu(frd_file, vtu_file);
    if (result == EXIT_SUCCESS) {
        std::cout << "Analysis pipeline completed successfully!" << std::endl;
        std::cout << "Generated files:" << std::endl;
        std::cout << "  - INP file: " << inp_file << std::endl;
        std::cout << "  - FRD file: " << frd_file << std::endl;
        std::cout << "  - VTU file: " << vtu_file << std::endl;
    } else {
        std::cerr << "エラー: FRD to VTU conversion failed" << std::endl;
    }

    return result;
}

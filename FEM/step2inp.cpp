#include "step2inp.h"
#include <iostream>
#include <gmsh.h>

Step2Inp::Step2Inp() {}

Step2Inp::~Step2Inp() {}

int Step2Inp::convert(const std::string& step_file,
                      const std::vector<ConstraintCondition>& constraints,
                      const std::vector<LoadCondition>& loads) {
    gmsh::initialize();

    try {
        // Generate mesh
        if (mesh_generator_.generateMesh(step_file) != 0) {
            gmsh::finalize();
            return 1;
        }

        // Validate surfaces
        for (const auto& constraint : constraints) {
            if (!mesh_generator_.hasSurface(constraint.surface_number)) {
                std::cerr << "エラー: Surface " << constraint.surface_number << " が見つかりません。" << std::endl;
                gmsh::finalize();
                return 1;
            }
        }

        for (const auto& load : loads) {
            if (!mesh_generator_.hasSurface(load.surface_number)) {
                std::cerr << "エラー: Surface " << load.surface_number << " が見つかりません。" << std::endl;
                gmsh::finalize();
                return 1;
            }
        }

        // Write initial INP file
        std::string base_name = InpWriter::getBaseFilename(step_file);
        std::string inp_file = base_name + ".inp";

        if (inp_writer_.initializeInpFile(step_file, inp_file) != 0) {
            gmsh::finalize();
            return 1;
        }

        // Open file for appending
        std::ofstream f(inp_file, std::ios::app);
        if (!f.is_open()) {
            std::cerr << "エラー: ファイルを開けませんでした: " << inp_file << std::endl;
            gmsh::finalize();
            return 1;
        }

        // Write material and element sets
        material_setter_.writeEall(f);
        material_setter_.writeMaterialElementSet(f);

        // Write constraint conditions
        for (const auto& constraint : constraints) {
            constraint_setter_.writeConstraintNodeSet(f, constraint.surface_number);
        }

        // Write material properties
        material_setter_.writePhysicalConstants(f);
        material_setter_.writeMaterial(f);
        material_setter_.writeSections(f);

        // Write analysis step
        inp_writer_.writeStep(f);
        constraint_setter_.writeFixedConstraints(f);

        // Write load conditions
        for (const auto& load : loads) {
            std::vector<std::size_t> node_tags;
            std::vector<double> coord, parametricCoord;
            gmsh::model::mesh::getNodes(node_tags, coord, parametricCoord, 2, load.surface_number, true);

            std::cout << "Surface " << load.surface_number << " のノード数: " << node_tags.size() << std::endl;

            // Use area-based force calculation with values from load condition
            load_setter_.writeForceBoundaryCondition(f, load.surface_number, load.magnitude, load.direction);
            std::cout << "Surface " << load.surface_number << " に寄与面積に基づく力の境界条件を追加しました" << std::endl;
        }

        // Write outputs and end step
        inp_writer_.writeOutputs(f);
        inp_writer_.writeEndStep(f);

        f.close();

        std::cout << "変換完了（境界条件追加済み): " << step_file << " -> " << inp_file << std::endl;
        std::cout << "適用された境界条件:" << std::endl;
        for (const auto& constraint : constraints) {
            std::cout << "  Surface " << constraint.surface_number << ": fixed" << std::endl;
        }
        for (const auto& load : loads) {
            std::cout << "  Surface " << load.surface_number << ": force (magnitude: " << load.magnitude << ")" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "エラーが発生しました: " << e.what() << std::endl;
        gmsh::finalize();
        return 1;
    }

    gmsh::finalize();
    return 0;
}

int convertStepToInp(const std::string& step_file,
                     const std::vector<ConstraintCondition>& constraints,
                     const std::vector<LoadCondition>& loads) {
    Step2Inp converter;
    return converter.convert(step_file, constraints, loads);
}

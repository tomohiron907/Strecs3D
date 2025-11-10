#include "SimulationConditionExporter.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool SimulationConditionExporter::exportToJson(
    const UIState* uiState,
    const QString& outputPath,
    double minElementSize,
    double maxElementSize
) {
    if (!uiState) {
        std::cerr << "Error: UIState is null" << std::endl;
        return false;
    }

    // UIStateから情報を取得
    QString stepFilePath = uiState->getStepFilePath();
    BoundaryCondition boundaryCondition = uiState->getBoundaryCondition();

    // JSONオブジェクトを構築
    json j;

    // step_file
    j["step_file"] = stepFilePath.toStdString();

    // mesh
    j["mesh"] = {
        {"min_element_size", minElementSize},
        {"max_element_size", maxElementSize}
    };

    // constraints - fixed_faces
    json fixedFacesArray = json::array();
    for (const auto& constrain : boundaryCondition.constrains) {
        json constrainObj = {
            {"surface_id", constrain.surface_id},
            {"name", constrain.name}
        };
        fixedFacesArray.push_back(constrainObj);
    }
    j["constraints"]["fixed_faces"] = fixedFacesArray;

    // loads - applied_loads
    json appliedLoadsArray = json::array();
    for (const auto& load : boundaryCondition.loads) {
        json loadObj = {
            {"surface_id", load.surface_id},
            {"name", load.name},
            {"magnitude", load.magnitude},
            {"direction", {
                {"x", load.direction.x},
                {"y", load.direction.y},
                {"z", load.direction.z}
            }}
        };
        appliedLoadsArray.push_back(loadObj);
    }
    j["loads"]["applied_loads"] = appliedLoadsArray;

    // ファイルに書き込み（インデント付き）
    try {
        std::ofstream outFile(outputPath.toStdString());
        if (!outFile.is_open()) {
            std::cerr << "Error: Failed to open file: " << outputPath.toStdString() << std::endl;
            return false;
        }

        // インデント2スペースで出力
        outFile << j.dump(2) << std::endl;
        outFile.close();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error writing JSON file: " << e.what() << std::endl;
        return false;
    }
}

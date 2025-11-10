#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct Vector3D {
    double x;
    double y; 
    double z;
};

struct MeshConfig {
    int min_element_size;
    int max_element_size;
};

struct FixedFace {
    int surface_id;
    std::string name;
};

struct AppliedLoad {
    int surface_id;
    std::string name;
    double magnitude;
    Vector3D direction;
};

struct ConstraintsConfig {
    std::vector<FixedFace> fixed_faces;
};

struct LoadsConfig {
    std::vector<AppliedLoad> applied_loads;
};

struct SimulationConfig {
    std::string step_file;
    MeshConfig mesh;
    ConstraintsConfig constraints;
    LoadsConfig loads;
    
    static SimulationConfig fromJsonFile(const std::string& filename);
    static SimulationConfig fromJson(const nlohmann::json& json);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3D, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeshConfig, min_element_size, max_element_size)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FixedFace, surface_id, name)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AppliedLoad, surface_id, name, magnitude, direction)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConstraintsConfig, fixed_faces)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LoadsConfig, applied_loads)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimulationConfig, step_file, mesh, constraints, loads)
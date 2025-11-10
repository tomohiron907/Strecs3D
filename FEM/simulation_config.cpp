#include "simulation_config.h"
#include <fstream>
#include <stdexcept>

SimulationConfig SimulationConfig::fromJsonFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filename);
    }
    
    nlohmann::json json;
    file >> json;
    
    return fromJson(json);
}

SimulationConfig SimulationConfig::fromJson(const nlohmann::json& json) {
    SimulationConfig config;
    
    json.at("step_file").get_to(config.step_file);
    json.at("mesh").get_to(config.mesh);
    json.at("constraints").get_to(config.constraints);
    json.at("loads").get_to(config.loads);
    
    return config;
}
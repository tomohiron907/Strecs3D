#include "MaterialManager.h"
#include <iostream>

MaterialManager& MaterialManager::instance() {
    static MaterialManager instance;
    return instance;
}

MaterialManager::MaterialManager() {
    initializeMaterials();
}

// Helper to register isotropic materials
void MaterialManager::registerIsotropicMaterial(const std::string& name, double E, double nu) {
    MaterialData mat;
    mat.name = name;
    mat.type = MaterialType::Isotropic;
    mat.youngs_modulus = E;
    mat.poisson_ratio = nu;
    
    // Fill orthotropic with same values just in case
    mat.E1 = mat.E2 = mat.E3 = E;
    mat.nu12 = mat.nu13 = mat.nu23 = nu;
    double G = E / (2.0 * (1.0 + nu));
    mat.G12 = mat.G13 = mat.G23 = G;

    materials_[name] = mat;
}

// Helper to register orthotropic materials
void MaterialManager::registerOrthotropicMaterial(const std::string& name, 
                                                  double E1, double E2, double E3,
                                                  double nu12, double nu13, double nu23,
                                                  double G12, double G13, double G23,
                                                  double approxE, double approxNu) {
    MaterialData mat;
    mat.name = name;
    mat.type = MaterialType::Orthotropic;
    
    mat.E1 = E1; mat.E2 = E2; mat.E3 = E3;
    mat.nu12 = nu12; mat.nu13 = nu13; mat.nu23 = nu23;
    mat.G12 = G12; mat.G13 = G13; mat.G23 = G23;
    
    mat.youngs_modulus = approxE;
    mat.poisson_ratio = approxNu;

    materials_[name] = mat;
}

void MaterialManager::initializeMaterials() {
    // PLA: Li, M., Xu, Y., & Fang, J. (2024). Thin-Walled Structures, 199, 111800.
    registerOrthotropicMaterial("PLA",
        2669.0, 2583.0, 2208.0, // E1, E2, E3
        0.43, 0.37, 0.37,       // nu12, nu13, nu23
        919.0, 844.0, 844.0,    // G12, G13, G23
        3640.0, 0.36            // Approx E, nu
    );

    // ABS: User provided values
    // G31 provided as 523 -> mapped to G13
    registerOrthotropicMaterial("ABS",
        1655.0, 1601.0, 1369.0, // E1, E2, E3
        0.43, 0.37, 0.37,       // nu12, nu13, nu23
        570.0, 523.0, 523.0,    // G12, G13, G23
        2200.0, 0.35            // Approx E, nu
    );
}

MaterialData MaterialManager::getMaterial(const std::string& name) const {
    auto it = materials_.find(name);
    if (it != materials_.end()) {
        return it->second;
    }
    
    // Return a default material if not found
    std::cerr << "Warning: Material '" << name << "' not found. Returning default." << std::endl;
    MaterialData defaultMat;
    defaultMat.name = "Default";
    defaultMat.type = MaterialType::Isotropic;
    defaultMat.youngs_modulus = 1000.0;
    defaultMat.poisson_ratio = 0.3;
    return defaultMat;
}

std::vector<std::string> MaterialManager::getAllMaterials() const {
    std::vector<std::string> names;
    for (const auto& pair : materials_) {
        names.push_back(pair.first);
    }
    return names;
}

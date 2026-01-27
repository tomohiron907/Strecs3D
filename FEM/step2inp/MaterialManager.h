#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

#include <string>
#include <map>
#include <vector>

enum class MaterialType {
    Isotropic,
    Orthotropic
};

struct MaterialData {
    std::string name;
    MaterialType type;

    // Isotropic properties
    double youngs_modulus; // E
    double poisson_ratio;  // nu

    // Orthotropic properties
    double E1, E2, E3;
    double nu12, nu13, nu23;
    double G12, G13, G23;

    // Helper for creating default MaterialData safely
    MaterialData() : type(MaterialType::Isotropic), 
                     youngs_modulus(0.0), poisson_ratio(0.0),
                     E1(0.0), E2(0.0), E3(0.0),
                     nu12(0.0), nu13(0.0), nu23(0.0),
                     G12(0.0), G13(0.0), G23(0.0) {}
};

class MaterialManager {
public:
    static MaterialManager& instance();

    MaterialData getMaterial(const std::string& name) const;
    std::vector<std::string> getAllMaterials() const;

private:
    MaterialManager();
    ~MaterialManager() = default;
    
    // Prevent copying
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    void initializeMaterials();

    void registerIsotropicMaterial(const std::string& name, double E, double nu);
    void registerOrthotropicMaterial(const std::string& name, 
                                     double E1, double E2, double E3,
                                     double nu12, double nu13, double nu23,
                                     double G12, double G13, double G23,
                                     double approxE, double approxNu);

    std::map<std::string, MaterialData> materials_;
};

#endif // MATERIAL_MANAGER_H

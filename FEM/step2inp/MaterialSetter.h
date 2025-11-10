#ifndef MATERIAL_SETTER_H
#define MATERIAL_SETTER_H

#include <string>
#include <fstream>

struct MaterialProperties {
    std::string name;
    double youngs_modulus;
    double poisson_ratio;
};

class MaterialSetter {
public:
    MaterialSetter();
    ~MaterialSetter();

    // Set material properties
    void setMaterial(const std::string& name, double youngs_modulus, double poisson_ratio);
    void setMaterial(const MaterialProperties& material);

    // Write material-related sections to file
    void writeEall(std::ofstream& f) const;
    void writeMaterialElementSet(std::ofstream& f) const;
    void writePhysicalConstants(std::ofstream& f) const;
    void writeMaterial(std::ofstream& f) const;
    void writeSections(std::ofstream& f) const;

    // Get material properties
    const MaterialProperties& getMaterial() const;

private:
    MaterialProperties material_;
};

#endif // MATERIAL_SETTER_H

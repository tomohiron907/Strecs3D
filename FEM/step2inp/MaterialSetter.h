#ifndef MATERIAL_SETTER_H
#define MATERIAL_SETTER_H

#include <string>
#include <fstream>
#include "MaterialManager.h" // Include MaterialManager definitions

class MaterialSetter {
public:
    MaterialSetter();
    ~MaterialSetter();

    // Set material properties
    void setMaterial(const std::string& name); 
    void setMaterial(const MaterialData& material);

    // Write material-related sections to file
    void writeEall(std::ofstream& f) const;
    void writeMaterialElementSet(std::ofstream& f) const;
    void writePhysicalConstants(std::ofstream& f) const;
    void writeMaterial(std::ofstream& f) const;
    void writeSections(std::ofstream& f) const;

    // Get material properties
    const MaterialData& getMaterial() const;

private:
    MaterialData material_;
};

#endif // MATERIAL_SETTER_H

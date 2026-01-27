#include "MaterialSetter.h"
#include <iostream>

MaterialSetter::MaterialSetter() {
    // Default to PLA
    setMaterial("PLA");
}

MaterialSetter::~MaterialSetter() {
}

void MaterialSetter::setMaterial(const std::string& name) {
    material_ = MaterialManager::instance().getMaterial(name);
}

void MaterialSetter::setMaterial(const MaterialData& material) {
    material_ = material;
}

void MaterialSetter::writeEall(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Define element set Eall\n";
    f << "*ELSET, ELSET=Eall\n";
    f << "volume1\n";
}

void MaterialSetter::writeMaterialElementSet(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Element sets for materials and FEM element type (solid, shell, beam, fluid)\n";
    f << "*ELSET, ELSET=MaterialSolidSolid\n";
    f << "volume1\n";
}

void MaterialSetter::writePhysicalConstants(std::ofstream& f) const {
    f << "** Physical constants for SI(mm) unit system with Kelvins\n";
    f << "*PHYSICAL CONSTANTS, ABSOLUTE ZERO=0, STEFAN BOLTZMANN=5.670374419e-11\n";
}

void MaterialSetter::writeMaterial(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Materials\n";
    f << "** see information about units at file end\n";
    f << "** FreeCAD material name: " << material_.name << "\n";
    f << "** " << material_.name << "\n";
    f << "*MATERIAL, NAME=" << material_.name << "\n";
    
    if (material_.type == MaterialType::Orthotropic) {
        f << "*ELASTIC, TYPE=ENGINEERING CONSTANTS\n";
        f << "** E1,   E2,   E3,   nu12, nu13, nu23, G12, G13\n";
        f << material_.E1 << ", " << material_.E2 << ", " << material_.E3 << ", "
          << material_.nu12 << ", " << material_.nu13 << ", " << material_.nu23 << ", " 
          << material_.G12 << ", " << material_.G13 << "\n";
        f << "** G23\n";
        f << material_.G23 << "\n";
    } else {
        // Fallback for Isotropic or others
        f << "*ELASTIC\n";
        f << material_.youngs_modulus << ", " << material_.poisson_ratio << "\n";
    }
}

void MaterialSetter::writeSections(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Sections\n";
    f << "*SOLID SECTION, ELSET=MaterialSolidSolid, MATERIAL=" << material_.name << "\n";
}

const MaterialData& MaterialSetter::getMaterial() const {
    return material_;
}

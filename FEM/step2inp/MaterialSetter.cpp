#include "MaterialSetter.h"

MaterialSetter::MaterialSetter()
    : material_{"MaterialSolid", 3640.0, 0.36}  // Default: PLA-Generic
{
}

MaterialSetter::~MaterialSetter() {
}

void MaterialSetter::setMaterial(const std::string& name, double youngs_modulus, double poisson_ratio) {
    material_.name = name;
    material_.youngs_modulus = youngs_modulus;
    material_.poisson_ratio = poisson_ratio;
}

void MaterialSetter::setMaterial(const MaterialProperties& material) {
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
    // Material property values for PLA (FDM) are based on:
    // Li, M., Xu, Y., & Fang, J. (2024). Orthotropic mechanical properties of PLA
    // materials fabricated by fused deposition modeling. Thin-Walled Structures,
    // 199, 111800. https://doi.org/10.1016/j.tws.2024.111800
    f << "***********************************************************\n";
    f << "** Materials\n";
    f << "** see information about units at file end\n";
    f << "** FreeCAD material name: " << material_.name << "\n";
    f << "** " << material_.name << "\n";
    f << "*MATERIAL, NAME=" << material_.name << "\n";
    f << "*ELASTIC, TYPE=ENGINEERING CONSTANTS\n";
    f << "** E1,   E2,   E3,   nu12, nu13, nu23, G12, G13\n";
    f << "2669., 2583., 2208., 0.43, 0.37, 0.37, 919., 844.\n";
    f << "** G23\n";
    f << "844.\n";
}

void MaterialSetter::writeSections(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Sections\n";
    f << "*SOLID SECTION, ELSET=MaterialSolidSolid, MATERIAL=" << material_.name << "\n";
}

const MaterialProperties& MaterialSetter::getMaterial() const {
    return material_;
}

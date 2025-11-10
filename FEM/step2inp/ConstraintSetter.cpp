#include "ConstraintSetter.h"
#include <gmsh.h>
#include <iostream>

ConstraintSetter::ConstraintSetter() {
}

ConstraintSetter::~ConstraintSetter() {
}

void ConstraintSetter::addConstraint(const ConstraintCondition& constraint) {
    constraints_.push_back(constraint);
}

void ConstraintSetter::addConstraint(int surface_number) {
    constraints_.push_back({surface_number});
}

std::vector<int> ConstraintSetter::getConstraintNodeTags(int surface_number) const {
    std::vector<std::size_t> node_tags;
    std::vector<double> coord, parametricCoord;
    gmsh::model::mesh::getNodes(node_tags, coord, parametricCoord, 2, surface_number, true);

    std::vector<int> int_node_tags(node_tags.begin(), node_tags.end());
    return int_node_tags;
}

void ConstraintSetter::writeConstraintNodeSet(std::ofstream& f, int surface_number) const {
    std::vector<int> node_tags = getConstraintNodeTags(surface_number);

    f << "***********************************************************\n";
    f << "** constraints fixed node sets\n";
    f << "** ConstraintFixed\n";
    f << "*NSET,NSET=ConstraintFixed\n";
    for (int tag : node_tags) {
        f << tag << ",\n";
    }

    std::cout << "Surface " << surface_number << " のノード数: " << node_tags.size() << std::endl;
}

void ConstraintSetter::writeFixedConstraints(std::ofstream& f) const {
    f << "***********************************************************\n";
    f << "** Fixed Constraints\n";
    f << "** ConstraintFixed\n";
    f << "*BOUNDARY\n";
    f << "ConstraintFixed,1\n";
    f << "ConstraintFixed,2\n";
    f << "ConstraintFixed,3\n";
}

const std::vector<ConstraintCondition>& ConstraintSetter::getConstraints() const {
    return constraints_;
}

ConstraintCondition createConstraintCondition(int surface_number) {
    return {surface_number};
}

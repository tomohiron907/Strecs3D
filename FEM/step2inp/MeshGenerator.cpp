#include "MeshGenerator.h"
#include <gmsh.h>
#include <iostream>
#include <algorithm>

MeshGenerator::MeshGenerator()
    : char_length_min_(1.0)
    , char_length_max_(5.0)
    , mesh_algorithm_(1)  
    , mesh_order_(2)
{
}

MeshGenerator::~MeshGenerator() {
}

void MeshGenerator::setCharacteristicLength(double min_length, double max_length) {
    char_length_min_ = min_length;
    char_length_max_ = max_length;
}

void MeshGenerator::setMeshAlgorithm(int algorithm) {
    mesh_algorithm_ = algorithm;
}

void MeshGenerator::setMeshOrder(int order) {
    mesh_order_ = order;
}

int MeshGenerator::generateMesh(const std::string& step_file) {
    try {
        std::cout << "STEPファイルを読み込み中: " << step_file << std::endl;
        gmsh::open(step_file);

        gmsh::model::geo::synchronize();

        // Check if volumes exist
        std::vector<std::pair<int, int>> vols;
        gmsh::model::getEntities(vols, 3);

        if (vols.empty()) {
            std::cerr << "エラー: STEPファイル内にボリュームが見つかりませんでした。" << std::endl;
            return 1;
        }

        // Add physical group for volumes
        std::vector<int> vol_tags;
        for (const auto& vol : vols) {
            vol_tags.push_back(vol.second);
        }
        gmsh::model::addPhysicalGroup(3, vol_tags, -1, "SolidVolume");

        // Set mesh parameters
        gmsh::option::setNumber("Mesh.CharacteristicLengthMin", char_length_min_);
        gmsh::option::setNumber("Mesh.CharacteristicLengthMax", char_length_max_);
        gmsh::option::setNumber("Mesh.HighOrderOptimize", 2);

        // Generate 3D mesh
        std::cout << "3Dメッシュを生成中..." << std::endl;
        gmsh::option::setNumber("Mesh.Algorithm3D", mesh_algorithm_);
        gmsh::model::mesh::generate(3);
        gmsh::model::mesh::setOrder(mesh_order_);
        gmsh::model::mesh::optimize("HighOrderElastic");

        gmsh::option::setNumber("Mesh.SaveAll", 0);

        // Get surface tags
        std::vector<std::pair<int, int>> surfaces;
        gmsh::model::getEntities(surfaces, 2);

        surface_tags_.clear();
        for (const auto& surface : surfaces) {
            surface_tags_.push_back(surface.second);
        }

        std::cout << "利用可能な面 (Surface):" << std::endl;
        for (int tag : surface_tags_) {
            std::cout << "  Surface " << tag << std::endl;
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "メッシュ生成エラー: " << e.what() << std::endl;
        return 1;
    }
}

std::vector<int> MeshGenerator::getSurfaceTags() const {
    return surface_tags_;
}

bool MeshGenerator::hasSurface(int surface_number) const {
    return std::find(surface_tags_.begin(), surface_tags_.end(), surface_number) != surface_tags_.end();
}

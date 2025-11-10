#ifndef MESH_GENERATOR_H
#define MESH_GENERATOR_H

#include <string>
#include <vector>

class MeshGenerator {
public:
    MeshGenerator();
    ~MeshGenerator();

    // Generate mesh from STEP file
    int generateMesh(const std::string& step_file);

    // Get available surface tags
    std::vector<int> getSurfaceTags() const;

    // Check if a surface exists
    bool hasSurface(int surface_number) const;

    // Set mesh parameters
    void setCharacteristicLength(double min_length, double max_length);
    void setMeshAlgorithm(int algorithm);
    void setMeshOrder(int order);

private:
    std::vector<int> surface_tags_;
    double char_length_min_;
    double char_length_max_;
    int mesh_algorithm_;
    int mesh_order_;
};

#endif // MESH_GENERATOR_H

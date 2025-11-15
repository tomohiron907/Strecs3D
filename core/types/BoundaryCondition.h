#pragma once

#include <string>
#include <vector>

// 3次元ベクトル構造体
struct Vector3D {
    double x;
    double y;
    double z;
};

// 拘束条件構造体
struct ConstraintCondition {
    int surface_id;
    std::string name;
};

// 荷重条件構造体
struct LoadCondition {
    int surface_id;
    std::string name;
    double magnitude;
    Vector3D direction;
};

// 境界条件構造体
struct BoundaryCondition {
    std::vector<ConstraintCondition> constraints;
    std::vector<LoadCondition> loads;
};

#include "LoadConditionSetter.h"
#include <gmsh.h>
#include <iostream>
#include <cmath>
#include <map>
#include <iomanip>

LoadConditionSetter::LoadConditionSetter() {
}

LoadConditionSetter::~LoadConditionSetter() {
}

void LoadConditionSetter::addLoad(const LoadCondition& load) {
    loads_.push_back(load);
}

void LoadConditionSetter::addLoad(int surface_number, double magnitude, const std::vector<double>& direction) {
    loads_.push_back({surface_number, magnitude, direction});
}

double LoadConditionSetter::calculateElementArea(const std::vector<std::vector<double>>& coords) {
    int n_nodes = coords.size();

    if (n_nodes == 3) {
        // 三角形要素: 2つの辺ベクトルの外積の大きさの半分
        std::vector<double> v1 = {coords[1][0] - coords[0][0], coords[1][1] - coords[0][1], coords[1][2] - coords[0][2]};
        std::vector<double> v2 = {coords[2][0] - coords[0][0], coords[2][1] - coords[0][1], coords[2][2] - coords[0][2]};

        // 外積計算
        std::vector<double> cross = {
            v1[1] * v2[2] - v1[2] * v2[1],
            v1[2] * v2[0] - v1[0] * v2[2],
            v1[0] * v2[1] - v1[1] * v2[0]
        };

        // ベクトルの大きさ
        double magnitude = std::sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]);
        return 0.5 * magnitude;

    } else if (n_nodes == 4) {
        // 四角形要素: 2つの三角形に分割して面積を合計
        std::vector<double> v1 = {coords[1][0] - coords[0][0], coords[1][1] - coords[0][1], coords[1][2] - coords[0][2]};
        std::vector<double> v2 = {coords[2][0] - coords[0][0], coords[2][1] - coords[0][1], coords[2][2] - coords[0][2]};

        std::vector<double> cross1 = {
            v1[1] * v2[2] - v1[2] * v2[1],
            v1[2] * v2[0] - v1[0] * v2[2],
            v1[0] * v2[1] - v1[1] * v2[0]
        };
        double area1 = 0.5 * std::sqrt(cross1[0] * cross1[0] + cross1[1] * cross1[1] + cross1[2] * cross1[2]);

        std::vector<double> v3 = {coords[2][0] - coords[0][0], coords[2][1] - coords[0][1], coords[2][2] - coords[0][2]};
        std::vector<double> v4 = {coords[3][0] - coords[0][0], coords[3][1] - coords[0][1], coords[3][2] - coords[0][2]};

        std::vector<double> cross2 = {
            v3[1] * v4[2] - v3[2] * v4[1],
            v3[2] * v4[0] - v3[0] * v4[2],
            v3[0] * v4[1] - v3[1] * v4[0]
        };
        double area2 = 0.5 * std::sqrt(cross2[0] * cross2[0] + cross2[1] * cross2[1] + cross2[2] * cross2[2]);

        return area1 + area2;

    } else {
        // その他の多角形要素（簡易的に扇形分割）
        double area = 0.0;
        for (int i = 1; i < n_nodes - 1; ++i) {
            std::vector<double> v1 = {coords[i][0] - coords[0][0], coords[i][1] - coords[0][1], coords[i][2] - coords[0][2]};
            std::vector<double> v2 = {coords[i+1][0] - coords[0][0], coords[i+1][1] - coords[0][1], coords[i+1][2] - coords[0][2]};

            std::vector<double> cross = {
                v1[1] * v2[2] - v1[2] * v2[1],
                v1[2] * v2[0] - v1[0] * v2[2],
                v1[0] * v2[1] - v1[1] * v2[0]
            };

            double magnitude = std::sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]);
            area += 0.5 * magnitude;
        }
        return area;
    }
}

void LoadConditionSetter::writeForceBoundaryCondition(std::ofstream& f, int surface_number,
                                                      double total_force,
                                                      const std::vector<double>& force_direction) const {
    f << "***********************************************************\n";
    f << "** constraints force node loads\n";
    f << "*CLOAD\n";
    f << "** ConstraintForce\n";
    f << "** node loads on shape: Part__Feature:Face" << surface_number << "\n";
    f << "** Total force: " << total_force << " N, Direction: ["
      << force_direction[0] << ", " << force_direction[1] << ", " << force_direction[2] << "]\n";

    // ステップ1: 初期化
    std::map<std::size_t, double> node_areas;  // 各節点の寄与面積
    double total_surface_area = 0.0;  // 面全体の面積

    // 指定された面の要素タグとノードタグを取得
    std::vector<int> element_types;
    std::vector<std::vector<std::size_t>> element_tags;  // 要素番号
    std::vector<std::vector<std::size_t>> node_tags;     // 節点番号
    gmsh::model::mesh::getElements(element_types, element_tags, node_tags, 2, surface_number);

    // ステップ2: 全要素をループして面積を計算・分配
    std::vector<double> element_areas;  // 要素面積の記録用

    for (size_t i = 0; i < element_types.size(); ++i) {
        int elem_type = element_types[i];

        // 要素タイプごとの節点数を取得
        std::string element_name;
        int dim, order, num_nodes, num_primary_nodes;
        std::vector<double> parametric_coords;
        gmsh::model::mesh::getElementProperties(elem_type, element_name, dim, order, num_nodes, parametric_coords, num_primary_nodes);

        // この要素タイプの全要素を処理
        const auto& current_element_tags = element_tags[i];
        int num_elements = current_element_tags.size();

        for (int j = 0; j < num_elements; ++j) {
            std::size_t element_tag = current_element_tags[j];

            // 要素の節点タグを取得
            std::vector<std::size_t> element_node_tags;
            int element_type_out, entity_dim, entity_tag;
            gmsh::model::mesh::getElement(element_tag, element_type_out, element_node_tags, entity_dim, entity_tag);

            // 節点の座標を取得
            std::vector<std::vector<double>> coords;
            for (std::size_t node_tag : element_node_tags) {
                std::vector<double> coord;
                std::vector<double> parametric_coord;
                int entity_dim, entity_tag;
                gmsh::model::mesh::getNode(node_tag, coord, parametric_coord, entity_dim, entity_tag);

                if (coord.size() >= 3) {
                    coords.push_back({coord[0], coord[1], coord[2]});
                } else {
                    coords.push_back(coord);
                }
            }

            // 要素の面積を計算
            double element_area = calculateElementArea(coords);
            total_surface_area += element_area;
            element_areas.push_back(element_area);  // 面積を記録

            // 要素面積を節点に分配
            double area_portion = element_area / num_nodes;
            std::cout << "  各ノードへの面積分配: " << std::fixed << std::setprecision(8) << area_portion << std::endl;

            for (std::size_t node_tag : element_node_tags) {
                node_areas[node_tag] += area_portion;
            }
        }
    }

    // ステップ3: 各節点にかかる力を計算
    if (total_surface_area > 0) {
        double pressure = total_force / total_surface_area;  // 面全体の圧力

        // 力の方向ベクトルを正規化
        std::vector<double> normalized_direction = force_direction;
        double magnitude = std::sqrt(normalized_direction[0] * normalized_direction[0] +
                                   normalized_direction[1] * normalized_direction[1] +
                                   normalized_direction[2] * normalized_direction[2]);
        if (magnitude > 0) {
            for (double& component : normalized_direction) {
                component /= magnitude;
            }
        }

        f << "** Total surface area: " << std::fixed << std::setprecision(6) << total_surface_area << "\n";
        f << "** Pressure: " << std::fixed << std::setprecision(6) << pressure << " N/unit_area\n";

        // 各節点への力を計算・出力
        for (const auto& [node_tag, node_area] : node_areas) {
            double force_magnitude = pressure * node_area;
            std::vector<double> force_vector = {
                force_magnitude * normalized_direction[0],
                force_magnitude * normalized_direction[1],
                force_magnitude * normalized_direction[2]
            };

            // 各自由度に対する力成分を出力
            for (int dof = 1; dof <= 3; ++dof) {
                if (std::abs(force_vector[dof-1]) > 1e-12) {  // 微小な値は無視
                    f << node_tag << "," << dof << ","
                      << std::fixed << std::setprecision(6) << force_vector[dof-1] << "\n";
                }
            }
        }
    } else {
        std::cout << "警告: Surface " << surface_number << " の面積が0です。力の境界条件を適用できません。" << std::endl;
    }
}

const std::vector<LoadCondition>& LoadConditionSetter::getLoads() const {
    return loads_;
}

LoadCondition createLoadCondition(int surface_number, double magnitude,
                                  const std::vector<double>& direction) {
    return {surface_number, magnitude, direction};
}

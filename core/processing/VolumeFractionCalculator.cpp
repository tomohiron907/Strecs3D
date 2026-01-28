#include "VolumeFractionCalculator.h"
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkCell.h>
#include <vtkTetra.h>
#include <vtkCellType.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>

VolumeFractionCalculator::VolumeFractionCalculator() = default;

void VolumeFractionCalculator::clear() {
    m_volumeFractions.clear();
    m_totalVolume = 0.0;
    m_computed = false;
}

bool VolumeFractionCalculator::validateInput(vtkUnstructuredGrid* vtuData,
                                              const std::string& stressLabel,
                                              double stressMin,
                                              double stressMax) {
    if (!vtuData) {
        std::cerr << "[VolumeFraction] Error: No VTU data available." << std::endl;
        return false;
    }
    if (stressMin >= stressMax) {
        std::cerr << "[VolumeFraction] Error: Invalid stress range (min >= max)." << std::endl;
        return false;
    }
    if (stressLabel.empty()) {
        std::cerr << "[VolumeFraction] Error: Stress label is empty." << std::endl;
        return false;
    }
    return true;
}

double VolumeFractionCalculator::computeCellAverageStress(vtkCell* cell, vtkDataArray* stressArray) {
    double avgStress = 0.0;
    vtkIdType numPoints = cell->GetNumberOfPoints();

    for (vtkIdType i = 0; i < numPoints; ++i) {
        vtkIdType pointId = cell->GetPointId(i);
        avgStress += stressArray->GetTuple1(pointId);
    }

    return avgStress / numPoints;
}

double VolumeFractionCalculator::computeTetraVolume(vtkCell* cell) {
    double p0[3], p1[3], p2[3], p3[3];
    cell->GetPoints()->GetPoint(0, p0);
    cell->GetPoints()->GetPoint(1, p1);
    cell->GetPoints()->GetPoint(2, p2);
    cell->GetPoints()->GetPoint(3, p3);
    return std::abs(vtkTetra::ComputeVolume(p0, p1, p2, p3));
}

double VolumeFractionCalculator::computeHexahedronVolume(vtkCell* cell) {
    double points[8][3];
    for (int i = 0; i < 8; ++i) {
        cell->GetPoints()->GetPoint(i, points[i]);
    }

    // 六面体を5つの四面体に分割して体積を計算
    double volume = 0.0;
    volume += std::abs(vtkTetra::ComputeVolume(points[0], points[1], points[3], points[4]));
    volume += std::abs(vtkTetra::ComputeVolume(points[1], points[2], points[3], points[6]));
    volume += std::abs(vtkTetra::ComputeVolume(points[1], points[4], points[5], points[6]));
    volume += std::abs(vtkTetra::ComputeVolume(points[3], points[4], points[6], points[7]));
    volume += std::abs(vtkTetra::ComputeVolume(points[1], points[3], points[4], points[6]));

    return volume;
}

double VolumeFractionCalculator::computeCellVolume(vtkCell* cell) {
    if (!cell) return 0.0;

    int cellType = cell->GetCellType();

    switch (cellType) {
        case VTK_TETRA:
        case VTK_QUADRATIC_TETRA:  // 二次四面体（10ノード）も頂点4点で体積計算可能
            return computeTetraVolume(cell);
        case VTK_HEXAHEDRON:
        case VTK_QUADRATIC_HEXAHEDRON:  // 二次六面体（20ノード）も頂点8点で体積計算可能
            return computeHexahedronVolume(cell);
        default:
            // 未対応のセルタイプは0として扱う
            return 0.0;
    }
}

int VolumeFractionCalculator::determineBinIndex(double stress,
                                                 double stressMin,
                                                 double stressStep,
                                                 int numDivisions) {
    int binIndex = static_cast<int>((stress - stressMin) / stressStep);
    return std::clamp(binIndex, 0, numDivisions - 1);
}

bool VolumeFractionCalculator::compute(vtkUnstructuredGrid* vtuData,
                                        const std::string& stressLabel,
                                        double stressMin,
                                        double stressMax,
                                        int numDivisions) {
    std::cerr << "[VolumeFraction] ===== Starting volume fraction calculation =====" << std::endl;
    std::cerr << "[VolumeFraction] Input parameters:" << std::endl;
    std::cerr << "[VolumeFraction]   stressLabel: " << stressLabel << std::endl;
    std::cerr << "[VolumeFraction]   stressMin: " << stressMin << std::endl;
    std::cerr << "[VolumeFraction]   stressMax: " << stressMax << std::endl;
    std::cerr << "[VolumeFraction]   numDivisions: " << numDivisions << std::endl;
    std::cerr << "[VolumeFraction]   vtuData pointer: " << vtuData << std::endl;

    // 入力検証
    if (!validateInput(vtuData, stressLabel, stressMin, stressMax)) {
        std::cerr << "[VolumeFraction] Input validation failed!" << std::endl;
        return false;
    }
    std::cerr << "[VolumeFraction] Input validation passed." << std::endl;

    // 初期化
    clear();
    m_volumeFractions.resize(numDivisions, 0.0);

    // 応力データを取得
    std::cerr << "[VolumeFraction] Getting stress array..." << std::endl;
    vtkDataArray* stressArray = vtuData->GetPointData()->GetScalars(stressLabel.c_str());
    if (!stressArray) {
        std::cerr << "[VolumeFraction] Error: Could not get stress data for label: " << stressLabel << std::endl;

        // 利用可能な配列名をリスト
        std::cerr << "[VolumeFraction] Available arrays in PointData:" << std::endl;
        vtkPointData* pointData = vtuData->GetPointData();
        int numArrays = pointData->GetNumberOfArrays();
        for (int i = 0; i < numArrays; ++i) {
            const char* name = pointData->GetArrayName(i);
            std::cerr << "[VolumeFraction]   [" << i << "]: " << (name ? name : "(null)") << std::endl;
        }
        return false;
    }
    std::cerr << "[VolumeFraction] Stress array obtained successfully." << std::endl;
    std::cerr << "[VolumeFraction]   Array name: " << (stressArray->GetName() ? stressArray->GetName() : "(null)") << std::endl;
    std::cerr << "[VolumeFraction]   Number of tuples: " << stressArray->GetNumberOfTuples() << std::endl;
    std::cerr << "[VolumeFraction]   Number of components: " << stressArray->GetNumberOfComponents() << std::endl;

    double stressStep = (stressMax - stressMin) / numDivisions;
    vtkIdType numCells = vtuData->GetNumberOfCells();
    std::cerr << "[VolumeFraction] stressStep: " << stressStep << std::endl;
    std::cerr << "[VolumeFraction] Number of cells: " << numCells << std::endl;

    // セルタイプの統計を収集
    std::map<int, int> cellTypeCounts;
    int processedCells = 0;
    int skippedCellsNoVolume = 0;

    // 各セルを処理
    for (vtkIdType cellId = 0; cellId < numCells; ++cellId) {
        vtkCell* cell = vtuData->GetCell(cellId);
        if (!cell) {
            std::cerr << "[VolumeFraction] Warning: Cell " << cellId << " is null" << std::endl;
            continue;
        }

        int cellType = cell->GetCellType();
        cellTypeCounts[cellType]++;

        // セルの平均応力を計算
        double avgStress = computeCellAverageStress(cell, stressArray);

        // セルの体積を計算
        double cellVolume = computeCellVolume(cell);

        // 最初の10セルの詳細をデバッグ出力
        if (cellId < 10) {
            std::cerr << "[VolumeFraction] Cell " << cellId << ": type=" << cellType
                      << ", avgStress=" << avgStress << ", volume=" << cellVolume << std::endl;
        }

        if (cellVolume <= 0.0) {
            skippedCellsNoVolume++;
            continue;
        }

        processedCells++;
        m_totalVolume += cellVolume;

        // 該当する区間に体積を加算
        int binIndex = determineBinIndex(avgStress, stressMin, stressStep, numDivisions);
        m_volumeFractions[binIndex] += cellVolume;
    }

    // セルタイプの統計を出力
    std::cerr << "[VolumeFraction] Cell type statistics:" << std::endl;
    for (const auto& pair : cellTypeCounts) {
        std::cerr << "[VolumeFraction]   CellType " << pair.first << ": " << pair.second << " cells";
        if (pair.first == VTK_TETRA) {
            std::cerr << " (VTK_TETRA)";
        } else if (pair.first == VTK_QUADRATIC_TETRA) {
            std::cerr << " (VTK_QUADRATIC_TETRA)";
        } else if (pair.first == VTK_HEXAHEDRON) {
            std::cerr << " (VTK_HEXAHEDRON)";
        } else if (pair.first == VTK_QUADRATIC_HEXAHEDRON) {
            std::cerr << " (VTK_QUADRATIC_HEXAHEDRON)";
        } else {
            std::cerr << " (unsupported)";
        }
        std::cerr << std::endl;
    }

    std::cerr << "[VolumeFraction] Processed cells: " << processedCells << std::endl;
    std::cerr << "[VolumeFraction] Skipped cells (no volume): " << skippedCellsNoVolume << std::endl;
    std::cerr << "[VolumeFraction] Total volume: " << m_totalVolume << std::endl;

    // 体積分率に変換前の値を出力
    std::cerr << "[VolumeFraction] Volume per bin (before normalization):" << std::endl;
    for (int i = 0; i < numDivisions; ++i) {
        std::cerr << "[VolumeFraction]   Bin " << i << ": " << m_volumeFractions[i] << std::endl;
    }

    // 体積分率に変換
    if (m_totalVolume > 0.0) {
        for (double& fraction : m_volumeFractions) {
            fraction /= m_totalVolume;
        }
    } else {
        std::cerr << "[VolumeFraction] Warning: Total volume is 0!" << std::endl;
    }

    // 最終結果を出力
    std::cerr << "[VolumeFraction] Volume fractions (after normalization):" << std::endl;
    double sum = 0.0;
    for (int i = 0; i < numDivisions; ++i) {
        std::cerr << "[VolumeFraction]   Bin " << i << ": " << m_volumeFractions[i] << std::endl;
        sum += m_volumeFractions[i];
    }
    std::cerr << "[VolumeFraction] Sum of fractions: " << sum << std::endl;

    m_computed = true;
    std::cerr << "[VolumeFraction] ===== Calculation completed =====" << std::endl;
    return true;
}

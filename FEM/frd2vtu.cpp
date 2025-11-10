#include "frd2vtu.h"
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkCellType.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>

// 文字列の先頭と末尾の空白を削除するヘルパー関数
void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

int convertFrdToVtu(const std::string& frd_filename, const std::string& vtu_filename) {

    std::ifstream frd_file(frd_filename);
    if (!frd_file.is_open()) {
        return EXIT_FAILURE;
    }

    // --- VTKオブジェクトの準備 ---
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();

    // --- 結果データ用配列の準備 ---
    auto displacement = vtkSmartPointer<vtkDoubleArray>::New();
    displacement->SetName("Displacement");
    displacement->SetNumberOfComponents(3);

    auto stress = vtkSmartPointer<vtkDoubleArray>::New();
    stress->SetName("Stress");
    stress->SetNumberOfComponents(6); // Sxx, Syy, Szz, Sxy, Syz, Szx

    auto strain = vtkSmartPointer<vtkDoubleArray>::New();
    strain->SetName("Total_Strain");
    strain->SetNumberOfComponents(6); // Exx, Eyy, Ezz, Exy, Eyz, Ezx

    auto error = vtkSmartPointer<vtkDoubleArray>::New();
    error->SetName("Estimation_Error");
    error->SetNumberOfComponents(1);

    auto vonMisesStress = vtkSmartPointer<vtkDoubleArray>::New();
    vonMisesStress->SetName("von Mises Stress");
    vonMisesStress->SetNumberOfComponents(1);

    // --- ファイル解析 ---
    std::string line;
    // ファイルの状態を管理する変数
    enum class ParserState { NONE, NODES, ELEMENTS, DISP, STRESS, STRAIN, ERROR };
    ParserState state = ParserState::NONE;

    while (std::getline(frd_file, line)) {
        trim(line);
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string keyword;
        ss >> keyword;

        // --- 状態遷移の判定 ---
        if (keyword == "2C") {
            state = ParserState::NODES;
            continue;
        } else if (keyword == "3C") {
            state = ParserState::ELEMENTS;
            continue;
        } else if (keyword == "-4") {
            std::string result_type;
            ss >> result_type;
            if (result_type == "DISP") state = ParserState::DISP;
            else if (result_type == "STRESS") state = ParserState::STRESS;
            else if (result_type == "TOSTRAIN") state = ParserState::STRAIN;
            else if (result_type == "ERROR") state = ParserState::ERROR;
            else state = ParserState::NONE;
            continue;
        } else if (keyword == "-3" || keyword == "9999") { // ブロック終了
            state = ParserState::NONE;
            continue;
        }

        // --- 各状態に応じたデータ処理 ---
        if (keyword == "-1") {
            switch (state) {
                case ParserState::NODES: {
                    int nodeId;
                    double x, y, z;
                    ss >> nodeId >> x >> y >> z;
                    points->InsertNextPoint(x, y, z);
                    break;
                }
                case ParserState::ELEMENTS: {
                    // 要素ヘッダ行は読み飛ばし、次の-2行を読む
                    if (std::getline(frd_file, line)) {
                        trim(line);
                        std::stringstream ss_nodes(line);
                        std::string nodes_keyword;
                        ss_nodes >> nodes_keyword; // "-2" を読み飛ばす
                        
                        std::vector<vtkIdType> nodeIds;
                        int nodeId_val;
                        while (ss_nodes >> nodeId_val) {
                            // FRDは1-based, VTKは0-basedなので-1する
                            nodeIds.push_back(nodeId_val - 1);
                        }

                        // ノード数から要素タイプを判断
                        if (nodeIds.size() == 10) { // 10ノード -> 二次四面体
                            unstructuredGrid->InsertNextCell(VTK_QUADRATIC_TETRA, nodeIds.size(), nodeIds.data());
                        }
                        // ここに他の要素タイプ（8ノードならVTK_HEXAHEDRONなど）の判定を追加可能
                    }
                    break;
                }
                case ParserState::DISP: {
                    int nodeId;
                    double d1, d2, d3;
                    ss >> nodeId >> d1 >> d2 >> d3;
                    displacement->InsertNextTuple3(d1, d2, d3);
                    break;
                }
                case ParserState::STRESS: {
                    int nodeId;
                    double s1, s2, s3, s4, s5, s6;
                    ss >> nodeId >> s1 >> s2 >> s3 >> s4 >> s5 >> s6;
                    
                    // MPaからPaに変換（1 MPa = 1e6 Pa）
                    s1 *= 1e6; s2 *= 1e6; s3 *= 1e6;
                    s4 *= 1e6; s5 *= 1e6; s6 *= 1e6;
                    
                    stress->InsertNextTuple6(s1, s2, s3, s4, s5, s6);
                    
                    // von Mises応力を計算
                    // von Mises = sqrt(0.5 * ((s1-s2)^2 + (s2-s3)^2 + (s3-s1)^2 + 6*(s4^2 + s5^2 + s6^2)))
                    double vonMises = std::sqrt(0.5 * (
                        (s1 - s2) * (s1 - s2) + 
                        (s2 - s3) * (s2 - s3) + 
                        (s3 - s1) * (s3 - s1) + 
                        6.0 * (s4 * s4 + s5 * s5 + s6 * s6)
                    ));
                    vonMisesStress->InsertNextTuple1(vonMises);
                    break;
                }
                case ParserState::STRAIN: {
                     int nodeId;
                    double e1, e2, e3, e4, e5, e6;
                    ss >> nodeId >> e1 >> e2 >> e3 >> e4 >> e5 >> e6;
                    strain->InsertNextTuple6(e1, e2, e3, e4, e5, e6);
                    break;
                }
                case ParserState::ERROR: {
                    int nodeId;
                    double err_val;
                    ss >> nodeId >> err_val;
                    error->InsertNextTuple1(err_val);
                    break;
                }
                default:
                    break;
            }
        }
    }
    frd_file.close();

    // --- 組み立てたデータをUnstructuredGridに設定 ---
    unstructuredGrid->SetPoints(points);

    // 各データ配列をPointDataに追加
    unstructuredGrid->GetPointData()->AddArray(displacement);
    unstructuredGrid->GetPointData()->AddArray(stress);
    unstructuredGrid->GetPointData()->AddArray(strain);
    unstructuredGrid->GetPointData()->AddArray(error);
    unstructuredGrid->GetPointData()->AddArray(vonMisesStress);


    // --- VTUファイルに書き出し ---
    auto writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetFileName(vtu_filename.c_str());
    writer->SetInputData(unstructuredGrid);
    writer->SetDataModeToAscii(); // テキスト形式で出力
    writer->Write();


    return EXIT_SUCCESS;
}


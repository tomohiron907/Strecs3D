#pragma once

#include <string>
#include <vector>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyData.h>

class TopoDS_Shape;

// 面のジオメトリ情報を保持する構造体
struct FaceGeometry {
    double centerX, centerY, centerZ;  // 面の中心座標
    double normalX, normalY, normalZ;  // 面の法線ベクトル
    bool isValid;                       // 取得成功フラグ
};

class StepReader {
public:
    StepReader();
    ~StepReader();

    // STEPファイルを読み込む
    bool readStepFile(const std::string& filename);

    // OpenCASCADE形状をVTKアクターに変換（面とエッジを保持）
    vtkSmartPointer<vtkActor> getFacesActor() const;
    vtkSmartPointer<vtkActor> getEdgesActor() const;

    // 形状が正常に読み込まれたかチェック
    bool isValid() const;

    // 面の総数を取得
    int getFaceCount() const { return faceCount_; }

    // 面ごとのアクターを取得（ホバー検出用）
    std::vector<vtkSmartPointer<vtkActor>> getFaceActors() const;

    // 面の中心と法線を取得（surface_idは1-based）
    FaceGeometry getFaceGeometry(int surfaceId) const;

private:
    TopoDS_Shape* shape_;
    bool isValid_;
    int faceCount_;

    // OpenCASCADE形状をVTKポリデータに変換
    vtkSmartPointer<vtkPolyData> convertFacesToPolyData() const;
    vtkSmartPointer<vtkPolyData> convertEdgesToPolyData() const;
};

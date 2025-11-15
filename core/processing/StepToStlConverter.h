#pragma once

#include <string>
#include <QString>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

class TopoDS_Shape;

/**
 * STEPファイルをSTLファイルに変換するクラス
 * OpenCASCADEを使用してSTEPファイルを読み込み、
 * VTKを使用してバイナリSTLファイルとして保存する
 */
class StepToStlConverter {
public:
    StepToStlConverter();
    ~StepToStlConverter();

    /**
     * STEPファイルをSTLファイルに変換
     * @param stepFilePath 入力STEPファイルのパス
     * @param outputStlPath 出力STLファイルのパス
     * @return 成功した場合true、失敗した場合false
     */
    bool convertStepToStl(const std::string& stepFilePath,
                          const std::string& outputStlPath);

    /**
     * STEPファイルを読み込み、Strecs3D.temp/meshディレクトリにSTLとして保存
     * @param stepFilePath 入力STEPファイルのパス
     * @return 保存したSTLファイルのパス（失敗時は空文字列）
     */
    QString convertAndSave(const QString& stepFilePath);

    /**
     * メッシュ解像度を設定（デフォルト: 0.01）
     * 値が小さいほど細かいメッシュになる
     * @param resolution メッシュ解像度
     */
    void setMeshResolution(double resolution) { meshResolution_ = resolution; }

    /**
     * 現在のメッシュ解像度を取得
     * @return メッシュ解像度
     */
    double getMeshResolution() const { return meshResolution_; }

private:
    double meshResolution_;  // メッシュ化の解像度

    /**
     * OpenCASCADE ShapeをvtkPolyDataに変換
     * @param shape OpenCASCADE形状
     * @return VTKポリデータ
     */
    vtkSmartPointer<vtkPolyData> shapeToPolyData(const TopoDS_Shape& shape);

    /**
     * vtkPolyDataをバイナリSTLファイルとして保存
     * @param polyData VTKポリデータ
     * @param outputPath 出力ファイルパス
     * @return 成功した場合true、失敗した場合false
     */
    bool saveAsBinaryStl(vtkSmartPointer<vtkPolyData> polyData,
                        const std::string& outputPath);

    /**
     * ファイルパスから拡張子を除いたベース名を取得
     * @param filePath ファイルパス
     * @return ベース名
     */
    std::string getFileBaseName(const std::string& filePath);
};

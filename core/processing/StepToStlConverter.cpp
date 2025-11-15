#include "StepToStlConverter.h"
#include "../../utils/tempPathUtility.h"

// OpenCASCADE includes
#include <STEPControl_Reader.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <gp_Pnt.hxx>
#include <TopLoc_Location.hxx>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkSTLWriter.h>
#include <vtkPolyDataNormals.h>

// Qt includes
#include <QDir>
#include <QFileInfo>
#include <QString>

#include <iostream>

StepToStlConverter::StepToStlConverter()
    : meshResolution_(0.01)
{
}

StepToStlConverter::~StepToStlConverter()
{
}

bool StepToStlConverter::convertStepToStl(const std::string& stepFilePath,
                                          const std::string& outputStlPath)
{
    try {
        // 1. STEPファイルを読み込む
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(stepFilePath.c_str());

        if (status != IFSelect_RetDone) {
            std::cerr << "Error reading STEP file: " << stepFilePath << std::endl;
            return false;
        }

        // ファイルから形状を転送
        reader.TransferRoots();

        // 形状を取得
        TopoDS_Shape shape = reader.OneShape();

        if (shape.IsNull()) {
            std::cerr << "Failed to get shape from STEP file" << std::endl;
            return false;
        }

        std::cout << "Successfully loaded STEP file: " << stepFilePath << std::endl;

        // 2. 形状をメッシュ化（テッセレーション）
        BRepMesh_IncrementalMesh mesh(shape, meshResolution_);
        mesh.Perform();

        if (!mesh.IsDone()) {
            std::cerr << "Failed to mesh the STEP shape" << std::endl;
            return false;
        }

        std::cout << "Successfully meshed STEP shape with resolution: " << meshResolution_ << std::endl;

        // 3. OpenCASCADE ShapeをvtkPolyDataに変換
        vtkSmartPointer<vtkPolyData> polyData = shapeToPolyData(shape);

        if (!polyData || polyData->GetNumberOfPoints() == 0) {
            std::cerr << "Failed to convert STEP shape to VTK PolyData" << std::endl;
            return false;
        }

        std::cout << "Converted to VTK PolyData: "
                  << polyData->GetNumberOfPoints() << " points, "
                  << polyData->GetNumberOfCells() << " triangles" << std::endl;

        // 4. バイナリSTLファイルとして保存
        if (!saveAsBinaryStl(polyData, outputStlPath)) {
            std::cerr << "Failed to save STL file: " << outputStlPath << std::endl;
            return false;
        }

        std::cout << "Successfully saved binary STL file: " << outputStlPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in convertStepToStl: " << e.what() << std::endl;
        return false;
    }
}

QString StepToStlConverter::convertAndSave(const QString& stepFilePath)
{
    try {
        // 1. meshディレクトリのパスを取得
        QString meshDir = TempPathUtility::getTempSubDir("mesh");

        // 2. meshディレクトリが存在しない場合は作成
        QDir dir;
        if (!dir.exists(meshDir)) {
            if (!dir.mkpath(meshDir)) {
                std::cerr << "Failed to create mesh directory: "
                          << meshDir.toStdString() << std::endl;
                return QString();
            }
            std::cout << "Created mesh directory: " << meshDir.toStdString() << std::endl;
        }

        // 3. 出力ファイル名を生成（元のファイル名 + .stl）
        QFileInfo fileInfo(stepFilePath);
        QString baseName = fileInfo.completeBaseName();  // 拡張子を除いたファイル名
        QString outputFileName = baseName + ".stl";
        QString outputPath = meshDir + "/" + outputFileName;

        std::cout << "Converting STEP to STL..." << std::endl;
        std::cout << "  Input:  " << stepFilePath.toStdString() << std::endl;
        std::cout << "  Output: " << outputPath.toStdString() << std::endl;

        // 4. STEPファイルをSTLに変換
        if (!convertStepToStl(stepFilePath.toStdString(), outputPath.toStdString())) {
            return QString();
        }

        return outputPath;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in convertAndSave: " << e.what() << std::endl;
        return QString();
    }
}

vtkSmartPointer<vtkPolyData> StepToStlConverter::shapeToPolyData(const TopoDS_Shape& shape)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

    // すべての面を探索
    for (TopExp_Explorer faceExp(shape, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);

        if (triangulation.IsNull()) {
            continue;
        }

        // 現在のポイント数を記録（オフセットとして使用）
        vtkIdType baseIndex = points->GetNumberOfPoints();

        // 頂点を追加（OpenCASCADE 7.9.2のAPIを使用）
        for (Standard_Integer i = 1; i <= triangulation->NbNodes(); i++) {
            gp_Pnt p = triangulation->Node(i).Transformed(location.Transformation());
            points->InsertNextPoint(p.X(), p.Y(), p.Z());
        }

        // 三角形を追加（OpenCASCADE 7.9.2のAPIを使用）
        for (Standard_Integer i = 1; i <= triangulation->NbTriangles(); i++) {
            const Poly_Triangle& tri = triangulation->Triangle(i);
            Standard_Integer n1, n2, n3;
            tri.Get(n1, n2, n3);

            vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();

            // 面の向きに応じて頂点の順序を調整
            if (face.Orientation() == TopAbs_REVERSED) {
                triangle->GetPointIds()->SetId(0, baseIndex + n1 - 1);
                triangle->GetPointIds()->SetId(1, baseIndex + n3 - 1);
                triangle->GetPointIds()->SetId(2, baseIndex + n2 - 1);
            } else {
                triangle->GetPointIds()->SetId(0, baseIndex + n1 - 1);
                triangle->GetPointIds()->SetId(1, baseIndex + n2 - 1);
                triangle->GetPointIds()->SetId(2, baseIndex + n3 - 1);
            }

            triangles->InsertNextCell(triangle);
        }
    }

    // PolyDataを作成
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);

    // 法線を計算（STLファイルには法線情報が必要）
    vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGenerator->SetInputData(polyData);
    normalGenerator->ComputePointNormalsOff();
    normalGenerator->ComputeCellNormalsOn();
    normalGenerator->Update();

    return normalGenerator->GetOutput();
}

bool StepToStlConverter::saveAsBinaryStl(vtkSmartPointer<vtkPolyData> polyData,
                                         const std::string& outputPath)
{
    try {
        vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
        writer->SetFileName(outputPath.c_str());
        writer->SetInputData(polyData);
        writer->SetFileTypeToBinary();  // バイナリ形式で保存
        writer->Write();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in saveAsBinaryStl: " << e.what() << std::endl;
        return false;
    }
}

std::string StepToStlConverter::getFileBaseName(const std::string& filePath)
{
    size_t lastSlash = filePath.find_last_of("/\\");
    size_t lastDot = filePath.find_last_of(".");

    std::string fileName = (lastSlash == std::string::npos)
                          ? filePath
                          : filePath.substr(lastSlash + 1);

    if (lastDot != std::string::npos && lastDot > lastSlash) {
        fileName = fileName.substr(0, lastDot - (lastSlash + 1));
    }

    return fileName;
}

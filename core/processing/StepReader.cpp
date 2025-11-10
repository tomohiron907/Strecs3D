#include "StepReader.h"

// OpenCASCADE includes
#include <STEPControl_Reader.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <gp_Pnt.hxx>
#include <TColgp_Array1OfPnt.hxx>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkLine.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyDataNormals.h>

#include <iostream>

StepReader::StepReader()
    : shape_(nullptr)
    , isValid_(false)
{
}

StepReader::~StepReader()
{
    if (shape_) {
        delete shape_;
    }
}

bool StepReader::readStepFile(const std::string& filename)
{
    try {
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filename.c_str());

        if (status != IFSelect_RetDone) {
            std::cerr << "Error reading STEP file: " << filename << std::endl;
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

        // 形状をメッシュ化（テッセレーション）
        // 第2パラメータを小さくすると、より細かいメッシュになります（デフォルト0.1を0.01に変更）
        BRepMesh_IncrementalMesh mesh(shape, 0.01);
        mesh.Perform();

        if (shape_) {
            delete shape_;
        }
        shape_ = new TopoDS_Shape(shape);
        isValid_ = true;

        std::cout << "Successfully loaded STEP file: " << filename << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Exception while reading STEP file: " << e.what() << std::endl;
        isValid_ = false;
        return false;
    }
}

bool StepReader::isValid() const
{
    return isValid_ && shape_ != nullptr;
}

vtkSmartPointer<vtkPolyData> StepReader::convertFacesToPolyData() const
{
    if (!isValid()) {
        return nullptr;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

    // すべての面を探索
    for (TopExp_Explorer faceExp(*shape_, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);

        if (triangulation.IsNull()) {
            continue;
        }

        // 三角形分割から点を追加
        vtkIdType baseIndex = points->GetNumberOfPoints();

        for (Standard_Integer i = 1; i <= triangulation->NbNodes(); i++) {
            gp_Pnt p = triangulation->Node(i).Transformed(location.Transformation());
            points->InsertNextPoint(p.X(), p.Y(), p.Z());
        }

        // 三角形を追加
        for (Standard_Integer i = 1; i <= triangulation->NbTriangles(); i++) {
            const Poly_Triangle& tri = triangulation->Triangle(i);
            Standard_Integer n1, n2, n3;
            tri.Get(n1, n2, n3);

            vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();

            // 面の向きに応じて頂点順序を調整
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

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);

    return polyData;
}

vtkSmartPointer<vtkPolyData> StepReader::convertEdgesToPolyData() const
{
    if (!isValid()) {
        return nullptr;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // すべてのエッジを探索
    for (TopExp_Explorer edgeExp(*shape_, TopAbs_EDGE); edgeExp.More(); edgeExp.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());

        if (BRep_Tool::Degenerated(edge)) {
            continue;
        }

        BRepAdaptor_Curve curve(edge);
        Standard_Real first = curve.FirstParameter();
        Standard_Real last = curve.LastParameter();

        // エッジを離散化
        const int numPoints = 50;
        Standard_Real step = (last - first) / (numPoints - 1);

        vtkIdType baseIndex = points->GetNumberOfPoints();

        for (int i = 0; i < numPoints; i++) {
            Standard_Real param = first + i * step;
            gp_Pnt p = curve.Value(param);
            points->InsertNextPoint(p.X(), p.Y(), p.Z());
        }

        // ラインセグメントを作成
        for (int i = 0; i < numPoints - 1; i++) {
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, baseIndex + i);
            line->GetPointIds()->SetId(1, baseIndex + i + 1);
            lines->InsertNextCell(line);
        }
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    return polyData;
}

vtkSmartPointer<vtkActor> StepReader::getFacesActor() const
{
    vtkSmartPointer<vtkPolyData> polyData = convertFacesToPolyData();
    if (!polyData) {
        return nullptr;
    }

    // 法線ベクトルを計算して滑らかなシェーディングを実現
    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData(polyData);
    normals->ComputePointNormalsOn();  // 頂点法線を計算
    normals->ComputeCellNormalsOff();  // セル法線は計算しない
    normals->SplittingOff();           // エッジを分割しない（滑らかに）
    normals->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(normals->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 面の色と外観を設定
    actor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    actor->GetProperty()->SetOpacity(1);
    actor->GetProperty()->SetInterpolationToPhong();  // Phongシェーディングで滑らかに

    // Polygon Offsetを設定してエッジとの重なりを防ぐ
    // 面を少し奥に押し込むことで、エッジが常に手前に表示される
    mapper->SetResolveCoincidentTopologyToPolygonOffset();
    mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0.5, 0.5);

    return actor;
}

vtkSmartPointer<vtkActor> StepReader::getEdgesActor() const
{
    vtkSmartPointer<vtkPolyData> polyData = convertEdgesToPolyData();
    if (!polyData) {
        return nullptr;
    }

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // エッジの色と外観を設定
    actor->GetProperty()->SetColor(0.0, 0.0, 0.0); // 黒
    actor->GetProperty()->SetLineWidth(2.0);

    // エッジを面より手前に描画するための設定
    mapper->SetResolveCoincidentTopologyToPolygonOffset();
    mapper->SetResolveCoincidentTopologyLineOffsetParameters(-0.5, -0.5);

    return actor;
}

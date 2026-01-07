#include "BoundaryConditionVisualizer.h"
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkMath.h>
#include <iostream>
#include <cmath>

BoundaryConditionVisualizer::BoundaryConditionVisualizer() {
}

BoundaryConditionActors BoundaryConditionVisualizer::createBoundaryConditionActors(
    const BoundaryCondition& condition,
    const StepReader* stepReader)
{
    BoundaryConditionActors result;

    if (!stepReader || !stepReader->isValid()) {
        std::cerr << "StepReader is null or invalid" << std::endl;
        return result;
    }

    // 拘束条件の可視化
    for (const auto& constraint : condition.constraints) {
        FaceGeometry geom = stepReader->getFaceGeometry(constraint.surface_id);

        if (!geom.isValid) {
            std::cerr << "Failed to get geometry for constraint surface_id: "
                     << constraint.surface_id << std::endl;
            continue;
        }

        auto actor = createConstraintActor(geom.centerX, geom.centerY, geom.centerZ);
        if (actor) {
            result.constraintActors.push_back(actor);
        }
    }

    // 荷重条件の可視化
    for (const auto& load : condition.loads) {
        FaceGeometry geom = stepReader->getFaceGeometry(load.surface_id);

        if (!geom.isValid) {
            std::cerr << "Failed to get geometry for load surface_id: "
                     << load.surface_id << std::endl;
            continue;
        }

        auto actor = createLoadArrowActor(
            geom.centerX, geom.centerY, geom.centerZ,
            load.direction.x, load.direction.y, load.direction.z,
            geom.normalX, geom.normalY, geom.normalZ
        );

        if (actor) {
            result.loadActors.push_back(actor);
        }
    }

    return result;
}

vtkSmartPointer<vtkActor> BoundaryConditionVisualizer::createConstraintActor(
    double centerX, double centerY, double centerZ)
{
    // 直方体を生成（5mm角）
    vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
    cube->SetXLength(CONSTRAINT_CUBE_SIZE);
    cube->SetYLength(CONSTRAINT_CUBE_SIZE);
    cube->SetZLength(CONSTRAINT_CUBE_SIZE);
    cube->SetCenter(centerX, centerY, centerZ);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cube->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 拘束条件は緑色で表示
    actor->GetProperty()->SetColor(0.0, 1.0, 0.0);
    actor->GetProperty()->SetOpacity(0.8);

    return actor;
}

vtkSmartPointer<vtkActor> BoundaryConditionVisualizer::createLoadArrowActor(
    double centerX, double centerY, double centerZ,
    double dirX, double dirY, double dirZ,
    double normalX, double normalY, double normalZ)
{
    // 方向ベクトルを正規化
    double length = std::sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    if (length < 1e-6) {
        std::cerr << "Invalid direction vector (zero length)" << std::endl;
        return nullptr;
    }

    dirX /= length;
    dirY /= length;
    dirZ /= length;

    // 法線ベクトルと力のベクトルの内積を計算
    double dotProduct = normalX * dirX + normalY * dirY + normalZ * dirZ;

    // 内積が正（引張）の場合、矢印の根本を面の中心に合わせる
    // 現在の仕様では先端（円錐の先）が配置位置に来るため、
    // 引張の場合は矢印の長さ分だけ力の方向にずらす
    if (dotProduct > 0) {
        double arrowLength = ARROW_CONE_HEIGHT + ARROW_CYLINDER_LENGTH;
        centerX += dirX * arrowLength;
        centerY += dirY * arrowLength;
        centerZ += dirZ * arrowLength;
    }

    // 矢印の円柱部分を作成（Y軸方向、中心が原点）
    vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    cylinder->SetRadius(ARROW_CYLINDER_RADIUS);
    cylinder->SetHeight(ARROW_CYLINDER_LENGTH);
    cylinder->SetResolution(16);

    // 円柱をローカル座標で配置：円錐の底面から円柱の長さの半分だけ下
    // 円錐の底面は y = -ARROW_CONE_HEIGHT/2 なので、
    // 円柱の中心は y = -ARROW_CONE_HEIGHT/2 - ARROW_CYLINDER_LENGTH/2
    double cylinderLocalY = -(ARROW_CONE_HEIGHT / 2.0 + ARROW_CYLINDER_LENGTH / 2.0);
    cylinder->SetCenter(0, cylinderLocalY, 0);

    // 矢印の円錐部分を作成（Y軸方向、中心が原点、先端がy正方向）
    vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
    cone->SetRadius(ARROW_CONE_RADIUS);
    cone->SetHeight(ARROW_CONE_HEIGHT);
    cone->SetResolution(16);
    cone->SetDirection(0, 1, 0);

    // 円錐の中心をローカル座標で配置：先端がy=0になるように
    // 円錐の先端は中心から height/2 上にあるので、
    // 中心を y = -ARROW_CONE_HEIGHT/2 に配置すると先端がy=0になる
    cone->SetCenter(0, -ARROW_CONE_HEIGHT / 2.0, 0);

    // 円柱と円錐を結合（ローカル座標系で）
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    appendFilter->AddInputConnection(cylinder->GetOutputPort());
    appendFilter->AddInputConnection(cone->GetOutputPort());
    appendFilter->Update();

    // 共通のTransformを作成（回転と平行移動）
    // VTKのTransformは逆順に適用されるため、Translate -> Rotate の順で書く
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();

    // 矢印全体を面の中心に移動（先端が面の中心に来る）
    // この操作は最後に適用される
    transform->Translate(centerX, centerY, centerZ);

    // Y軸から目的の方向への回転を計算
    double yAxis[3] = {0, 1, 0};
    double targetDir[3] = {dirX, dirY, dirZ};

    // 回転軸を外積で計算
    double rotationAxis[3];
    vtkMath::Cross(yAxis, targetDir, rotationAxis);
    double rotationAxisLength = vtkMath::Norm(rotationAxis);

    if (rotationAxisLength > 1e-6) {
        // 回転角度を内積で計算
        double angle = std::acos(vtkMath::Dot(yAxis, targetDir)) * 180.0 / vtkMath::Pi();

        // 回転軸を正規化
        vtkMath::Normalize(rotationAxis);

        // 回転を適用（この操作は最初に適用される）
        transform->RotateWXYZ(angle, rotationAxis);
    }

    // Transformを適用
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(appendFilter->GetOutputPort());
    transformFilter->SetTransform(transform);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(transformFilter->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 荷重条件は赤色で表示
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    actor->GetProperty()->SetOpacity(0.8);

    return actor;
}

#include "TurntableInteractorStyle.h"
#include <vtkObjectFactory.h>

vtkStandardNewMacro(TurntableInteractorStyle);

void TurntableInteractorStyle::Rotate()
{
    if (this->CurrentRenderer == nullptr) {
        return;
    }

    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
    if (camera == nullptr) {
        return;
    }

    // 現在のマウス位置と前回のマウス位置を取得
    int deltaX = this->Interactor->GetEventPosition()[0] - this->Interactor->GetLastEventPosition()[0];
    int deltaY = this->Interactor->GetEventPosition()[1] - this->Interactor->GetLastEventPosition()[1];
    
    // 回転の感度を調整（azimuthを反転させて自然な回転方向にする）
    double azimuth = -deltaX * this->MotionFactor * rotationSpeed_;
    double elevation = deltaY * this->MotionFactor * rotationSpeed_;
    
    // 現在のカメラ位置と焦点を取得
    double* position = camera->GetPosition();
    double* focalPoint = camera->GetFocalPoint();
    double* viewUp = camera->GetViewUp();
    
    // 焦点からカメラ位置へのベクトル
    double direction[3];
    direction[0] = position[0] - focalPoint[0];
    direction[1] = position[1] - focalPoint[1];
    direction[2] = position[2] - focalPoint[2];
    
    // 水平回転（Z軸周り）
    double cosAz = cos(vtkMath::RadiansFromDegrees(azimuth));
    double sinAz = sin(vtkMath::RadiansFromDegrees(azimuth));
    
    double newDirection[3];
    newDirection[0] = direction[0] * cosAz - direction[1] * sinAz;
    newDirection[1] = direction[0] * sinAz + direction[1] * cosAz;
    newDirection[2] = direction[2];
    
    // 垂直回転（水平面内で制限）
    double horizontalLength = sqrt(newDirection[0] * newDirection[0] + newDirection[1] * newDirection[1]);
    double currentElevationAngle = atan2(newDirection[2], horizontalLength);
    double newElevationAngle = currentElevationAngle - vtkMath::RadiansFromDegrees(elevation);
    
    // 垂直角度を制限（-85度から85度まで）
    const double maxElevation = vtkMath::RadiansFromDegrees(85.0);
    const double minElevation = vtkMath::RadiansFromDegrees(-85.0);
    if (newElevationAngle > maxElevation) newElevationAngle = maxElevation;
    if (newElevationAngle < minElevation) newElevationAngle = minElevation;
    
    // 新しいカメラ位置を計算
    double distance = sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
    double newHorizontalLength = distance * cos(newElevationAngle);
    double newZ = distance * sin(newElevationAngle);
    
    // 水平方向の単位ベクトル
    double horizontalUnit[2] = {0.0, 0.0};
    if (horizontalLength > 1e-6) {
        horizontalUnit[0] = newDirection[0] / horizontalLength;
        horizontalUnit[1] = newDirection[1] / horizontalLength;
    }
    
    // 新しいカメラ位置を設定
    double newPosition[3];
    newPosition[0] = focalPoint[0] + horizontalUnit[0] * newHorizontalLength;
    newPosition[1] = focalPoint[1] + horizontalUnit[1] * newHorizontalLength;
    newPosition[2] = focalPoint[2] + newZ;
    
    camera->SetPosition(newPosition);
    
    // ViewUpベクトルを常にZ軸方向に設定
    camera->SetViewUp(0, 0, 1);
    
    if (this->AutoAdjustCameraClippingRange) {
        this->CurrentRenderer->ResetCameraClippingRange();
    }
    
    if (this->Interactor->GetLightFollowCamera()) {
        this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
    
    this->Interactor->Render();
}

void TurntableInteractorStyle::Pan()
{
    if (this->CurrentRenderer == nullptr) {
        return;
    }

    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
    if (camera == nullptr) {
        return;
    }

    // パンの処理を親クラスに委譲
    vtkInteractorStyleTrackballCamera::Pan();
    
    // パン後にZ軸を上に保つ
    camera->SetViewUp(0, 0, 1);
    
    if (this->AutoAdjustCameraClippingRange) {
        this->CurrentRenderer->ResetCameraClippingRange();
    }
    
    this->Interactor->Render();
}

void TurntableInteractorStyle::Dolly()
{
    if (this->CurrentRenderer == nullptr) {
        return;
    }

    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
    if (camera == nullptr) {
        return;
    }

    // ズームの処理を親クラスに委譲
    vtkInteractorStyleTrackballCamera::Dolly();
    
    // ズーム後にZ軸を上に保つ
    camera->SetViewUp(0, 0, 1);
    
    if (this->AutoAdjustCameraClippingRange) {
        this->CurrentRenderer->ResetCameraClippingRange();
    }
    
    this->Interactor->Render();
}

void TurntableInteractorStyle::ConstrainCameraPosition()
{
    if (this->CurrentRenderer == nullptr) {
        return;
    }

    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
    if (camera == nullptr) {
        return;
    }
    
    // ViewUpベクトルを常にZ軸方向に設定
    camera->SetViewUp(0, 0, 1);
}
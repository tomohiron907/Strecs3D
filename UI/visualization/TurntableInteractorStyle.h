#ifndef TURNTABLEINTERACTORSTYLE_H
#define TURNTABLEINTERACTORSTYLE_H

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkMath.h>
#include <vtkRenderWindowInteractor.h>

class TurntableInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static TurntableInteractorStyle* New();
    vtkTypeMacro(TurntableInteractorStyle, vtkInteractorStyleTrackballCamera);
    
    void SetRotationSpeed(double speed) { rotationSpeed_ = speed; }
    double GetRotationSpeed() const { return rotationSpeed_; }
    
protected:
    TurntableInteractorStyle() : rotationSpeed_(0.015) {}
    ~TurntableInteractorStyle() override = default;
    
    void Rotate() override;
    void Pan() override;
    void Dolly() override;
    
private:
    void ConstrainCameraPosition();
    double rotationSpeed_;
};

#endif // TURNTABLEINTERACTORSTYLE_H
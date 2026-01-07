#pragma once

#include "TurntableInteractorStyle.h"
#include <vtkSmartPointer.h>
#include <vtkPropPicker.h>
#include <vtkActor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vector>

#include <functional>

class StepFacePickerStyle : public TurntableInteractorStyle
{
public:
    static StepFacePickerStyle* New();
    vtkTypeMacro(StepFacePickerStyle, TurntableInteractorStyle);

    void OnMouseMove() override;
    void OnLeftButtonDown() override;

    // 面アクターのリストを設定
    void SetFaceActors(const std::vector<vtkSmartPointer<vtkActor>>& actors);

    // クリック時のコールバックを設定
    void SetOnFaceClicked(std::function<void(int, const double*)> callback) { onFaceClicked_ = callback; }

    // レンダラーを設定
    void SetRenderer(vtkRenderer* renderer);

    // ラベルアクターを取得（clearRenderer()での保護用）
    vtkTextActor* GetLabel() const { return label_; }

protected:
    StepFacePickerStyle();
    ~StepFacePickerStyle() override = default;

private:
    vtkSmartPointer<vtkPropPicker> picker_;
    vtkSmartPointer<vtkTextActor> label_;
    std::vector<vtkSmartPointer<vtkActor>> faceActors_;
    vtkActor* lastPickedActor_;
    vtkRenderer* renderer_;

    // 元の色を保存
    double originalColor_[3];
    bool hasOriginalColor_;

    void ResetLastPickedActor();
    void HighlightActor(vtkActor* actor);
    void UpdateLabel(int faceNumber, int x, int y);
    void HideLabel();

    std::function<void(int, const double*)> onFaceClicked_;
};

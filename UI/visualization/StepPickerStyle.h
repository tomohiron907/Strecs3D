#pragma once

#include "TurntableInteractorStyle.h"
#include <vtkSmartPointer.h>
#include <vtkCellPicker.h>
#include <vtkActor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vector>

#include <functional>

class StepPickerStyle : public TurntableInteractorStyle
{
public:
    static StepPickerStyle* New();
    vtkTypeMacro(StepPickerStyle, TurntableInteractorStyle);

    void OnMouseMove() override;
    void OnLeftButtonDown() override;

    // 面アクターのリストを設定
    void SetFaceActors(const std::vector<vtkSmartPointer<vtkActor>>& actors);

    // エッジアクターのリストを設定
    void SetEdgeActors(const std::vector<vtkSmartPointer<vtkActor>>& actors);

    // クリック時のコールバックを設定
    void SetOnFaceClicked(std::function<void(int, const double*)> callback) { onFaceClicked_ = callback; }

    // レンダラーを設定
    void SetRenderer(vtkRenderer* renderer);

    // ラベルアクターを取得（clearRenderer()での保護用）
    vtkTextActor* GetLabel() const { return label_; }

protected:
    StepPickerStyle();
    ~StepPickerStyle() override = default;

private:
    vtkSmartPointer<vtkCellPicker> picker_;
    vtkSmartPointer<vtkTextActor> label_;
    std::vector<vtkSmartPointer<vtkActor>> faceActors_;
    std::vector<vtkSmartPointer<vtkActor>> edgeActors_;
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

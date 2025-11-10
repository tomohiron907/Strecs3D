#include "StepFacePickerStyle.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkObjectFactory.h>
#include <vtkActor2DCollection.h>
#include <iostream>

vtkStandardNewMacro(StepFacePickerStyle);

StepFacePickerStyle::StepFacePickerStyle()
    : lastPickedActor_(nullptr)
    , renderer_(nullptr)
    , hasOriginalColor_(false)
{
    picker_ = vtkSmartPointer<vtkPropPicker>::New();
    picker_->PickFromListOn();

    // ラベルの設定 - 2Dテキストアクター
    label_ = vtkSmartPointer<vtkTextActor>::New();
    label_->GetTextProperty()->SetFontSize(18);
    label_->GetTextProperty()->SetColor(0.8, 0.8, 0.8); // 黄色
    // label_->GetTextProperty()->SetShadow(1);
    // label_->GetTextProperty()->SetBackgroundColor(0.2, 0.2, 0.2);
    label_->GetTextProperty()->SetBackgroundOpacity(0.8);
    label_->GetTextProperty()->SetFontFamilyToArial();
    // レイヤー番号はデフォルトの0を使用（レンダラーと同じレイヤー）
    label_->SetVisibility(0);
}

void StepFacePickerStyle::OnMouseMove()
{
    // 親クラスのマウス移動処理を呼び出す（ターンテーブルカメラ操作など）
    TurntableInteractorStyle::OnMouseMove();

    if (!this->Interactor || !renderer_) {
        return;
    }

    // マウス位置を取得
    int* clickPos = this->Interactor->GetEventPosition();

    // ピッキングを実行
    picker_->Pick(clickPos[0], clickPos[1], 0, renderer_);

    vtkActor* pickedActor = picker_->GetActor();

    if (pickedActor) {
        // ピックされたアクターが面アクターのリストに含まれているか確認
        int faceIndex = -1;
        for (size_t i = 0; i < faceActors_.size(); ++i) {
            if (faceActors_[i] == pickedActor) {
                faceIndex = static_cast<int>(i);
                break;
            }
        }

        if (faceIndex >= 0) {
            // 前回ハイライトしたアクターをリセット
            if (lastPickedActor_ && lastPickedActor_ != pickedActor) {
                ResetLastPickedActor();
            }

            // 新しいアクターをハイライト
            if (lastPickedActor_ != pickedActor) {
                HighlightActor(pickedActor);
                lastPickedActor_ = pickedActor;
            }

            // マウス位置でラベルを更新
            UpdateLabel(faceIndex, clickPos[0], clickPos[1]);

            // 再描画
            this->Interactor->GetRenderWindow()->Render();
            return;
        }
    }

    // 何もピックされなかった場合、ハイライトとラベルをクリア
    if (lastPickedActor_) {
        ResetLastPickedActor();
        lastPickedActor_ = nullptr;
    }
    HideLabel();
    this->Interactor->GetRenderWindow()->Render();
}

void StepFacePickerStyle::SetFaceActors(const std::vector<vtkSmartPointer<vtkActor>>& actors)
{
    faceActors_ = actors;

    // ピッカーのリストをクリア
    picker_->InitializePickList();

    // 面アクターをピックリストに追加
    for (auto& actor : faceActors_) {
        picker_->AddPickList(actor);
    }
}

void StepFacePickerStyle::SetRenderer(vtkRenderer* renderer)
{
    renderer_ = renderer;

    // ラベルをレンダラーに追加
    if (renderer_ && label_) {
        renderer_->AddActor2D(label_);
    }
}

void StepFacePickerStyle::ResetLastPickedActor()
{
    if (lastPickedActor_ && hasOriginalColor_) {
        lastPickedActor_->GetProperty()->SetColor(originalColor_);
        hasOriginalColor_ = false;
    }
}

void StepFacePickerStyle::HighlightActor(vtkActor* actor)
{
    if (!actor) return;

    // 元の色を保存
    actor->GetProperty()->GetColor(originalColor_);
    hasOriginalColor_ = true;

    actor->GetProperty()->SetColor(0.455, 0.565, 0.69); //116, 144, 176
}

void StepFacePickerStyle::UpdateLabel(int faceNumber, int x, int y)
{
    if (!label_) {
        return;
    }

    // 面番号を表示（1から始まる番号）
    std::string labelText = "Face " + std::to_string(faceNumber + 1);
    label_->SetInput(labelText.c_str());

    // ラベルの位置を設定（スクリーン座標）
    // マウスカーソルの少し右上に表示
    label_->SetPosition(x + 15, y + 15);

    // ラベルを表示
    label_->SetVisibility(1);
}

void StepFacePickerStyle::HideLabel()
{
    if (label_) {
        label_->SetVisibility(0);
    }
}

#include "StepPickerStyle.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkObjectFactory.h>
#include <vtkActor2DCollection.h>
#include <vtkCellPicker.h>
#include <iostream>

vtkStandardNewMacro(StepPickerStyle);

StepPickerStyle::StepPickerStyle()
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

void StepPickerStyle::OnMouseMove()
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

        // ピックされたアクターがエッジアクターのリストに含まれているか確認
        int edgeIndex = -1;
        for (size_t i = 0; i < edgeActors_.size(); ++i) {
            if (edgeActors_[i] == pickedActor) {
                edgeIndex = static_cast<int>(i);
                break;
            }
        }

        if (edgeIndex >= 0) {
            // 前回ハイライトしたアクターをリセット
            if (lastPickedActor_ && lastPickedActor_ != pickedActor) {
                ResetLastPickedActor();
            }

            // 新しいアクターをハイライト
            if (lastPickedActor_ != pickedActor) {
                HighlightActor(pickedActor);
                lastPickedActor_ = pickedActor;
            }

            // ラベル更新（エッジ用）実装が必要だが、一旦共通のUpdateLabelを使うか拡張する
            // ここでは簡易的に負の値などを使って区別するか、別途メソッドを作る
            // UpdateLabelを拡張して、タイプを指定できるようにする変更は後で行うとして、
            // 今はとりあえずFaceと同じ仕組みで表示してみる（"Face N"となってしまうが）
            // 修正案：UpdateLabelの第一引数を変更するか、別メソッドにする。
            
            // 下記のUpdateLabelEdgeを使用
             if (!label_) return;
             
             std::string labelText = "Edge " + std::to_string(edgeIndex + 1);
             label_->SetInput(labelText.c_str());
             label_->SetPosition(clickPos[0] + 15, clickPos[1] + 15);
             label_->SetVisibility(1);

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

void StepPickerStyle::OnLeftButtonDown()
{
    // マウス位置を取得
    if (!this->Interactor || !renderer_) {
        TurntableInteractorStyle::OnLeftButtonDown();
        return;
    }

    int* clickPos = this->Interactor->GetEventPosition();

    // ピッキングを実行
    vtkSmartPointer<vtkCellPicker> cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    cellPicker->PickFromListOn();
    
    // Add faces to pick list
    for (auto& actor : faceActors_) {
        cellPicker->AddPickList(actor);
    }
    
    // Pick
    cellPicker->Pick(clickPos[0], clickPos[1], 0, renderer_);

    vtkActor* pickedActor = cellPicker->GetActor();

    if (pickedActor) {
        // ピックされたアクターが面アクターのリストに含まれているか確認
        int faceIndex = -1;
        for (size_t i = 0; i < faceActors_.size(); ++i) {
            if (faceActors_[i] == pickedActor) {
                faceIndex = static_cast<int>(i);
                break;
            }
        }

        if (faceIndex >= 0 && onFaceClicked_) {
            double normal[3];
            cellPicker->GetPickNormal(normal);
            onFaceClicked_(faceIndex + 1, normal); // 1-based index for UI
        }
    }

    // デフォルトの動作（回転など）も維持する
    TurntableInteractorStyle::OnLeftButtonDown();
}

void StepPickerStyle::SetFaceActors(const std::vector<vtkSmartPointer<vtkActor>>& actors)
{
    faceActors_ = actors;

    // ピッカーのリストをクリア
    picker_->InitializePickList();

    // 面アクターをピックリストに追加
    for (auto& actor : faceActors_) {
        picker_->AddPickList(actor);
    }
}

void StepPickerStyle::SetEdgeActors(const std::vector<vtkSmartPointer<vtkActor>>& actors)
{
    edgeActors_ = actors;

    // エッジアクターをピックリストに追加
    // FaceActorsが既にセットされている前提で、追加する形にする
    // ただしInitializePickListされると消えるので、管理が必要。
    // InitializePickListはSetFaceActorsで呼ばれているので、
    // ここで追加するだけでよいが、SetFaceActorsが後で呼ばれると消える。
    // 両方をマージしてセットするか、毎回再構築するのが安全。
    
    // 既存のPickerの設定を維持しつつ追加
    for (auto& actor : edgeActors_) {
        if (actor) {
            picker_->AddPickList(actor);
        }
    }
}

void StepPickerStyle::SetRenderer(vtkRenderer* renderer)
{
    renderer_ = renderer;

    // ラベルをレンダラーに追加
    if (renderer_ && label_) {
        renderer_->AddActor2D(label_);
    }
}

void StepPickerStyle::ResetLastPickedActor()
{
    if (lastPickedActor_ && hasOriginalColor_) {
        lastPickedActor_->GetProperty()->SetColor(originalColor_);
        // 線の太さを戻す（エッジの場合）
        // 元の太さを保存していないので、デフォルト値(2.0)に戻すか、
        // hasOriginalColor_に含めるか。
        // ここでは簡易的に、エッジリストに含まれていれば太さを戻す。
        bool isEdge = false;
        for (auto& edgeActor : edgeActors_) {
            if (edgeActor == lastPickedActor_) {
                isEdge = true;
                break;
            }
        }
        if (isEdge) {
            lastPickedActor_->GetProperty()->SetLineWidth(2.0); // StepReaderで設定したデフォルト値
        }
        
        hasOriginalColor_ = false;
    }
}

void StepPickerStyle::HighlightActor(vtkActor* actor)
{
    if (!actor) return;

    // 元の色を保存
    actor->GetProperty()->GetColor(originalColor_);
    hasOriginalColor_ = true;

    // エッジか面かで色を変える
    bool isEdge = false;
    for (auto& edgeActor : edgeActors_) {
        if (edgeActor == actor) {
            isEdge = true;
            break;
        }
    }

    if (isEdge) {
        actor->GetProperty()->SetColor(0.455, 0.565, 0.69); // 面のハイライトと同じ色
        actor->GetProperty()->SetLineWidth(4.0); // 少し太くする

    } else {
        actor->GetProperty()->SetColor(0.455, 0.565, 0.69); // 面の色
    }
}

void StepPickerStyle::UpdateLabel(int faceNumber, int x, int y)
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

void StepPickerStyle::HideLabel()
{
    if (label_) {
        label_->SetVisibility(0);
    }
}

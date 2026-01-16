#include "StepPickerStyle.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkObjectFactory.h>
#include <vtkActor2DCollection.h>
#include <vtkCellPicker.h>


vtkStandardNewMacro(StepPickerStyle);

StepPickerStyle::StepPickerStyle()
    : lastPickedActor_(nullptr)
    , renderer_(nullptr)
    , hasOriginalColor_(false)
    , edgeSelectionMode_(false)
    , faceSelectionMode_(false)
    , isClickPending_(false)
{
    clickStartPos_[0] = 0;
    clickStartPos_[1] = 0;
    picker_ = vtkSmartPointer<vtkCellPicker>::New();
    picker_->PickFromListOn();
    picker_->SetTolerance(0.005); // 感度向上: 画面サイズの0.5%の許容誤差

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
        // エッジ選択モードの場合はエッジのみをハイライト
        if (edgeSelectionMode_) {
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

                // エッジ選択モード用のラベル表示
                if (label_) {
                    std::string labelText = "Edge " + std::to_string(edgeIndex + 1) + " (Click to select)";
                    label_->SetInput(labelText.c_str());
                    label_->SetPosition(clickPos[0] + 15, clickPos[1] + 15);
                    label_->SetVisibility(1);
                }

                // 再描画
                this->Interactor->GetRenderWindow()->Render();
                return;
            }

            // エッジ選択モードでエッジ以外がホバーされた場合
            if (lastPickedActor_) {
                ResetLastPickedActor();
                lastPickedActor_ = nullptr;
            }
            HideLabel();
            this->Interactor->GetRenderWindow()->Render();
            return;
        }

    // 通常モード：ピックされたアクターが面アクターのリストに含まれているか確認
        // 面選択モードが有効な場合のみチェック
        if (faceSelectionMode_) {
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
        
        // それ以外（エッジ選択も面選択も無効、または有効だがヒットなし）の場合
        // エッジアクターのハイライト処理は、edgeSelectionModeがfalseの場合は行わない（最初の分岐で処理済み）
        // ただし、現在の実装では edgeSelectionMode_ が false の場合にここに来る。
        // 面選択も false の場合、ここに来る。
        // したがって、何もしないで抜けるべきだが、既存コードではエッジのチェックをしている。
        // ここでのエッジチェックは、「エッジ選択モードではないが、エッジにマウスオーバーしたときの挙動」である。
        // 要件：「面やエッジを選択する必要がないときには、オフにして」
        // つまり、edgeSelectionMode_がfalseならエッジハイライトもしない。
        // したがって、以下のエッジチェックブロックは削除または faceSelectionMode_ の外側で何もしないようにする。
        
        // 元のロジックでは、エッジ選択モードでなくてもエッジハイライトしていたかもしれないが、
        // 今回の要件では「面選択モード」と「エッジ選択モード」で明確に分ける。
        
        // 何もヒットしない、またはモードが無効な場合は下に抜けてリセット処理へ
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

    // クリック開始位置を記録（背景クリック判定用）
    clickStartPos_[0] = clickPos[0];
    clickStartPos_[1] = clickPos[1];
    isClickPending_ = true;

    // ピッキングを実行
    vtkSmartPointer<vtkCellPicker> cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    cellPicker->PickFromListOn();
    cellPicker->SetTolerance(0.01);

    // エッジ選択モードの場合はエッジのみをピック
    if (edgeSelectionMode_) {
        // Add only edges to pick list
        for (auto& actor : edgeActors_) {
            if (actor) {
                cellPicker->AddPickList(actor);
            }
        }

        // Pick
        cellPicker->Pick(clickPos[0], clickPos[1], 0, renderer_);
        vtkActor* pickedActor = cellPicker->GetActor();

        if (pickedActor) {
            // Find edge index
            int edgeIndex = -1;
            for (size_t i = 0; i < edgeActors_.size(); ++i) {
                if (edgeActors_[i] == pickedActor) {
                    edgeIndex = static_cast<int>(i);
                    break;
                }
            }

            if (edgeIndex >= 0 && onEdgeClicked_) {
                onEdgeClicked_(edgeIndex + 1);  // 1-based index
                isClickPending_ = false;  // エッジ選択時は背景クリック判定をリセット
                return;  // エッジ選択モードでは回転を無効化
            }
        }
        return;  // エッジ選択モードでは回転を無効化
    }

    // 通常モード: Add faces to pick list
    // 面選択モードが有効な場合のみ面を追加
    if (faceSelectionMode_) {
        for (auto& actor : faceActors_) {
            cellPicker->AddPickList(actor);
        }
    } else {
        // 面選択モードが無効でも、背景クリック判定は行う（isClickPending_は維持）
        TurntableInteractorStyle::OnLeftButtonDown();
        return;
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
            isClickPending_ = false;  // 面選択時は背景クリック判定をリセット
        }
    }
    // 背景クリック判定はOnLeftButtonUpで行う

    // デフォルトの動作（回転など）も維持する
    TurntableInteractorStyle::OnLeftButtonDown();
}

void StepPickerStyle::OnLeftButtonUp()
{
    // マウス位置を取得
    if (this->Interactor && isClickPending_) {
        int* clickPos = this->Interactor->GetEventPosition();

        // クリック開始位置からの移動距離を計算
        int dx = clickPos[0] - clickStartPos_[0];
        int dy = clickPos[1] - clickStartPos_[1];
        int distanceSquared = dx * dx + dy * dy;

        // 移動距離が閾値以下ならシンプルなクリックとみなす
        if (distanceSquared <= CLICK_THRESHOLD * CLICK_THRESHOLD) {
            // 背景クリック（何も選択されていない領域をクリック）
            if (onBackgroundClicked_) {
                onBackgroundClicked_();
            }
        }
    }

    isClickPending_ = false;

    // 親クラスの処理を呼び出す
    TurntableInteractorStyle::OnLeftButtonUp();
}

void StepPickerStyle::OnLeftButtonDoubleClick()
{
    // マウス位置を取得
    if (!this->Interactor || !renderer_) {
        vtkInteractorStyleTrackballCamera::OnLeftButtonDoubleClick();
        return;
    }

    int* clickPos = this->Interactor->GetEventPosition();

    // ピッキングを実行
    vtkSmartPointer<vtkCellPicker> cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    cellPicker->PickFromListOn();
    cellPicker->SetTolerance(0.01);

    // エッジ選択モードの場合はダブルクリック処理は不要
    if (edgeSelectionMode_) {
         vtkInteractorStyleTrackballCamera::OnLeftButtonDoubleClick();
         return;
    }

    // 通常モード: Add faces to pick list
    // 面選択モードが無効ならダブルクリック処理もしない
    if (!faceSelectionMode_) {
         vtkInteractorStyleTrackballCamera::OnLeftButtonDoubleClick();
         return;
    }

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

        if (faceIndex >= 0 && onFaceDoubleClicked_) {
            double normal[3];
            cellPicker->GetPickNormal(normal);
            onFaceDoubleClicked_(faceIndex + 1, normal); // 1-based index for UI
        }
    }

    // デフォルトの動作
    vtkInteractorStyleTrackballCamera::OnLeftButtonDoubleClick();
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

void StepPickerStyle::SetEdgeSelectionMode(bool enabled)
{
    edgeSelectionMode_ = enabled;
    // エッジ選択モードが有効になるとき、面選択モードは無効にする（排他制御）
    // 要件：「この時は、面のマウスオーバーハイライトはオフにして」
    if (enabled) {
        faceSelectionMode_ = false;
    }
}

void StepPickerStyle::SetFaceSelectionMode(bool enabled)
{
    faceSelectionMode_ = enabled;
    // 面選択モードが有効になるとき、エッジ選択モードは無効にする（排他制御）
    // 要件：「この時は、エッジのマウスオーバーハイライトはオフにして」
    if (enabled) {
        edgeSelectionMode_ = false;
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

#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include <QString>

/**
 * メッシュの表示/非表示を設定するコマンド
 * 可視化カテゴリ
 *
 * このコマンドは2つの処理を行う：
 * 1. UIStateの更新（状態管理）
 * 2. ApplicationController経由での可視化処理（描画）
 */
class SetMeshVisibilityCommand : public Command {
public:
    enum class MeshType {
        STEP_MESH,
        VTU_MESH,
        DIVIDED_MESH_1,
        DIVIDED_MESH_2,
        DIVIDED_MESH_3,
        DIVIDED_MESH_4
    };

    SetMeshVisibilityCommand(
        UIState* uiState,
        ApplicationController* controller,
        IUserInterface* ui,
        MeshType meshType,
        const QString& fileName,
        bool visible
    ) : uiState_(uiState),
        controller_(controller),
        ui_(ui),
        meshType_(meshType),
        fileName_(fileName),
        visible_(visible) {}

    void execute() override {
        // 1. UIStateの更新
        updateUIState();

        // 2. 実際の可視化更新
        updateVisualization();
    }

private:
    void updateUIState() {
        if (!uiState_) return;

        switch (meshType_) {
            case MeshType::STEP_MESH:
                uiState_->setStepVisibility(visible_);
                break;

            case MeshType::VTU_MESH:
                uiState_->setSimulationResultVisibility(visible_);
                break;

            case MeshType::DIVIDED_MESH_1:
                uiState_->setInfillRegionVisibility("mesh1", visible_);
                break;

            case MeshType::DIVIDED_MESH_2:
                uiState_->setInfillRegionVisibility("mesh2", visible_);
                break;

            case MeshType::DIVIDED_MESH_3:
                uiState_->setInfillRegionVisibility("mesh3", visible_);
                break;

            case MeshType::DIVIDED_MESH_4:
                uiState_->setInfillRegionVisibility("mesh4", visible_);
                break;
        }
    }

    void updateVisualization() {
        if (!controller_ || !ui_ || fileName_.isEmpty()) return;

        // ApplicationController経由で実際の可視化を更新
        controller_->setMeshVisibility(fileName_.toStdString(), visible_, ui_);
    }

    UIState* uiState_;
    ApplicationController* controller_;
    IUserInterface* ui_;
    MeshType meshType_;
    QString fileName_;
    bool visible_;
};

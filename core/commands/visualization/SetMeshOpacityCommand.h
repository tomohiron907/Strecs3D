#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include <QString>

/**
 * メッシュの不透明度を設定するコマンド
 * 可視化カテゴリ
 *
 * このコマンドは2つの処理を行う：
 * 1. UIStateの更新（状態管理）
 * 2. ApplicationController経由での可視化処理（描画）
 */
class SetMeshOpacityCommand : public Command {
public:
    enum class MeshType {
        STEP_MESH,
        VTU_MESH,
        DIVIDED_MESH_1,
        DIVIDED_MESH_2,
        DIVIDED_MESH_3,
        DIVIDED_MESH_4
    };

    SetMeshOpacityCommand(
        UIState* uiState,
        ApplicationController* controller,
        IUserInterface* ui,
        MeshType meshType,
        const QString& fileName,
        double opacity
    ) : uiState_(uiState),
        controller_(controller),
        ui_(ui),
        meshType_(meshType),
        fileName_(fileName),
        opacity_(opacity) {}

    void execute() override {
        // 1. UIStateの更新
        updateUIState();

        // 2. 実際の可視化更新
        updateVisualization();
    }

private:
    void updateUIState() {
        if (!uiState_) return;

        DisplaySettings settings;

        switch (meshType_) {
            case MeshType::STEP_MESH:
                settings = uiState_->getStepDisplaySettings();
                settings.opacity = opacity_;
                uiState_->setStepDisplaySettings(settings);
                break;

            case MeshType::VTU_MESH:
                settings = uiState_->getVtuDisplaySettings();
                settings.opacity = opacity_;
                uiState_->setVtuDisplaySettings(settings);
                break;

            case MeshType::DIVIDED_MESH_1:
                settings = uiState_->getDividedMesh1Settings();
                settings.opacity = opacity_;
                uiState_->setDividedMesh1Settings(settings);
                break;

            case MeshType::DIVIDED_MESH_2:
                settings = uiState_->getDividedMesh2Settings();
                settings.opacity = opacity_;
                uiState_->setDividedMesh2Settings(settings);
                break;

            case MeshType::DIVIDED_MESH_3:
                settings = uiState_->getDividedMesh3Settings();
                settings.opacity = opacity_;
                uiState_->setDividedMesh3Settings(settings);
                break;

            case MeshType::DIVIDED_MESH_4:
                settings = uiState_->getDividedMesh4Settings();
                settings.opacity = opacity_;
                uiState_->setDividedMesh4Settings(settings);
                break;
        }
    }

    void updateVisualization() {
        if (!controller_ || !ui_ || fileName_.isEmpty()) return;

        // ApplicationController経由で実際の可視化を更新
        controller_->setMeshOpacity(fileName_.toStdString(), opacity_, ui_);
    }

    UIState* uiState_;
    ApplicationController* controller_;
    IUserInterface* ui_;
    MeshType meshType_;
    QString fileName_;
    double opacity_;
};

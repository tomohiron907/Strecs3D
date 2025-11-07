#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * メッシュの表示/非表示を設定するコマンド
 * UIStateを直接操作する
 */
class SetMeshVisibilityCommand : public Command {
public:
    enum class MeshType {
        STL_MESH,
        VTU_MESH,
        DIVIDED_MESH_1,
        DIVIDED_MESH_2,
        DIVIDED_MESH_3,
        DIVIDED_MESH_4
    };

    SetMeshVisibilityCommand(
        UIState* uiState,
        MeshType meshType,
        bool visible
    ) : uiState_(uiState),
        meshType_(meshType),
        visible_(visible) {}

    void execute() override {
        if (!uiState_) return;

        DisplaySettings settings;

        switch (meshType_) {
            case MeshType::STL_MESH:
                settings = uiState_->getMeshDisplaySettings();
                settings.isVisible = visible_;
                uiState_->setMeshDisplaySettings(settings);
                break;

            case MeshType::VTU_MESH:
                settings = uiState_->getVtuDisplaySettings();
                settings.isVisible = visible_;
                uiState_->setVtuDisplaySettings(settings);
                break;

            case MeshType::DIVIDED_MESH_1:
                settings = uiState_->getDividedMesh1Settings();
                settings.isVisible = visible_;
                uiState_->setDividedMesh1Settings(settings);
                break;

            case MeshType::DIVIDED_MESH_2:
                settings = uiState_->getDividedMesh2Settings();
                settings.isVisible = visible_;
                uiState_->setDividedMesh2Settings(settings);
                break;

            case MeshType::DIVIDED_MESH_3:
                settings = uiState_->getDividedMesh3Settings();
                settings.isVisible = visible_;
                uiState_->setDividedMesh3Settings(settings);
                break;

            case MeshType::DIVIDED_MESH_4:
                settings = uiState_->getDividedMesh4Settings();
                settings.isVisible = visible_;
                uiState_->setDividedMesh4Settings(settings);
                break;
        }
    }

private:
    UIState* uiState_;
    MeshType meshType_;
    bool visible_;
};

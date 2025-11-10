#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include "../../ui/UIState.h"
#include <QString>

/**
 * FEM設定ファイル（JSON）をエクスポートするコマンド
 * 処理カテゴリ
 */
class ExportFEMConfigCommand : public Command {
public:
    ExportFEMConfigCommand(
        ApplicationController* controller,
        IUserInterface* ui,
        UIState* uiState,
        const QString& outputPath
    ) : controller_(controller),
        ui_(ui),
        uiState_(uiState),
        outputPath_(outputPath) {}

    void execute() override {
        if (!controller_ || !ui_ || !uiState_) {
            return;
        }

        // FEM設定ファイルのエクスポートを実行
        controller_->exportSimulationCondition(ui_, uiState_, outputPath_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    UIState* uiState_;
    QString outputPath_;
};

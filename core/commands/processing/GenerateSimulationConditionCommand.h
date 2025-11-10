#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include "../../ui/UIState.h"
#include <QString>

/**
 * シミュレーション条件をJSONファイルにエクスポートするコマンド
 * 処理カテゴリ
 */
class GenerateSimulationConditionCommand : public Command {
public:
    GenerateSimulationConditionCommand(
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

        // シミュレーション条件のエクスポートを実行
        controller_->exportSimulationCondition(ui_, uiState_, outputPath_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    UIState* uiState_;
    QString outputPath_;
};

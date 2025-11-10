#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include "../../ui/UIState.h"
#include <QString>

/**
 * FEM解析パイプライン全体を実行するコマンド
 * 処理カテゴリ
 *
 * このコマンドは以下の2つのステップを順次実行します：
 * 1. FEM設定ファイル（JSON）をエクスポート (ExportFEMConfigCommand相当)
 * 2. エクスポートした設定ファイルを使用してFEM解析を実行 (RunFEMAnalysisCommand相当)
 */
class RunFEMPipelineCommand : public Command {
public:
    RunFEMPipelineCommand(
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

        // Step 1: FEM設定ファイルをJSONにエクスポート
        bool exportSuccess = controller_->exportSimulationCondition(ui_, uiState_, outputPath_);

        if (!exportSuccess) {
            // エクスポートが失敗した場合、FEM解析は実行しない
            ui_->showCriticalMessage("エラー", "FEM設定ファイルのエクスポートに失敗したため、解析を実行できません");
            return;
        }

        // Step 2: エクスポートした設定ファイルを使用してFEM解析を実行
        controller_->runSimulation(ui_, outputPath_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    UIState* uiState_;
    QString outputPath_;
};

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
        if (controller_) {
            controller_->runFEMPipeline(ui_, uiState_, outputPath_);
        }
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    UIState* uiState_;
    QString outputPath_;
};

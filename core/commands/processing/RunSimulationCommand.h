#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include <QString>

/**
 * FEMシミュレーションを実行するコマンド
 * 処理カテゴリ
 */
class RunSimulationCommand : public Command {
public:
    RunSimulationCommand(
        ApplicationController* controller,
        IUserInterface* ui,
        const QString& configFilePath
    ) : controller_(controller),
        ui_(ui),
        configFilePath_(configFilePath) {}

    void execute() override {
        if (!controller_ || !ui_) {
            return;
        }

        // FEMシミュレーションの実行
        controller_->runSimulation(ui_, configFilePath_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    QString configFilePath_;
};

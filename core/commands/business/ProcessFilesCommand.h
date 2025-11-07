#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"

/**
 * ファイル処理を実行するコマンド
 * ApplicationControllerのビジネスロジックを呼び出す
 */
class ProcessFilesCommand : public Command {
public:
    ProcessFilesCommand(
        ApplicationController* controller,
        IUserInterface* ui
    ) : controller_(controller), ui_(ui) {}

    void execute() override {
        if (!controller_ || !ui_) {
            return;
        }

        // ビジネスロジックを実行
        controller_->processFiles(ui_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
};

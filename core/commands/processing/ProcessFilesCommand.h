#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"

/**
 * ファイル処理（メッシュ分割）を実行するコマンド
 * 処理カテゴリ
 */
class ProcessFilesCommand : public Command {
public:
    ProcessFilesCommand(
        ApplicationController* controller,
        IUserInterface* ui
    ) : controller_(controller),
        ui_(ui) {}

    void execute() override {
        if (!controller_ || !ui_) {
            return;
        }

        // ファイル処理（メッシュ分割）を実行
        controller_->processFiles(ui_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
};

#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"

/**
 * 3MFファイルをエクスポートするコマンド
 * ApplicationControllerのビジネスロジックを呼び出す
 */
class Export3mfCommand : public Command {
public:
    Export3mfCommand(
        ApplicationController* controller,
        IUserInterface* ui
    ) : controller_(controller), ui_(ui) {}

    void execute() override {
        if (!controller_ || !ui_) {
            return;
        }

        // ビジネスロジックを実行
        controller_->export3mfFile(ui_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
};

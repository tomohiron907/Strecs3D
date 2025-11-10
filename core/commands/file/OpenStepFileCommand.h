#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include <QString>
#include <string>

/**
 * STEPファイルを開くコマンド
 * ファイル操作カテゴリ
 */
class OpenStepFileCommand : public Command {
public:
    OpenStepFileCommand(
        ApplicationController* controller,
        IUserInterface* ui,
        const QString& filePath
    ) : controller_(controller),
        ui_(ui),
        filePath_(filePath) {}

    void execute() override {
        if (!controller_ || !ui_ || filePath_.isEmpty()) {
            return;
        }

        // ビジネスロジックを実行
        std::string file = filePath_.toStdString();
        controller_->openStepFile(file, ui_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    QString filePath_;
};

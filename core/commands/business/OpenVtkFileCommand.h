#pragma once

#include "../Command.h"
#include "../../application/ApplicationController.h"
#include "../../interfaces/IUserInterface.h"
#include <QString>
#include <string>
#include <QObject>

/**
 * VTKファイルを開くコマンド
 * ApplicationControllerのビジネスロジックを呼び出し、
 * 応力範囲の更新も処理する
 */
class OpenVtkFileCommand : public Command {
public:
    OpenVtkFileCommand(
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

        // ApplicationControllerのstressRangeChangedシグナルを一時的に接続
        QObject::connect(controller_, &ApplicationController::stressRangeChanged,
                        controller_, [this](double minStress, double maxStress) {
            ui_->initializeStressConfiguration(minStress, maxStress);
        }, Qt::SingleShotConnection);

        // ビジネスロジックを実行（stressRangeChangedシグナルが発行される）
        std::string file = filePath_.toStdString();
        controller_->openVtkFile(file, ui_);
    }

private:
    ApplicationController* controller_;
    IUserInterface* ui_;
    QString filePath_;
};

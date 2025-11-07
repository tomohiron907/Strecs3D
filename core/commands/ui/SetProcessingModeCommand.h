#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * 処理モードを設定するコマンド
 * UIStateを直接操作する
 */
class SetProcessingModeCommand : public Command {
public:
    SetProcessingModeCommand(
        UIState* uiState,
        ProcessingMode mode
    ) : uiState_(uiState), mode_(mode) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateを直接操作
        // 内部でシグナルが自動発火し、UIが更新される
        uiState_->setProcessingMode(mode_);
    }

private:
    UIState* uiState_;
    ProcessingMode mode_;
};

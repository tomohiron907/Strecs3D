#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * 応力範囲を設定するコマンド
 * UIStateを直接操作する
 */
class SetStressRangeCommand : public Command {
public:
    SetStressRangeCommand(
        UIState* uiState,
        double minStress,
        double maxStress
    ) : uiState_(uiState),
        minStress_(minStress),
        maxStress_(maxStress) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateを直接操作
        // 内部でシグナルが自動発火し、UIが更新される
        uiState_->setStressRange(minStress_, maxStress_);
    }

private:
    UIState* uiState_;
    double minStress_;
    double maxStress_;
};

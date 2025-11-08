#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * 処理モード（Bambu/Cura/Prusa）を設定するコマンド
 * 可視化カテゴリ
 *
 * このコマンドはUIStateの更新のみを行う（描画は不要）
 */
class SetProcessingModeCommand : public Command {
public:
    SetProcessingModeCommand(
        UIState* uiState,
        ProcessingMode mode
    ) : uiState_(uiState),
        mode_(mode) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateの処理モードを更新
        uiState_->setProcessingMode(mode_);
    }

private:
    UIState* uiState_;
    ProcessingMode mode_;
};

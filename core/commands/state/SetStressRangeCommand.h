#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * ストレス範囲を設定するコマンド
 * 可視化カテゴリ
 *
 * このコマンドはUIStateの更新のみを行う（描画は不要）
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

        // UIStateのストレス範囲を更新
        uiState_->setStressRange(minStress_, maxStress_);
    }

private:
    UIState* uiState_;
    double minStress_;
    double maxStress_;
};

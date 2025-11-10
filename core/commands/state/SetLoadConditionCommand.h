#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../types/BoundaryCondition.h"

/**
 * 荷重条件を設定するコマンド
 * 状態管理カテゴリ
 *
 * このコマンドはUIStateの更新のみを行う（描画は不要）
 */
class SetLoadConditionCommand : public Command {
public:
    SetLoadConditionCommand(
        UIState* uiState,
        const LoadCondition& load
    ) : uiState_(uiState),
        load_(load) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateに荷重条件を追加
        uiState_->addLoadCondition(load_);
    }

private:
    UIState* uiState_;
    LoadCondition load_;
};

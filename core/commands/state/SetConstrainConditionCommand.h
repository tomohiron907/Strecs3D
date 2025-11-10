#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../types/BoundaryCondition.h"

/**
 * 拘束条件を設定するコマンド
 * 状態管理カテゴリ
 *
 * このコマンドはUIStateの更新のみを行う（描画は不要）
 */
class SetConstrainConditionCommand : public Command {
public:
    SetConstrainConditionCommand(
        UIState* uiState,
        const ConstrainCondition& constrain
    ) : uiState_(uiState),
        constrain_(constrain) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateに拘束条件を追加
        uiState_->addConstrainCondition(constrain_);
    }

private:
    UIState* uiState_;
    ConstrainCondition constrain_;
};

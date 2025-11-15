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
class SetConstraintConditionCommand : public Command {
public:
    SetConstraintConditionCommand(
        UIState* uiState,
        const ConstraintCondition& constraint
    ) : uiState_(uiState),
        constraint_(constraint) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateに拘束条件を追加
        uiState_->addConstraintCondition(constraint_);
    }

private:
    UIState* uiState_;
    ConstraintCondition constraint_;
};

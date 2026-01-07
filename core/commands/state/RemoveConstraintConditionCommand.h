#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * 拘束条件を削除するコマンド
 */
class RemoveConstraintConditionCommand : public Command {
public:
    RemoveConstraintConditionCommand(
        UIState* uiState,
        int index
    ) : uiState_(uiState),
        index_(index) {}

    void execute() override {
        if (!uiState_) return;
        uiState_->removeConstraintCondition(index_);
    }

private:
    UIState* uiState_;
    int index_;
};

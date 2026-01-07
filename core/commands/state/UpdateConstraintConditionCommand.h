#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../types/BoundaryCondition.h"

/**
 * 拘束条件を更新するコマンド
 */
class UpdateConstraintConditionCommand : public Command {
public:
    UpdateConstraintConditionCommand(
        UIState* uiState,
        int index,
        const ConstraintCondition& constraint
    ) : uiState_(uiState),
        index_(index),
        constraint_(constraint) {}

    void execute() override {
        if (!uiState_) return;
        uiState_->updateConstraintCondition(index_, constraint_);
    }

private:
    UIState* uiState_;
    int index_;
    ConstraintCondition constraint_;
};

#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../types/BoundaryCondition.h"

/**
 * 荷重条件を更新するコマンド
 */
class UpdateLoadConditionCommand : public Command {
public:
    UpdateLoadConditionCommand(
        UIState* uiState,
        int index,
        const LoadCondition& load
    ) : uiState_(uiState),
        index_(index),
        load_(load) {}

    void execute() override {
        if (!uiState_) return;
        uiState_->updateLoadCondition(index_, load_);
    }

private:
    UIState* uiState_;
    int index_;
    LoadCondition load_;
};

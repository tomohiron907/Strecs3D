#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"

/**
 * 荷重条件を削除するコマンド
 */
class RemoveLoadConditionCommand : public Command {
public:
    RemoveLoadConditionCommand(
        UIState* uiState,
        int index
    ) : uiState_(uiState),
        index_(index) {}

    void execute() override {
        if (!uiState_) return;
        uiState_->removeLoadCondition(index_);
    }

private:
    UIState* uiState_;
    int index_;
};

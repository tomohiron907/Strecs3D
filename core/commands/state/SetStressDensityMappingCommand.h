#pragma once

#include "../Command.h"
#include "../../ui/UIState.h"
#include "../../types/StressDensityMapping.h"
#include <vector>

/**
 * ストレス-密度マッピングを設定するコマンド
 * 状態管理カテゴリ
 *
 * このコマンドはUIStateの更新のみを行う（描画は不要）
 */
class SetStressDensityMappingCommand : public Command {
public:
    SetStressDensityMappingCommand(
        UIState* uiState,
        const std::vector<StressDensityMapping>& mappings
    ) : uiState_(uiState),
        mappings_(mappings) {}

    void execute() override {
        if (!uiState_) return;

        // UIStateのストレス-密度マッピングを更新
        uiState_->setStressDensityMappings(mappings_);
    }

private:
    UIState* uiState_;
    std::vector<StressDensityMapping> mappings_;
};

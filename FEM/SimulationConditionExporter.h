#pragma once

#include <string>
#include <QString>
#include "../core/ui/UIState.h"

/**
 * シミュレーション条件をJSONファイルにエクスポートするクラス
 * nlohmann-jsonを使用してJSON形式で出力
 */
class SimulationConditionExporter {
public:
    SimulationConditionExporter() = default;
    ~SimulationConditionExporter() = default;

    /**
     * UIStateからシミュレーション条件を取得し、JSONファイルとして出力する
     * @param uiState UIStateオブジェクト
     * @param outputPath 出力先のJSONファイルパス
     * @param minElementSize メッシュの最小要素サイズ（デフォルト: 1）
     * @param maxElementSize メッシュの最大要素サイズ（デフォルト: 2）
     * @return 成功した場合true、失敗した場合false
     */
    bool exportToJson(
        const UIState* uiState,
        const QString& outputPath,
        double minElementSize = 1.0,
        double maxElementSize = 2.0
    );
};

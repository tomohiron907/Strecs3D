#pragma once
#include <QString>

class SettingsManager {
public:
    // デフォルト値（公開定数）
    static constexpr int DEFAULT_MIN_DENSITY = 5;
    static constexpr int DEFAULT_MAX_DENSITY = 90;

    // シングルトンインスタンスの取得
    static SettingsManager& instance();

    // コピー・ムーブを禁止
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    // 設定値のゲッター
    int minDensity() const { return m_minDensity; }
    int maxDensity() const { return m_maxDensity; }

    // 設定値のセッター
    void setMinDensity(int value);
    void setMaxDensity(int value);

    // 設定の保存・読み込み
    bool save();
    bool load();

    // 設定ファイルパスの取得
    static QString getSettingsFilePath();

private:
    SettingsManager();
    ~SettingsManager() = default;

    int m_minDensity = DEFAULT_MIN_DENSITY;
    int m_maxDensity = DEFAULT_MAX_DENSITY;

    bool ensureConfigDirectory();
};

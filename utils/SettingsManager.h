#pragma once
#include <QString>

class SettingsManager {
public:
    // デフォルト値（公開定数）
    static constexpr int DEFAULT_MIN_DENSITY = 5;
    static constexpr int DEFAULT_MAX_DENSITY = 90;
    static constexpr double DEFAULT_SAFETY_FACTOR = 3.0;

    // シングルトンインスタンスの取得
    static SettingsManager& instance();

    // コピー・ムーブを禁止
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    // 設定値のゲッター
    int minDensity() const { return m_minDensity; }
    int maxDensity() const { return m_maxDensity; }
    double safetyFactor() const { return m_safetyFactor; }

    // 設定値のセッター
    void setMinDensity(int value);
    void setMaxDensity(int value);
    void setSafetyFactor(double value);

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
    double m_safetyFactor = DEFAULT_SAFETY_FACTOR;
    std::string m_slicerType = "Bambu"; // Default to Bambu

    bool ensureConfigDirectory();

public:
    static constexpr const char* DEFAULT_SLICER_TYPE = "Bambu";
    static constexpr const char* DEFAULT_MATERIAL_TYPE = "PLA";
    static constexpr const char* DEFAULT_INFILL_PATTERN = "gyroid";

    std::string slicerType() const { return m_slicerType; }
    void setSlicerType(const std::string& type) { m_slicerType = type; }

    std::string materialType() const { return m_materialType; }
    void setMaterialType(const std::string& type) { m_materialType = type; }

    std::string infillPattern() const { return m_infillPattern; }
    void setInfillPattern(const std::string& pattern) { m_infillPattern = pattern; }

private:
    std::string m_materialType = "PLA";
    std::string m_infillPattern = "gyroid";
};

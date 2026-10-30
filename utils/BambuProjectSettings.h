#pragma once
#include <QByteArray>
#include <QString>
#include <optional>
#include <string>

// Bambu Studio の 3MF に同梱する project_settings.config を管理する。
// ユーザーが自分のプリンタで保存した 3MF から取り込んだ設定があればそれを使い、
// 無ければ同梱のデフォルト (A1 mini) を使う。
class BambuProjectSettings {
public:
    static std::optional<BambuProjectSettings> loadCurrent();
    // 3MF から設定を取り出して保存する。失敗時は error に理由を入れて false を返す
    static bool importFrom3mf(const std::string& path, QString& error);
    static bool resetToDefault();

    const QByteArray& data() const { return m_data; }
    const std::string& version() const { return m_version; }
    const std::string& printerName() const { return m_printerName; }
    float plateCenterX() const { return m_plateCenterX; }
    float plateCenterY() const { return m_plateCenterY; }
    int filamentCount() const { return m_filamentCount; }

private:
    static QString userSettingsPath();
    static std::optional<BambuProjectSettings> parse(const QByteArray& data);

    QByteArray m_data;
    std::string m_version;
    std::string m_printerName;
    float m_plateCenterX = 0.0f;
    float m_plateCenterY = 0.0f;
    int m_filamentCount = 0;
};

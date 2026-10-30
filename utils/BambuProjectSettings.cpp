#include "BambuProjectSettings.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringList>
#include <nlohmann/json.hpp>
#include <zip.h>
#include <algorithm>
#include <limits>

using json = nlohmann::json;

namespace {
constexpr const char* DEFAULT_SETTINGS_RESOURCE = ":/resources/bambu/project_settings.config";
constexpr const char* SETTINGS_ENTRY = "Metadata/project_settings.config";

bool readSettingsEntry(const std::string& path, QByteArray& data) {
    int errorCode = 0;
    zip_t* archive = zip_open(path.c_str(), ZIP_RDONLY, &errorCode);
    if (!archive) {
        return false;
    }

    bool success = false;
    zip_stat_t stat;
    if (zip_stat(archive, SETTINGS_ENTRY, 0, &stat) == 0 && (stat.valid & ZIP_STAT_SIZE)) {
        if (zip_file_t* file = zip_fopen(archive, SETTINGS_ENTRY, 0)) {
            data.resize(static_cast<qsizetype>(stat.size));
            success = zip_fread(file, data.data(), stat.size) == static_cast<zip_int64_t>(stat.size);
            zip_fclose(file);
        }
    }
    zip_close(archive);
    return success;
}
}

QString BambuProjectSettings::userSettingsPath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bambu_project_settings.config";
}

std::optional<BambuProjectSettings> BambuProjectSettings::parse(const QByteArray& data) {
    json j = json::parse(data.constData(), data.constData() + data.size(), nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        return std::nullopt;
    }

    const bool hasRequiredKeys = j.contains("version") && j["version"].is_string()
        && j.contains("printer_model") && j["printer_model"].is_string()
        && j.contains("nozzle_diameter") && j["nozzle_diameter"].is_array()
        && j.contains("filament_colour") && j["filament_colour"].is_array()
        && !j["filament_colour"].empty()
        && j.contains("printable_area") && j["printable_area"].is_array();
    if (!hasRequiredKeys) {
        return std::nullopt;
    }

    // printable_area は "0x0", "180x0", ... の形式
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    for (const auto& point : j["printable_area"]) {
        if (!point.is_string()) {
            return std::nullopt;
        }
        const QStringList coordinates = QString::fromStdString(point.get<std::string>()).split('x');
        bool validX = false;
        bool validY = false;
        const float x = coordinates.value(0).toFloat(&validX);
        const float y = coordinates.value(1).toFloat(&validY);
        if (coordinates.size() != 2 || !validX || !validY) {
            return std::nullopt;
        }
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }
    if (minX > maxX) {
        return std::nullopt;
    }

    BambuProjectSettings settings;
    settings.m_data = data;
    settings.m_version = j["version"].get<std::string>();
    settings.m_printerName = j.value("printer_settings_id", j["printer_model"].get<std::string>());
    settings.m_plateCenterX = (minX + maxX) / 2;
    settings.m_plateCenterY = (minY + maxY) / 2;
    settings.m_filamentCount = static_cast<int>(j["filament_colour"].size());
    return settings;
}

std::optional<BambuProjectSettings> BambuProjectSettings::loadCurrent() {
    QFile userFile(userSettingsPath());
    if (userFile.open(QIODevice::ReadOnly)) {
        if (auto settings = parse(userFile.readAll())) {
            return settings;
        }
    }

    QFile resource(DEFAULT_SETTINGS_RESOURCE);
    if (!resource.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }
    return parse(resource.readAll());
}

bool BambuProjectSettings::importFrom3mf(const std::string& path, QString& error) {
    QByteArray data;
    if (!readSettingsEntry(path, data)) {
        error = "This file has no printer settings. Select a 3MF project saved from Bambu Studio.";
        return false;
    }
    if (!parse(data)) {
        error = "The printer settings in this file could not be read.";
        return false;
    }

    const QString settingsPath = userSettingsPath();
    QDir().mkpath(QFileInfo(settingsPath).absolutePath());
    QFile file(settingsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)
        || file.write(data) != data.size()) {
        error = "Failed to save the printer settings.";
        return false;
    }
    return true;
}

bool BambuProjectSettings::resetToDefault() {
    QFile file(userSettingsPath());
    return !file.exists() || file.remove();
}

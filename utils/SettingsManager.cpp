#include "SettingsManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

SettingsManager::SettingsManager() {
    // コンストラクタで設定を読み込み
    if (!load()) {
        // ファイルが存在しない場合はデフォルト値で保存
        save();
    }
}

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance;
    return instance;
}

QString SettingsManager::getSettingsFilePath() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataLocation + "/settings.json";
}

bool SettingsManager::ensureConfigDirectory() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

void SettingsManager::setMinDensity(int value) {
    m_minDensity = value;
}

void SettingsManager::setMaxDensity(int value) {
    m_maxDensity = value;
}

bool SettingsManager::save() {
    if (!ensureConfigDirectory()) {
        return false;
    }

    json j;
    j["density_slider"]["min_density"] = m_minDensity;
    j["density_slider"]["max_density"] = m_maxDensity;
    j["slicer"]["type"] = m_slicerType;
    j["material"]["type"] = m_materialType;

    QString filePath = getSettingsFilePath();
    std::ofstream file(filePath.toStdString());
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(2);
    file.close();
    return true;
}

bool SettingsManager::load() {
    QString filePath = getSettingsFilePath();
    QFile qfile(filePath);
    if (!qfile.exists()) {
        return false;
    }

    std::ifstream file(filePath.toStdString());
    if (!file.is_open()) {
        return false;
    }

    try {
        json j = json::parse(file);
        file.close();

        if (j.contains("density_slider")) {
            auto& ds = j["density_slider"];
            if (ds.contains("min_density")) {
                m_minDensity = ds["min_density"].get<int>();
            }
            if (ds.contains("max_density")) {
                m_maxDensity = ds["max_density"].get<int>();
            }
        }
        
        if (j.contains("slicer")) {
            auto& sl = j["slicer"];
            if (sl.contains("type")) {
                m_slicerType = sl["type"].get<std::string>();
            }
        }

        if (j.contains("material")) {
            auto& mat = j["material"];
            if (mat.contains("type")) {
                m_materialType = mat["type"].get<std::string>();
            }
        }
        return true;
    } catch (const json::exception&) {
        file.close();
        return false;
    }
}

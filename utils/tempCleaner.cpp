#include "tempCleaner.h"
#include "tempPathUtility.h"
#include "fileUtility.h"
#include <QDir>
#include <iostream>

void TempCleaner::cleanupAll()
{
    QStringList subDirs = getKnownSubDirs();

    for (const QString& subDir : subDirs) {
        cleanupSubDir(subDir);
    }
}

void TempCleaner::cleanupSubDir(const QString& subDir)
{
    std::filesystem::path dirPath = TempPathUtility::getTempSubDirPath(subDir.toStdString());

    if (std::filesystem::exists(dirPath)) {
        if (FileUtility::clearDirectoryContents(dirPath)) {
            std::cout << "Cleaned up temp directory: " << dirPath.string() << std::endl;
        }
    }
}

QStringList TempCleaner::getKnownSubDirs()
{
    return {
        "div",
        "step",
        "step_import",
        "result",
        "3mf",
        "FEM",
        "mesh"
    };
}

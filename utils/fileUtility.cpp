#include "fileUtility.h"

#include <zip.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>
#include "tempPathUtility.h"

namespace fs = std::filesystem;

bool FileUtility::zipDirectory(const std::string& directoryPath, const std::string& zipFilePath) {
    // 出力ディレクトリを作成
    fs::path zipPath(zipFilePath);
    fs::path outputDir = zipPath.parent_path();
    if (!outputDir.empty()) {
        try {
            fs::create_directories(outputDir);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "出力ディレクトリの作成に失敗: " << e.what() << std::endl;
            return false;
        }
    }
    
    int errorp;
    // ZIPファイル作成（既存の場合は上書き）
    zip_t *archive = zip_open(zipFilePath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    if (!archive) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        std::cerr << "ZIPアーカイブのオープンに失敗: " 
                  << zip_error_strerror(&ziperror) << std::endl;
        zip_error_fini(&ziperror);
        return false;
    }

    try {
        fs::path dirPath(directoryPath);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "指定されたパスは存在しないか、ディレクトリではありません: " 
                      << directoryPath << std::endl;
            zip_close(archive);
            return false;
        }

        // ディレクトリ内の全ファイル・サブディレクトリを再帰的に処理
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            fs::path filePath = entry.path();
            // ZIP内でのパスは、ベースディレクトリからの相対パスにする
            std::string relativePath = fs::relative(filePath, dirPath).generic_string();

            if (fs::is_directory(filePath)) {
                // ディレクトリの場合は末尾に "/" を付ける
                std::string dirEntryName = relativePath + "/";
                if (zip_dir_add(archive, dirEntryName.c_str(), ZIP_FL_ENC_UTF_8) < 0) {
                    std::cerr << "ディレクトリエントリの追加に失敗: " 
                              << relativePath << std::endl;
                    zip_close(archive);
                    return false;
                }
            } else if (fs::is_regular_file(filePath)) {
                // ファイルの場合は zip_source_file を利用してエントリを作成
                zip_source_t* source = zip_source_file(archive, filePath.string().c_str(), 0, 0);
                if (source == nullptr) {
                    std::cerr << "ファイル用のzip source作成に失敗: " 
                              << filePath << std::endl;
                    zip_close(archive);
                    return false;
                }
                if (zip_file_add(archive, relativePath.c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
                    std::cerr << "ZIPファイルへの追加に失敗: " 
                              << relativePath << " - " << zip_strerror(archive) << std::endl;
                    zip_source_free(source);
                    zip_close(archive);
                    return false;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "ファイルシステムエラー: " << e.what() << std::endl;
        zip_close(archive);
        return false;
    }

    if (zip_close(archive) < 0) {
        std::cerr << "ZIPアーカイブのクローズに失敗: " 
                  << zip_strerror(archive) << std::endl;
        return false;
    }
    return true;
}

bool FileUtility::unzipFile(const std::string& zipFilePath, const std::string& extractToDirectory) {
    int errorp;
    zip_t* archive = zip_open(zipFilePath.c_str(), 0, &errorp);
    if (!archive) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        std::cerr << "ZIPアーカイブのオープンに失敗: " 
                  << zip_error_strerror(&ziperror) << std::endl;
        zip_error_fini(&ziperror);
        return false;
    }

    fs::path extractDir(extractToDirectory);
    try {
        if (!fs::exists(extractDir)) {
            fs::create_directories(extractDir);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "展開先ディレクトリ作成エラー: " << e.what() << std::endl;
        zip_close(archive);
        return false;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);
    for (zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(numEntries); i++) {
        zip_stat_t stat;
        if (zip_stat_index(archive, i, 0, &stat) != 0) {
            std::cerr << "ZIPエントリ情報の取得に失敗 (index " << i << ")" << std::endl;
            zip_close(archive);
            return false;
        }
        std::string entryName(stat.name);
        fs::path outPath = extractDir / entryName;

        // エントリがディレクトリの場合
        if (entryName.back() == '/') {
            try {
                fs::create_directories(outPath);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "ディレクトリ作成エラー: " << e.what() << std::endl;
                zip_close(archive);
                return false;
            }
        } else {
            // エントリがファイルの場合、まず親ディレクトリを作成
            try {
                fs::create_directories(outPath.parent_path());
            } catch (const fs::filesystem_error& e) {
                std::cerr << "親ディレクトリ作成エラー: " << e.what() << std::endl;
                zip_close(archive);
                return false;
            }

            zip_file_t* zf = zip_fopen_index(archive, i, 0);
            if (zf == nullptr) {
                std::cerr << "ZIP内のファイルオープンに失敗: " << entryName << std::endl;
                zip_close(archive);
                return false;
            }

            std::ofstream outFile(outPath, std::ios::binary);
            if (!outFile.is_open()) {
                std::cerr << "出力ファイル作成に失敗: " << outPath << std::endl;
                zip_fclose(zf);
                zip_close(archive);
                return false;
            }

            const size_t bufferSize = 4096;
            std::vector<char> buffer(bufferSize);
            zip_int64_t bytesRead = 0;
            while ((bytesRead = zip_fread(zf, buffer.data(), bufferSize)) > 0) {
                outFile.write(buffer.data(), bytesRead);
            }
            if (bytesRead < 0) {
                std::cerr << "ZIP内ファイル読み込みエラー: " << entryName << std::endl;
                zip_fclose(zf);
                zip_close(archive);
                return false;
            }
            zip_fclose(zf);
            outFile.close();
        }
    }

    if (zip_close(archive) < 0) {
        std::cerr << "ZIPアーカイブクローズ失敗: " 
                  << zip_strerror(archive) << std::endl;
        return false;
    }
    return true;
}



bool FileUtility::clearDirectoryContents(const std::filesystem::path& dir) {
    for (const auto& entry : fs::directory_iterator(dir)) {
        try {
            if (fs::is_directory(entry)) {
                // サブディレクトリの場合は再帰的に中身を削除
                clearDirectoryContents(entry.path());
            } else {
                // ファイルやシンボリックリンクの場合は削除
                fs::remove(entry);
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "エラー: " << entry.path() << " の削除に失敗しました: " << e.what() << "\n";
        }
    }
    return true;
    return true;
}

std::string FileUtility::createSafeTempCopy(const std::string& originalPath) {
    if (!fs::exists(originalPath)) {
        return "";
    }

    try {
        // 一時ディレクトリ "step_import" を使用
        fs::path tempSubDir = TempPathUtility::getTempSubDirPath("step_import");
        if (!fs::exists(tempSubDir)) {
            fs::create_directories(tempSubDir);
        }

        // ユニークなファイル名を生成 (タイムスタンプを使用)
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        // 拡張子を取得
        fs::path originalFsPath(originalPath);
        std::string extension = originalFsPath.extension().string();
        if (extension.empty()) {
            extension = ".step";
        }

        std::string safeName = "import_" + std::to_string(timestamp) + extension;
        fs::path targetPath = tempSubDir / safeName;

        // コピーを実行
        fs::copy_file(originalPath, targetPath, fs::copy_options::overwrite_existing);

        return targetPath.string();
    } catch (const std::exception& e) {
        std::cerr << "Safe temp copy failed: " << e.what() << std::endl;
        return "";
    }
}
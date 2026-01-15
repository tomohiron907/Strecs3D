#ifndef ZIPUTILITY_H
#define ZIPUTILITY_H

#include <string>
#include <filesystem>

class FileUtility {
public:
    /// @brief 指定されたディレクトリをZIPファイルに圧縮します。
    /// @param directoryPath 圧縮するディレクトリのパス
    /// @param zipFilePath 出力するZIPファイルのパス
    /// @return 圧縮に成功した場合は true、失敗した場合は false
    static bool zipDirectory(const std::string& directoryPath, const std::string& zipFilePath);
    
    /// @brief 指定されたZIPファイルを解凍して指定のディレクトリに展開します。
    /// @param zipFilePath 解凍するZIPファイルのパス
    /// @param extractToDirectory 展開先のディレクトリパス
    /// @return 解凍に成功した場合は true、失敗した場合は false
    static bool unzipFile(const std::string& zipFilePath, const std::string& extractToDirectory);
    static bool clearDirectoryContents(const std::filesystem::path& dir);

    /// @brief マルチバイト文字や空白を含むパスの問題を回避するために、ファイルを一時ディレクトリにコピーします。
    /// @param originalPath コピー元のファイルパス
    /// @return コピーされた一時ファイルのパス（失敗した場合は空文字列）
    static std::string createSafeTempCopy(const std::string& originalPath);

};

#endif // ZIPUTILITY_H

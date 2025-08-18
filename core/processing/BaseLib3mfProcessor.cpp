#include "BaseLib3mfProcessor.h"
#include "../../utils/tempPathUtility.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

bool BaseLib3mfProcessor::getMeshes(){
    std::string directoryPath = TempPathUtility::getTempSubDirPath("div").string();
    try {
        // ディレクトリの存在確認
        fs::path dirPath(directoryPath);
        if (!fs::exists(dirPath)) {
            std::cerr << "Directory does not exist: " << directoryPath << std::endl;
            return false;
        }
        if (!fs::is_directory(dirPath)) {
            std::cerr << "Path is not a directory: " << directoryPath << std::endl;
            return false;
        }
        
        std::vector<fs::directory_entry> files;

        // ディレクトリ内のエントリを走査し、通常ファイルの場合のみ vector に追加
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                files.push_back(entry);
            }
        }

        // ファイルが見つからない場合
        if (files.empty()) {
            std::cerr << "No files found in directory: " << directoryPath << std::endl;
            return false;
        }

        // ファイル名で昇順にソート
        std::sort(files.begin(), files.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.path().filename().string() < b.path().filename().string();
            }
        );

        // ソート済みのファイルを処理
        for (const auto& entry : files) {
            std::string filename = entry.path().filename().string();
            std::cout << "filename: " << filename << std::endl;
            // ここで setStl 関数を呼び出す（directoryPath と filename を結合）
            setStl(directoryPath + "/" + filename);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "file system error: " << e.what() << "\n";
        return false;
    }
    return true;
}

bool BaseLib3mfProcessor::setStl(const std::string stlFileName){
    // Import Model from File
    std::cout << "reading " << stlFileName << "..." << std::endl;
    try{
        reader->ReadFromFile(stlFileName);
    } catch (Lib3MF::ELib3MFException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    auto meshIterator = model->GetMeshObjects();
    // メッシュの総数を取得
    size_t meshCount = meshIterator->Count();
    std::cout << "Mesh count: " << meshCount << std::endl;
    //メッシュが存在しない場合はエラーメッセージを出力して終了
    if (meshCount == 0) {
        std::cerr << "No mesh objects found in the model." << std::endl;
    }
    // 最後のメッシュのインデックスを計算
    size_t lastIndex = meshCount - 1;
    size_t currentIndex = 0;      // 現在のインデックスを初期化
    Lib3MF_uint32 lastMeshID = 0; // 最後のメッシュのIDを格納する変数を初期化

    // イテレータを使用してメッシュを順に処理
    while (meshIterator->MoveNext()) {
        // 現在のインデックスが最後のインデックスに達したら
        if (currentIndex == lastIndex) {
            auto currentMesh = meshIterator->GetCurrent();      // 現在のメッシュを取得
            lastMeshID = currentMesh->GetResourceID();          // メッシュのリソースIDを取得
            break;                                              // ループを抜ける
        }
        currentIndex++; // インデックスをインクリメント
    }

    //有効なメッシュIDが取得できた場合
    if (lastMeshID != 0) {
        // メッシュIDを使用してメッシュオブジェクトを取得
        auto lastMesh = model->GetMeshObjectByID(lastMeshID);
        std::filesystem::path pathObj(stlFileName);
        std::string fileName = pathObj.filename().string();
        lastMesh->SetName(fileName); // メッシュの名前を設定
    } else {
        // メッシュIDの取得に失敗した場合はエラーメッセージを出力
        std::cerr << "Failed to set name for the last mesh from file: " << stlFileName << std::endl;
    } 
    return true;
}

bool BaseLib3mfProcessor::save3mf(const std::string outputFilename){
    // 出力ディレクトリを作成
    std::filesystem::path outputPath(outputFilename);
    std::filesystem::path outputDir = outputPath.parent_path();
    if (!outputDir.empty()) {
        try {
            std::filesystem::create_directories(outputDir);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Failed to create output directory: " << e.what() << std::endl;
            return false;
        }
    }
    
    PWriter writer = model->QueryWriter("3mf");
    std::cout << "Writing " << outputFilename << "..." << std::endl;
    writer->WriteToFile(outputFilename);
    std::cout << "Done" << std::endl;
    return true;
}
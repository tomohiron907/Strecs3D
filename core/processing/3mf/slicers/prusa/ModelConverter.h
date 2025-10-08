#ifndef MODEL_CONVERTER_H
#define MODEL_CONVERTER_H

#include <string>
#include <vector>
#include <cstddef>

// 元のオブジェクト情報を保持するための構造体
struct ModelObjectInfo {
    std::string name;             // オブジェクト名
    size_t start_triangle_index;  // 統合後モデルでの開始三角形インデックス
    size_t end_triangle_index;    // 統合後モデルでの終了三角形インデックス
};

/**
 * @class ModelConverter
 * @brief 3MFモデルファイル内の複数のオブジェクトを一つに統合するクラス
 */
class ModelConverter {
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    ModelConverter() = default;
    
    /**
     * @brief デストラクタ
     */
    ~ModelConverter() = default;
    
    /**
     * @brief コピーコンストラクタ
     */
    ModelConverter(const ModelConverter&) = default;
    
    /**
     * @brief ムーブコンストラクタ
     */
    ModelConverter(ModelConverter&&) = default;
    
    /**
     * @brief コピー代入演算子
     */
    ModelConverter& operator=(const ModelConverter&) = default;
    
    /**
     * @brief ムーブ代入演算子
     */
    ModelConverter& operator=(ModelConverter&&) = default;

    /**
     * @brief 指定された入力ファイルを処理し、オブジェクトを統合して出力ファイルに保存します。
     * @param input_path 統合元の3MFモデルファイルのパス
     * @param output_path 統合先の3MFモデルファイルのパス
     * @return 処理が成功した場合はtrue、失敗した場合はfalse
     * @throws std::runtime_error ファイル操作に失敗した場合
     */
    bool process(const std::string& input_path, const std::string& output_path);

    /**
     * @brief 統合処理中に収集したオブジェクト情報を取得します。
     * @return オブジェクト情報のベクターへのconst参照
     */
    const std::vector<ModelObjectInfo>& get_object_infos() const;

private:
    // 処理中に収集したオブジェクト情報を格納するベクター
    std::vector<ModelObjectInfo> object_infos_;
};

#endif // MODEL_CONVERTER_H
#ifndef TEMPCLEANER_H
#define TEMPCLEANER_H

#include <QString>
#include <QStringList>

/**
 * @brief アプリケーションの一時ファイルをクリーンアップするユーティリティクラス
 *
 * アプリケーション起動時・終了時に Strecs3D.temp ディレクトリをクリーンアップするために使用します。
 */
class TempCleaner {
public:
    /**
     * @brief tempディレクトリ全体をクリーンアップ
     *
     * Strecs3D.temp ディレクトリ内の全サブディレクトリの内容を削除します。
     * ディレクトリ構造自体は保持されます。
     */
    static void cleanupAll();

    /**
     * @brief 特定サブディレクトリのみクリーンアップ
     * @param subDir サブディレクトリ名（例: "div", "result", "3mf"）
     */
    static void cleanupSubDir(const QString& subDir);

    /**
     * @brief 既知サブディレクトリ一覧を取得
     * @return サブディレクトリ名のリスト
     */
    static QStringList getKnownSubDirs();
};

#endif // TEMPCLEANER_H

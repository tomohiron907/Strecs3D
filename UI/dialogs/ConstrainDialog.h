#ifndef CONSTRAINDIALOG_H
#define CONSTRAINDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <vector>

/**
 * 拘束条件を設定するためのダイアログ
 * surface_idをテーブルビューで直接編集し、OKボタンで確定する
 */
class ConstrainDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConstrainDialog(QWidget* parent = nullptr);
    ~ConstrainDialog() = default;

    // 選択されたsurface_idのリストを取得
    std::vector<int> getSelectedSurfaceIds() const;

    // 既存のsurface_idリストをロードしてテーブルに表示
    void loadSurfaceIds(const std::vector<int>& surfaceIds);

private slots:
    void onAddButtonClicked();
    void onRemoveButtonClicked();
    void onCellChanged(int row, int column);

private:
    void setupUI();
    void addNewRow();
    void addRowWithValue(int surfaceId);
    void updateRowNumbers();

    QTableWidget* surfaceTableWidget;
    QPushButton* addButton;
    QPushButton* removeButton;
    QPushButton* okButton;
    QPushButton* cancelButton;

    bool m_isLoading; // データロード中フラグ（cellChangedシグナルを一時的に無視するため）
};

#endif // CONSTRAINDIALOG_H

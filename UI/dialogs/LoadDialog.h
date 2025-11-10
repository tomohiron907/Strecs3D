#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <vector>
#include "../../core/types/BoundaryCondition.h"

/**
 * 荷重条件を設定するためのダイアログ
 * surface_id, 力の値, ベクトル(x,y,z)をテーブルビューで直接編集し、OKボタンで確定する
 */
class LoadDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadDialog(QWidget* parent = nullptr);
    ~LoadDialog() = default;

    // 選択された荷重条件のリストを取得
    std::vector<LoadCondition> getSelectedLoadConditions() const;

    // 既存の荷重条件リストをロードしてテーブルに表示
    void loadLoadConditions(const std::vector<LoadCondition>& loads);

private slots:
    void onAddButtonClicked();
    void onRemoveButtonClicked();
    void onCellChanged(int row, int column);

private:
    void setupUI();
    void addNewRow();
    void addRowWithValue(const LoadCondition& load);
    void updateRowNumbers();

    QTableWidget* loadTableWidget;
    QPushButton* addButton;
    QPushButton* removeButton;
    QPushButton* okButton;
    QPushButton* cancelButton;

    bool m_isLoading; // データロード中フラグ（cellChangedシグナルを一時的に無視するため）
};

#endif // LOADDIALOG_H

#include "ConstraintDialog.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>

ConstraintDialog::ConstraintDialog(QWidget* parent)
    : QDialog(parent), m_isLoading(false)
{
    setupUI();
}

void ConstraintDialog::setupUI()
{
    setWindowTitle("Set Constraint Conditions");
    setModal(false); // モーダルレスに変更
    setMinimumWidth(500);
    setMinimumHeight(400);

    // ダイアログを常に最前面に表示
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 説明ラベル
    QLabel* descriptionLabel = new QLabel("Add surface IDs to constraint (edit table directly):", this);
    mainLayout->addWidget(descriptionLabel);

    // テーブルウィジェット
    surfaceTableWidget = new QTableWidget(this);
    surfaceTableWidget->setColumnCount(2);
    surfaceTableWidget->setHorizontalHeaderLabels({"No.", "Surface ID"});
    surfaceTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    surfaceTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // カラム幅の設定
    surfaceTableWidget->horizontalHeader()->setStretchLastSection(true);
    surfaceTableWidget->setColumnWidth(0, 80);

    mainLayout->addWidget(surfaceTableWidget);

    // Add/Removeボタン
    QHBoxLayout* buttonRowLayout = new QHBoxLayout();
    addButton = new QPushButton("Add Row", this);
    removeButton = new QPushButton("Remove Row", this);

    buttonRowLayout->addWidget(addButton);
    buttonRowLayout->addWidget(removeButton);
    buttonRowLayout->addStretch();

    mainLayout->addLayout(buttonRowLayout);

    // OK/Cancelボタン
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // シグナル接続
    connect(addButton, &QPushButton::clicked, this, &ConstraintDialog::onAddButtonClicked);
    connect(removeButton, &QPushButton::clicked, this, &ConstraintDialog::onRemoveButtonClicked);
    connect(surfaceTableWidget, &QTableWidget::cellChanged, this, &ConstraintDialog::onCellChanged);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // スタイル設定
    setStyleSheet(R"(
        QDialog {
            background-color: #2d2d2d;
            color: #ffffff;
        }
        QLabel {
            color: #ffffff;
            font-size: 14px;
        }
        QTableWidget {
            background-color: #1a1a1a;
            color: #ffffff;
            border: 1px solid #444444;
            border-radius: 5px;
            gridline-color: #444444;
        }
        QTableWidget::item {
            padding: 5px;
        }
        QTableWidget::item:selected {
            background-color: #4a4a4a;
        }
        QHeaderView::section {
            background-color: #3a3a3a;
            color: #ffffff;
            border: 1px solid #444444;
            padding: 5px;
            font-weight: bold;
        }
        QPushButton {
            background-color: #3a3a3a;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 5px;
            padding: 8px 16px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
        QPushButton:pressed {
            background-color: #2a2a2a;
        }
    )");
}

void ConstraintDialog::addNewRow()
{
    int rowCount = surfaceTableWidget->rowCount();
    surfaceTableWidget->insertRow(rowCount);

    // No. 列（読み取り専用）
    QTableWidgetItem* noItem = new QTableWidgetItem(QString::number(rowCount + 1));
    noItem->setTextAlignment(Qt::AlignCenter);
    noItem->setFlags(noItem->flags() & ~Qt::ItemIsEditable); // 編集不可
    surfaceTableWidget->setItem(rowCount, 0, noItem);

    // Surface ID 列（編集可能）
    QTableWidgetItem* idItem = new QTableWidgetItem("");
    idItem->setTextAlignment(Qt::AlignCenter);
    surfaceTableWidget->setItem(rowCount, 1, idItem);
}

void ConstraintDialog::updateRowNumbers()
{
    for (int i = 0; i < surfaceTableWidget->rowCount(); ++i) {
        QTableWidgetItem* noItem = surfaceTableWidget->item(i, 0);
        if (noItem) {
            noItem->setText(QString::number(i + 1));
        }
    }
}

void ConstraintDialog::onAddButtonClicked()
{
    addNewRow();
}

void ConstraintDialog::onRemoveButtonClicked()
{
    int currentRow = surfaceTableWidget->currentRow();
    if (currentRow >= 0) {
        surfaceTableWidget->removeRow(currentRow);
        updateRowNumbers();
    }
}

void ConstraintDialog::addRowWithValue(int surfaceId)
{
    int rowCount = surfaceTableWidget->rowCount();
    surfaceTableWidget->insertRow(rowCount);

    // No. 列（読み取り専用）
    QTableWidgetItem* noItem = new QTableWidgetItem(QString::number(rowCount + 1));
    noItem->setTextAlignment(Qt::AlignCenter);
    noItem->setFlags(noItem->flags() & ~Qt::ItemIsEditable); // 編集不可
    surfaceTableWidget->setItem(rowCount, 0, noItem);

    // Surface ID 列（編集可能）
    QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(surfaceId));
    idItem->setTextAlignment(Qt::AlignCenter);
    surfaceTableWidget->setItem(rowCount, 1, idItem);
}

void ConstraintDialog::loadSurfaceIds(const std::vector<int>& surfaceIds)
{
    // ロード中フラグを立てる
    m_isLoading = true;

    // テーブルをクリア
    surfaceTableWidget->setRowCount(0);

    // 既存データをロード
    for (int surfaceId : surfaceIds) {
        addRowWithValue(surfaceId);
    }

    // データがない場合は空の行を1つ追加
    if (surfaceIds.empty()) {
        addNewRow();
    }

    // ロード中フラグを下ろす
    m_isLoading = false;
}

void ConstraintDialog::onCellChanged(int row, int column)
{
    // データロード中は何もしない
    if (m_isLoading) return;

    // Surface ID列の変更のみ処理
    if (column != 1) return;

    QTableWidgetItem* item = surfaceTableWidget->item(row, column);
    if (!item) return;

    QString text = item->text().trimmed();

    // 空の場合は何もしない
    if (text.isEmpty()) return;

    // 数値チェック
    bool ok;
    int value = text.toInt(&ok);

    if (!ok || value < 0) {
        // 無効な入力の場合
        QMessageBox::warning(this, "Invalid Input",
            "Please enter a valid non-negative integer for Surface ID.");
        item->setText("");
        return;
    }

    // 重複チェック
    for (int i = 0; i < surfaceTableWidget->rowCount(); ++i) {
        if (i == row) continue; // 自分自身はスキップ

        QTableWidgetItem* otherItem = surfaceTableWidget->item(i, 1);
        if (otherItem && !otherItem->text().isEmpty()) {
            if (otherItem->text().toInt() == value) {
                QMessageBox::warning(this, "Duplicate Entry",
                    QString("Surface ID %1 is already in the list.").arg(value));
                item->setText("");
                return;
            }
        }
    }
}

std::vector<int> ConstraintDialog::getSelectedSurfaceIds() const
{
    std::vector<int> surfaceIds;

    for (int i = 0; i < surfaceTableWidget->rowCount(); ++i) {
        QTableWidgetItem* item = surfaceTableWidget->item(i, 1);
        if (item && !item->text().trimmed().isEmpty()) {
            bool ok;
            int value = item->text().toInt(&ok);
            if (ok) {
                surfaceIds.push_back(value);
            }
        }
    }

    return surfaceIds;
}

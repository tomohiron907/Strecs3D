#include "LoadDialog.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>

LoadDialog::LoadDialog(QWidget* parent)
    : QDialog(parent), m_isLoading(false)
{
    setupUI();
}

void LoadDialog::setupUI()
{
    setWindowTitle("Set Load Conditions");
    setModal(false); // モーダルレスに変更
    setMinimumWidth(700);
    setMinimumHeight(400);

    // ダイアログを常に最前面に表示
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 説明ラベル
    QLabel* descriptionLabel = new QLabel("Add surface IDs, force magnitude and direction vector.\nVector format: x, y, z (e.g., 0, 0, 1)", this);
    mainLayout->addWidget(descriptionLabel);

    // テーブルウィジェット
    loadTableWidget = new QTableWidget(this);
    loadTableWidget->setColumnCount(4);
    loadTableWidget->setHorizontalHeaderLabels({"No.", "Surface ID", "Magnitude", "Vector (x,y,z)"});
    loadTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    loadTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // カラム幅の設定
    loadTableWidget->horizontalHeader()->setStretchLastSection(true);
    loadTableWidget->setColumnWidth(0, 60);
    loadTableWidget->setColumnWidth(1, 100);
    loadTableWidget->setColumnWidth(2, 100);

    mainLayout->addWidget(loadTableWidget);

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
    connect(addButton, &QPushButton::clicked, this, &LoadDialog::onAddButtonClicked);
    connect(removeButton, &QPushButton::clicked, this, &LoadDialog::onRemoveButtonClicked);
    connect(loadTableWidget, &QTableWidget::cellChanged, this, &LoadDialog::onCellChanged);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // スタイル設定（ConstrainDialogと統一）
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

void LoadDialog::addNewRow()
{
    int rowCount = loadTableWidget->rowCount();
    loadTableWidget->insertRow(rowCount);

    // No. 列（読み取り専用）
    QTableWidgetItem* noItem = new QTableWidgetItem(QString::number(rowCount + 1));
    noItem->setTextAlignment(Qt::AlignCenter);
    noItem->setFlags(noItem->flags() & ~Qt::ItemIsEditable); // 編集不可
    loadTableWidget->setItem(rowCount, 0, noItem);

    // Surface ID 列（編集可能）
    QTableWidgetItem* idItem = new QTableWidgetItem("");
    idItem->setTextAlignment(Qt::AlignCenter);
    loadTableWidget->setItem(rowCount, 1, idItem);

    // Magnitude 列（編集可能）
    QTableWidgetItem* magItem = new QTableWidgetItem("");
    magItem->setTextAlignment(Qt::AlignCenter);
    loadTableWidget->setItem(rowCount, 2, magItem);

    // Vector 列（編集可能）
    QTableWidgetItem* vecItem = new QTableWidgetItem("");
    vecItem->setTextAlignment(Qt::AlignCenter);
    loadTableWidget->setItem(rowCount, 3, vecItem);
}

void LoadDialog::updateRowNumbers()
{
    for (int i = 0; i < loadTableWidget->rowCount(); ++i) {
        QTableWidgetItem* noItem = loadTableWidget->item(i, 0);
        if (noItem) {
            noItem->setText(QString::number(i + 1));
        }
    }
}

void LoadDialog::onAddButtonClicked()
{
    addNewRow();
}

void LoadDialog::onRemoveButtonClicked()
{
    int currentRow = loadTableWidget->currentRow();
    if (currentRow >= 0) {
        loadTableWidget->removeRow(currentRow);
        updateRowNumbers();
    }
}

void LoadDialog::addRowWithValue(const LoadCondition& load)
{
    int rowCount = loadTableWidget->rowCount();
    loadTableWidget->insertRow(rowCount);

    // No. 列（読み取り専用）
    QTableWidgetItem* noItem = new QTableWidgetItem(QString::number(rowCount + 1));
    noItem->setTextAlignment(Qt::AlignCenter);
    noItem->setFlags(noItem->flags() & ~Qt::ItemIsEditable); // 編集不可
    loadTableWidget->setItem(rowCount, 0, noItem);

    // Surface ID 列
    QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(load.surface_id));
    idItem->setTextAlignment(Qt::AlignCenter);
    loadTableWidget->setItem(rowCount, 1, idItem);

    // Magnitude 列
    QTableWidgetItem* magItem = new QTableWidgetItem(QString::number(load.magnitude));
    magItem->setTextAlignment(Qt::AlignCenter);
    loadTableWidget->setItem(rowCount, 2, magItem);

    // Vector 列（x,y,z形式）
    QString vectorStr = QString("%1, %2, %3")
        .arg(load.direction.x)
        .arg(load.direction.y)
        .arg(load.direction.z);
    QTableWidgetItem* vecItem = new QTableWidgetItem(vectorStr);
    vecItem->setTextAlignment(Qt::AlignCenter);
    loadTableWidget->setItem(rowCount, 3, vecItem);
}

void LoadDialog::loadLoadConditions(const std::vector<LoadCondition>& loads)
{
    // ロード中フラグを立てる
    m_isLoading = true;

    // テーブルをクリア
    loadTableWidget->setRowCount(0);

    // 既存データをロード
    for (const LoadCondition& load : loads) {
        addRowWithValue(load);
    }

    // データがない場合は空の行を1つ追加
    if (loads.empty()) {
        addNewRow();
    }

    // ロード中フラグを下ろす
    m_isLoading = false;
}

void LoadDialog::onCellChanged(int row, int column)
{
    // データロード中は何もしない
    if (m_isLoading) return;

    // No.列の変更は無視
    if (column == 0) return;

    QTableWidgetItem* item = loadTableWidget->item(row, column);
    if (!item) return;

    QString text = item->text().trimmed();

    // 空の場合は何もしない
    if (text.isEmpty()) return;

    // 数値チェック
    bool ok;

    if (column == 1) {
        // Surface ID: 非負整数
        int value = text.toInt(&ok);
        if (!ok || value < 0) {
            QMessageBox::warning(this, "Invalid Input",
                "Please enter a valid non-negative integer for Surface ID.");
            item->setText("");
            return;
        }

        // 重複チェック
        for (int i = 0; i < loadTableWidget->rowCount(); ++i) {
            if (i == row) continue; // 自分自身はスキップ

            QTableWidgetItem* otherItem = loadTableWidget->item(i, 1);
            if (otherItem && !otherItem->text().isEmpty()) {
                if (otherItem->text().toInt() == value) {
                    QMessageBox::warning(this, "Duplicate Entry",
                        QString("Surface ID %1 is already in the list.").arg(value));
                    item->setText("");
                    return;
                }
            }
        }
    } else if (column == 2) {
        // Magnitude: 実数
        double value = text.toDouble(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Invalid Input",
                "Please enter a valid number for Magnitude.");
            item->setText("");
            return;
        }
    } else if (column == 3) {
        // Vector: カンマ区切りの3つの実数 (x, y, z)
        QStringList parts = text.split(',');

        // スペースを削除してトリム
        for (QString& part : parts) {
            part = part.trimmed();
        }

        if (parts.size() != 3) {
            QMessageBox::warning(this, "Invalid Input",
                "Please enter vector in format: x, y, z (e.g., 0, 0, 1)");
            item->setText("");
            return;
        }

        // 各成分が数値かチェック
        double x, y, z;
        x = parts[0].toDouble(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Invalid Input",
                "Invalid X component in vector. Please enter a valid number.");
            item->setText("");
            return;
        }

        y = parts[1].toDouble(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Invalid Input",
                "Invalid Y component in vector. Please enter a valid number.");
            item->setText("");
            return;
        }

        z = parts[2].toDouble(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Invalid Input",
                "Invalid Z component in vector. Please enter a valid number.");
            item->setText("");
            return;
        }

        // フォーマットを統一（スペースなしのカンマ区切り）
        QString formattedVector = QString("%1, %2, %3").arg(x).arg(y).arg(z);

        // 一時的にシグナルをブロックして、フォーマット済みテキストを設定
        m_isLoading = true;
        item->setText(formattedVector);
        m_isLoading = false;
    }
}

std::vector<LoadCondition> LoadDialog::getSelectedLoadConditions() const
{
    std::vector<LoadCondition> loadConditions;

    for (int i = 0; i < loadTableWidget->rowCount(); ++i) {
        QTableWidgetItem* idItem = loadTableWidget->item(i, 1);
        QTableWidgetItem* magItem = loadTableWidget->item(i, 2);
        QTableWidgetItem* vecItem = loadTableWidget->item(i, 3);

        // すべてのフィールドが入力されているかチェック
        if (idItem && !idItem->text().trimmed().isEmpty() &&
            magItem && !magItem->text().trimmed().isEmpty() &&
            vecItem && !vecItem->text().trimmed().isEmpty()) {

            bool ok;
            LoadCondition load;

            load.surface_id = idItem->text().toInt(&ok);
            if (!ok) continue;

            load.magnitude = magItem->text().toDouble(&ok);
            if (!ok) continue;

            // ベクトルをパース
            QString vectorText = vecItem->text().trimmed();
            QStringList parts = vectorText.split(',');

            if (parts.size() != 3) continue;

            // 各成分をトリムして数値に変換
            load.direction.x = parts[0].trimmed().toDouble(&ok);
            if (!ok) continue;

            load.direction.y = parts[1].trimmed().toDouble(&ok);
            if (!ok) continue;

            load.direction.z = parts[2].trimmed().toDouble(&ok);
            if (!ok) continue;

            load.name = "Load_" + std::to_string(i + 1);
            loadConditions.push_back(load);
        }
    }

    return loadConditions;
}

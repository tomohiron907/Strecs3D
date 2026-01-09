#include "StressRangeWidget.h"
#include "../ColorManager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QLineEdit>

StressRangeWidget::StressRangeWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // ラベル
    // ラベル
    m_label = new QLabel("Stress Range:", this);
    m_label->setStyleSheet("color: white; font-weight: bold;");
    layout->addWidget(m_label);
    
    QString inputStyle = QString("QLineEdit { color: %1; background-color: %2; border: 1px solid %3; padding: 4px; border-radius: 4px; selection-background-color: #555555; }")
        .arg(ColorManager::INPUT_TEXT_COLOR.name())
        .arg(ColorManager::INPUT_BACKGROUND_COLOR.name())
        .arg(ColorManager::INPUT_BORDER_COLOR.name());
    
    // 最小値入力欄
    m_minStressEdit = new QLineEdit(this);
    m_minStressEdit->setFixedWidth(80);
    m_minStressEdit->setPlaceholderText("Min");
    m_minStressEdit->setValidator(new QDoubleValidator(this));
    m_minStressEdit->setStyleSheet(inputStyle);
    m_minStressEdit->setText("0");
    layout->addWidget(m_minStressEdit);
    
    // 区切りラベル
    QLabel* separator = new QLabel(" - ", this);
    separator->setStyleSheet("color: white;");
    layout->addWidget(separator);
    
    // 最大値入力欄
    m_maxStressEdit = new QLineEdit(this);
    m_maxStressEdit->setFixedWidth(80);
    m_maxStressEdit->setPlaceholderText("Max");
    m_maxStressEdit->setValidator(new QDoubleValidator(this));
    m_maxStressEdit->setStyleSheet(inputStyle);
    m_maxStressEdit->setText("1");
    layout->addWidget(m_maxStressEdit);
    
    layout->addStretch(); // 右端に余白を追加
    
    // シグナル接続
    connect(m_minStressEdit, &QLineEdit::editingFinished, this, &StressRangeWidget::onMinStressChanged);
    connect(m_maxStressEdit, &QLineEdit::editingFinished, this, &StressRangeWidget::onMaxStressChanged);
}

void StressRangeWidget::setStressRange(double minStress, double maxStress) {
    m_currentMinStress = minStress;
    m_currentMaxStress = maxStress;
    
    // 科学記数法で表示 (例: 9e+6)
    m_minStressEdit->setText(QString::number(minStress, 'e', 2));
    m_maxStressEdit->setText(QString::number(maxStress, 'e', 2));
}

double StressRangeWidget::getMinStress() const {
    return m_currentMinStress;
}

double StressRangeWidget::getMaxStress() const {
    return m_currentMaxStress;
}

void StressRangeWidget::onMinStressChanged() {
    validateAndEmitChanges();
}

void StressRangeWidget::onMaxStressChanged() {
    validateAndEmitChanges();
}

void StressRangeWidget::validateAndEmitChanges() {
    bool minOk = false, maxOk = false;
    double minStress = m_minStressEdit->text().toDouble(&minOk);
    double maxStress = m_maxStressEdit->text().toDouble(&maxOk);
    
    if (minOk && maxOk && minStress < maxStress) {
        m_currentMinStress = minStress;
        m_currentMaxStress = maxStress;
        emit stressRangeChanged(minStress, maxStress);
    }
}
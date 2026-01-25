#include "SettingsWidget.h"
#include "../../utils/SettingsManager.h"
#include "../../utils/StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QGroupBox>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_minDensityEdit(nullptr)
    , m_maxDensityEdit(nullptr)
    , m_densityValidator(nullptr)
{
    setupUI();
    loadSettings();
}

void SettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Density Slider section
    QGroupBox* densityGroup = new QGroupBox("Density Slider", this);
    densityGroup->setStyleSheet(
        QString("QGroupBox {"
        "  color: #FFFFFF;"
        "  font-size: %1px;"
        "  font-weight: bold;"
        "  border: 1px solid #444444;"
        "  border-radius: %2px;"
        "  margin-top: 10px;"
        "  padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 15px;"
        "  padding: 0 %3px;"
        "}")
        .arg(StyleManager::FONT_SIZE_LARGE)
        .arg(StyleManager::RADIUS_MEDIUM)
        .arg(StyleManager::PADDING_MEDIUM / 2)
    );

    QVBoxLayout* densityLayout = new QVBoxLayout(densityGroup);
    densityLayout->setContentsMargins(20, 25, 20, 20);
    densityLayout->setSpacing(15);

    // Validator for density values (0-100)
    m_densityValidator = new QIntValidator(0, 100, this);

    // Common stylesheet for labels
    QString labelStyle = QString("QLabel { color: #CCCCCC; font-size: %1px; }")
        .arg(StyleManager::FONT_SIZE_LARGE);

    // Common stylesheet for line edits
    QString lineEditStyle =
        QString("QLineEdit {"
        "  background-color: #333333;"
        "  color: #FFFFFF;"
        "  border: 1px solid #555555;"
        "  border-radius: %1px;"
        "  padding: %2px %3px;"
        "  font-size: %4px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #0078D4;"
        "}")
        .arg(StyleManager::RADIUS_SMALL)
        .arg(StyleManager::PADDING_MEDIUM)
        .arg(StyleManager::FORM_SPACING)
        .arg(StyleManager::FONT_SIZE_LARGE);

    // Min Density row
    QHBoxLayout* minDensityRow = new QHBoxLayout();
    QLabel* minDensityLabel = new QLabel("Min Density", this);
    minDensityLabel->setStyleSheet(labelStyle);
    minDensityLabel->setFixedWidth(120);

    m_minDensityEdit = new QLineEdit(this);
    m_minDensityEdit->setValidator(m_densityValidator);
    m_minDensityEdit->setStyleSheet(lineEditStyle);
    m_minDensityEdit->setFixedWidth(100);
    m_minDensityEdit->setAlignment(Qt::AlignCenter);

    minDensityRow->addWidget(minDensityLabel);
    minDensityRow->addWidget(m_minDensityEdit);
    minDensityRow->addStretch();

    // Max Density row
    QHBoxLayout* maxDensityRow = new QHBoxLayout();
    QLabel* maxDensityLabel = new QLabel("Max Density", this);
    maxDensityLabel->setStyleSheet(labelStyle);
    maxDensityLabel->setFixedWidth(120);

    m_maxDensityEdit = new QLineEdit(this);
    m_maxDensityEdit->setValidator(m_densityValidator);
    m_maxDensityEdit->setStyleSheet(lineEditStyle);
    m_maxDensityEdit->setFixedWidth(100);
    m_maxDensityEdit->setAlignment(Qt::AlignCenter);

    maxDensityRow->addWidget(maxDensityLabel);
    maxDensityRow->addWidget(m_maxDensityEdit);
    maxDensityRow->addStretch();

    densityLayout->addLayout(minDensityRow);
    densityLayout->addLayout(maxDensityRow);

    mainLayout->addWidget(densityGroup);
    mainLayout->addStretch();

    // Connect signals
    connect(m_minDensityEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onMinDensityEditingFinished);
    connect(m_maxDensityEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onMaxDensityEditingFinished);

    // Set background
    setStyleSheet("SettingsWidget { background-color: #1a1a1a; }");
}

void SettingsWidget::loadSettings()
{
    SettingsManager& settings = SettingsManager::instance();
    m_minDensityEdit->setText(QString::number(settings.minDensity()));
    m_maxDensityEdit->setText(QString::number(settings.maxDensity()));
}

void SettingsWidget::onMinDensityEditingFinished()
{
    bool ok;
    int value = m_minDensityEdit->text().toInt(&ok);
    if (ok) {
        SettingsManager& settings = SettingsManager::instance();
        settings.setMinDensity(value);
        settings.save();
    }
}

void SettingsWidget::onMaxDensityEditingFinished()
{
    bool ok;
    int value = m_maxDensityEdit->text().toInt(&ok);
    if (ok) {
        SettingsManager& settings = SettingsManager::instance();
        settings.setMaxDensity(value);
        settings.save();
    }
}

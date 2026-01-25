#include "SettingsWidget.h"
#include "../../utils/SettingsManager.h"
#include "../../utils/StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QGroupBox>
#include <QMouseEvent>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_minDensityEdit(nullptr)
    , m_maxDensityEdit(nullptr)
    , m_densityValidator(nullptr)
{
    setupUI();
    loadSettings();
    setFocusPolicy(Qt::ClickFocus); // Allow the widget to accept focus
}

void SettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop); // Center horizontally, stack from top

    // --- Density Slider Group ---
    
    // Wrapper to ensure Title and Frame are aligned and 600px wide
    QWidget* densityWrapper = new QWidget(this);
    densityWrapper->setFixedWidth(600);
    QVBoxLayout* wrapperLayout = new QVBoxLayout(densityWrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(5); // Spacing between Title and Frame

    // Title Label (Outside Frame)
    QLabel* densityTitle = new QLabel("Density Slider", densityWrapper);
    densityTitle->setStyleSheet(
        QString("QLabel {"
        "  color: #FFFFFF;"
        "  font-size: %1px;"
        "  font-weight: bold;"
        "  border: none;"
        "}")
        .arg(StyleManager::FONT_SIZE_LARGE)
    );
    wrapperLayout->addWidget(densityTitle);

    // Frame (Container)
    QFrame* densityContainer = new QFrame(densityWrapper);
    densityContainer->setStyleSheet(
        QString("QFrame {"
        "  background-color: transparent;"
        "  border: 1px solid #444444;"
        "  border-radius: %1px;"
        "}")
        .arg(StyleManager::CONTAINER_RADIUS)
    );

    QVBoxLayout* densityLayout = new QVBoxLayout(densityContainer);
    // Increase horizontal margins (80px) to bring label and input closer
    densityLayout->setContentsMargins(80, 20, 80, 20);
    densityLayout->setSpacing(15);

    wrapperLayout->addWidget(densityContainer);

    // Validator for density values (0-100)
    m_densityValidator = new QIntValidator(0, 100, this);

    // Common stylesheet for labels
    QString labelStyle = QString("QLabel { color: #CCCCCC; font-size: %1px; border: none; }")
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
    QLabel* minDensityLabel = new QLabel("Minimum Density", this);
    minDensityLabel->setStyleSheet(labelStyle);
    // Remove fixed width from label so it takes natural width or expands slightly
    // but with stretch it will be pushed left.

    m_minDensityEdit = new QLineEdit(this);
    m_minDensityEdit->setValidator(m_densityValidator);
    m_minDensityEdit->setStyleSheet(lineEditStyle);
    m_minDensityEdit->setFixedWidth(100);
    m_minDensityEdit->setAlignment(Qt::AlignLeft);
    m_minDensityEdit->setTextMargins(0, 0, 20, 0); // Reserve space for unit

    // Add "%" unit label inside the input
    QHBoxLayout* minUnitLayout = new QHBoxLayout(m_minDensityEdit);
    minUnitLayout->setContentsMargins(0, 0, 5, 0);
    minUnitLayout->addStretch();
    QLabel* minUnitLabel = new QLabel("%", m_minDensityEdit);
    minUnitLabel->setStyleSheet("color: #666666; font-weight: bold; border: none; background: transparent;");
    minUnitLayout->addWidget(minUnitLabel);
    minUnitLayout->setAlignment(minUnitLabel, Qt::AlignRight);

    minDensityRow->addWidget(minDensityLabel);
    minDensityRow->addStretch(); // Push Input to the right
    minDensityRow->addWidget(m_minDensityEdit);

    // Max Density row
    QHBoxLayout* maxDensityRow = new QHBoxLayout();
    QLabel* maxDensityLabel = new QLabel("Maximum Density", this);
    maxDensityLabel->setStyleSheet(labelStyle);

    m_maxDensityEdit = new QLineEdit(this);
    m_maxDensityEdit->setValidator(m_densityValidator);
    m_maxDensityEdit->setStyleSheet(lineEditStyle);
    m_maxDensityEdit->setFixedWidth(100);
    m_maxDensityEdit->setAlignment(Qt::AlignLeft);
    m_maxDensityEdit->setTextMargins(0, 0, 20, 0); // Reserve space for unit

    // Add "%" unit label inside the input
    QHBoxLayout* maxUnitLayout = new QHBoxLayout(m_maxDensityEdit);
    maxUnitLayout->setContentsMargins(0, 0, 5, 0);
    maxUnitLayout->addStretch();
    QLabel* maxUnitLabel = new QLabel("%", m_maxDensityEdit);
    maxUnitLabel->setStyleSheet("color: #666666; font-weight: bold; border: none; background: transparent;");
    maxUnitLayout->addWidget(maxUnitLabel);
    maxUnitLayout->setAlignment(maxUnitLabel, Qt::AlignRight);

    maxDensityRow->addWidget(maxDensityLabel);
    maxDensityRow->addStretch(); // Push Input to the right
    maxDensityRow->addWidget(m_maxDensityEdit);

    densityLayout->addLayout(minDensityRow);
    densityLayout->addLayout(maxDensityRow);

    mainLayout->addWidget(densityWrapper);
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

void SettingsWidget::mousePressEvent(QMouseEvent* event)
{
    // Clear focus from input fields by setting focus to this widget
    this->setFocus();
    QWidget::mousePressEvent(event);
}

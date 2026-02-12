#include "SettingsWidget.h"
#include "../../utils/SettingsManager.h"
#include "../../utils/StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QGroupBox>
#include <QMouseEvent>
#include <QShowEvent>
#include <QComboBox>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_minDensityEdit(nullptr)
    , m_maxDensityEdit(nullptr)
    , m_densityValidator(nullptr)
    , m_slicerComboBox(nullptr)
    , m_materialComboBox(nullptr)
    , m_infillPatternComboBox(nullptr)
    , m_safetyFactorEdit(nullptr)
    , m_safetyFactorValidator(nullptr)
    , m_zStressFactorEdit(nullptr)
    , m_zStressFactorValidator(nullptr)
    , m_regionCountEdit(nullptr)
    , m_regionCountValidator(nullptr)
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
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    mainLayout->addWidget(createSlicerSelectionGroup());
    mainLayout->addWidget(createDensitySliderGroup());
    mainLayout->addWidget(createMaterialSelectionGroup());
    mainLayout->addWidget(createSafetyGroup());
    mainLayout->addStretch();

    connectSignals();

    setStyleSheet("SettingsWidget { background-color: #1a1a1a; }");
}

QString SettingsWidget::getTitleLabelStyle() const
{
    return QString("QLabel {"
        "  color: #FFFFFF;"
        "  font-size: %1px;"
        "  font-weight: bold;"
        "  border: none;"
        "}")
        .arg(StyleManager::FONT_SIZE_LARGE);
}

QString SettingsWidget::getContainerFrameStyle() const
{
    return QString("QFrame {"
        "  background-color: transparent;"
        "  border: 1px solid #444444;"
        "  border-radius: %1px;"
        "}")
        .arg(StyleManager::CONTAINER_RADIUS);
}

QString SettingsWidget::getInputLabelStyle() const
{
    return QString("QLabel { color: #CCCCCC; font-size: %1px; border: none; }")
        .arg(StyleManager::FONT_SIZE_LARGE);
}

QString SettingsWidget::getLineEditStyle() const
{
    return QString("QLineEdit {"
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
}

QString SettingsWidget::getComboBoxStyle() const
{
    return QString("QComboBox {"
        "  background-color: #333333;"
        "  color: #FFFFFF;"
        "  border: 1px solid #555555;"
        "  border-radius: %1px;"
        "  padding: %2px %3px;"
        "  font-size: %4px;"
        "}"
        "QComboBox:focus {"
        "  border-color: #0078D4;"
        "}"
        "QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: center right;"
        "  width: 20px;"
        "  border: none;"
        "}"
        "QComboBox::down-arrow {"
        "  image: url(:/resources/icons/arrow_down.svg);"
        "  width: 10px;"
        "  height: 10px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #333333;"
        "  color: #FFFFFF;"
        "  selection-background-color: #0078D4;"
        "}")
        .arg(StyleManager::RADIUS_SMALL)
        .arg(StyleManager::PADDING_MEDIUM)
        .arg(StyleManager::FORM_SPACING)
        .arg(StyleManager::FONT_SIZE_LARGE);
}

QWidget* SettingsWidget::createSlicerSelectionGroup()
{
    QWidget* wrapper = new QWidget(this);
    wrapper->setFixedWidth(600);

    QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(5);

    // Title
    QLabel* title = new QLabel("Slicer Selection", wrapper);
    title->setStyleSheet(getTitleLabelStyle());
    wrapperLayout->addWidget(title);

    // Container frame
    QFrame* container = new QFrame(wrapper);
    container->setStyleSheet(getContainerFrameStyle());

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(80, 20, 80, 20);

    // Export to row
    QHBoxLayout* exportRow = new QHBoxLayout();

    QLabel* exportLabel = new QLabel("Export to", container);
    exportLabel->setStyleSheet(getInputLabelStyle());

    m_slicerComboBox = new QComboBox(container);
    m_slicerComboBox->addItems({"Bambu", "Cura", "Prusa"});
    m_slicerComboBox->setStyleSheet(getComboBoxStyle());
    m_slicerComboBox->setFixedWidth(100);

    exportRow->addWidget(exportLabel);
    exportRow->addStretch();
    exportRow->addWidget(m_slicerComboBox);

    containerLayout->addLayout(exportRow);
    wrapperLayout->addWidget(container);

    return wrapper;
}

QWidget* SettingsWidget::createMaterialSelectionGroup()
{
    QWidget* wrapper = new QWidget(this);
    wrapper->setFixedWidth(600);

    QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(5);

    // Title
    QLabel* title = new QLabel("Material", wrapper);
    title->setStyleSheet(getTitleLabelStyle());
    wrapperLayout->addWidget(title);

    // Container frame
    QFrame* container = new QFrame(wrapper);
    container->setStyleSheet(getContainerFrameStyle());

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(80, 20, 80, 20);

    // Material selection row
    QHBoxLayout* materialRow = new QHBoxLayout();

    QLabel* materialLabel = new QLabel("Material Type", container);
    materialLabel->setStyleSheet(getInputLabelStyle());

    m_materialComboBox = new QComboBox(container);
    m_materialComboBox->addItems({"PLA", "ABS"});
    m_materialComboBox->setStyleSheet(getComboBoxStyle());
    m_materialComboBox->setFixedWidth(100);

    materialRow->addWidget(materialLabel);
    materialRow->addStretch();
    materialRow->addWidget(m_materialComboBox);

    containerLayout->addLayout(materialRow);
    wrapperLayout->addWidget(container);

    return wrapper;
}

QWidget* SettingsWidget::createDensitySliderGroup()
{
    QWidget* wrapper = new QWidget(this);
    wrapper->setFixedWidth(600);

    QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(5);

    // Title
    QLabel* title = new QLabel("Infill", wrapper);
    title->setStyleSheet(getTitleLabelStyle());
    wrapperLayout->addWidget(title);

    // Container frame
    QFrame* container = new QFrame(wrapper);
    container->setStyleSheet(getContainerFrameStyle());

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(80, 20, 80, 20);
    containerLayout->setSpacing(15);

    // Validator for density values (0-100)
    m_densityValidator = new QIntValidator(0, 100, this);

    // Min Density row
    m_minDensityEdit = createDensityInput();
    containerLayout->addLayout(createDensityRow("Minimum Density", m_minDensityEdit));

    // Max Density row
    m_maxDensityEdit = createDensityInput();
    containerLayout->addLayout(createDensityRow("Maximum Density", m_maxDensityEdit));

    // Infill Pattern row
    QHBoxLayout* patternRow = new QHBoxLayout();

    QLabel* patternLabel = new QLabel("Infill Pattern", container);
    patternLabel->setStyleSheet(getInputLabelStyle());

    m_infillPatternComboBox = new QComboBox(container);
    m_infillPatternComboBox->addItems({"gyroid", "grid", "cubic"});
    m_infillPatternComboBox->setStyleSheet(getComboBoxStyle());
    m_infillPatternComboBox->setFixedWidth(100);

    patternRow->addWidget(patternLabel);
    patternRow->addStretch();
    patternRow->addWidget(m_infillPatternComboBox);

    containerLayout->addLayout(patternRow);

    // Region Count row
    m_regionCountValidator = new QIntValidator(2, 10, this);

    m_regionCountEdit = new QLineEdit(container);
    m_regionCountEdit->setValidator(m_regionCountValidator);
    m_regionCountEdit->setStyleSheet(getLineEditStyle());
    m_regionCountEdit->setFixedWidth(100);
    m_regionCountEdit->setAlignment(Qt::AlignLeft);

    QHBoxLayout* regionCountRow = new QHBoxLayout();
    QLabel* regionCountLabel = new QLabel("Region Count", container);
    regionCountLabel->setStyleSheet(getInputLabelStyle());
    regionCountRow->addWidget(regionCountLabel);
    regionCountRow->addStretch();
    regionCountRow->addWidget(m_regionCountEdit);

    containerLayout->addLayout(regionCountRow);

    wrapperLayout->addWidget(container);

    return wrapper;
}

QLineEdit* SettingsWidget::createDensityInput()
{
    QLineEdit* edit = new QLineEdit(this);
    edit->setValidator(m_densityValidator);
    edit->setStyleSheet(getLineEditStyle());
    edit->setFixedWidth(100);
    edit->setAlignment(Qt::AlignLeft);
    edit->setTextMargins(0, 0, 20, 0);

    // Add "%" unit label inside the input
    QHBoxLayout* unitLayout = new QHBoxLayout(edit);
    unitLayout->setContentsMargins(0, 0, 5, 0);
    unitLayout->addStretch();

    QLabel* unitLabel = new QLabel("%", edit);
    unitLabel->setStyleSheet("color: #666666; font-weight: bold; border: none; background: transparent;");
    unitLayout->addWidget(unitLabel);
    unitLayout->setAlignment(unitLabel, Qt::AlignRight);

    return edit;
}

QHBoxLayout* SettingsWidget::createDensityRow(const QString& labelText, QLineEdit* edit)
{
    QHBoxLayout* row = new QHBoxLayout();

    QLabel* label = new QLabel(labelText, this);
    label->setStyleSheet(getInputLabelStyle());

    row->addWidget(label);
    row->addStretch();
    row->addWidget(edit);

    return row;
}

void SettingsWidget::connectSignals()
{
    connect(m_minDensityEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onMinDensityEditingFinished);
    connect(m_maxDensityEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onMaxDensityEditingFinished);
    connect(m_slicerComboBox, &QComboBox::currentTextChanged,
            this, &SettingsWidget::onSlicerTypeChanged);
    connect(m_materialComboBox, &QComboBox::currentTextChanged,
            this, &SettingsWidget::onMaterialTypeChanged);
    connect(m_infillPatternComboBox, &QComboBox::currentTextChanged,
            this, &SettingsWidget::onInfillPatternChanged);
    connect(m_safetyFactorEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onSafetyFactorEditingFinished);
    connect(m_zStressFactorEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onZStressFactorEditingFinished);
    connect(m_regionCountEdit, &QLineEdit::editingFinished,
            this, &SettingsWidget::onRegionCountEditingFinished);
}

void SettingsWidget::loadSettings()
{
    SettingsManager& settings = SettingsManager::instance();
    m_minDensityEdit->setText(QString::number(settings.minDensity()));
    m_maxDensityEdit->setText(QString::number(settings.maxDensity()));

    QString currentSlicer = QString::fromStdString(settings.slicerType());
    int slicerIndex = m_slicerComboBox->findText(currentSlicer);
    if (slicerIndex != -1) {
        m_slicerComboBox->setCurrentIndex(slicerIndex);
    }

    QString currentMaterial = QString::fromStdString(settings.materialType());
    int materialIndex = m_materialComboBox->findText(currentMaterial);
    if (materialIndex != -1) {
        m_materialComboBox->setCurrentIndex(materialIndex);
    }

    QString currentPattern = QString::fromStdString(settings.infillPattern());
    int patternIndex = m_infillPatternComboBox->findText(currentPattern);
    if (patternIndex != -1) {
        m_infillPatternComboBox->setCurrentIndex(patternIndex);
    }

    m_safetyFactorEdit->setText(QString::number(settings.safetyFactor()));
    m_zStressFactorEdit->setText(QString::number(settings.zStressFactor()));
    m_regionCountEdit->setText(QString::number(settings.regionCount()));

    m_initialLoadComplete = true;
}

void SettingsWidget::onMinDensityEditingFinished()
{
    bool ok;
    int value = m_minDensityEdit->text().toInt(&ok);
    if (ok) {
        SettingsManager& settings = SettingsManager::instance();
        if (settings.minDensity() != value) {
            settings.setMinDensity(value);
            settings.save();
            if (m_initialLoadComplete) emit settingsChanged();
        }
    }
}

void SettingsWidget::onMaxDensityEditingFinished()
{
    bool ok;
    int value = m_maxDensityEdit->text().toInt(&ok);
    if (ok) {
        SettingsManager& settings = SettingsManager::instance();
        if (settings.maxDensity() != value) {
            settings.setMaxDensity(value);
            settings.save();
            if (m_initialLoadComplete) emit settingsChanged();
        }
    }
}

void SettingsWidget::onSlicerTypeChanged(const QString& text)
{
    SettingsManager& settings = SettingsManager::instance();
    if (QString::fromStdString(settings.slicerType()) != text) {
        settings.setSlicerType(text.toStdString());
        settings.save();
        if (m_initialLoadComplete) emit settingsChanged();
    }
}

void SettingsWidget::onMaterialTypeChanged(const QString& text)
{
    SettingsManager& settings = SettingsManager::instance();
    if (QString::fromStdString(settings.materialType()) != text) {
        settings.setMaterialType(text.toStdString());
        settings.save();
        if (m_initialLoadComplete) emit settingsChanged();
    }
}

void SettingsWidget::onInfillPatternChanged(const QString& text)
{
    SettingsManager& settings = SettingsManager::instance();
    if (QString::fromStdString(settings.infillPattern()) != text) {
        settings.setInfillPattern(text.toStdString());
        settings.save();
        if (m_initialLoadComplete) emit settingsChanged();
    }
}

QWidget* SettingsWidget::createSafetyGroup()
{
    QWidget* wrapper = new QWidget(this);
    wrapper->setFixedWidth(600);

    QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(5);

    // Title
    QLabel* title = new QLabel("Safety", wrapper);
    title->setStyleSheet(getTitleLabelStyle());
    wrapperLayout->addWidget(title);

    // Container frame
    QFrame* container = new QFrame(wrapper);
    container->setStyleSheet(getContainerFrameStyle());

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(80, 20, 80, 20);

    // Validator for safety factor (0.1 - 100.0, 1 decimal)
    m_safetyFactorValidator = new QDoubleValidator(0.1, 100.0, 1, this);

    // Safety Factor input
    m_safetyFactorEdit = new QLineEdit(this);
    m_safetyFactorEdit->setValidator(m_safetyFactorValidator);
    m_safetyFactorEdit->setStyleSheet(getLineEditStyle());
    m_safetyFactorEdit->setFixedWidth(100);
    m_safetyFactorEdit->setAlignment(Qt::AlignLeft);

    containerLayout->addLayout(createDensityRow("Safety Factor", m_safetyFactorEdit));

    // Z Stress Factor input
    m_zStressFactorValidator = new QDoubleValidator(1.0, 10.0, 1, this);

    m_zStressFactorEdit = new QLineEdit(this);
    m_zStressFactorEdit->setValidator(m_zStressFactorValidator);
    m_zStressFactorEdit->setStyleSheet(getLineEditStyle());
    m_zStressFactorEdit->setFixedWidth(100);
    m_zStressFactorEdit->setAlignment(Qt::AlignLeft);

    containerLayout->addLayout(createDensityRow("Delamination Risk Multiplier", m_zStressFactorEdit));

    wrapperLayout->addWidget(container);

    return wrapper;
}

void SettingsWidget::onSafetyFactorEditingFinished()
{
    bool ok;
    double value = m_safetyFactorEdit->text().toDouble(&ok);
    if (ok && value > 0.0) {
        SettingsManager& settings = SettingsManager::instance();
        if (settings.safetyFactor() != value) {
            settings.setSafetyFactor(value);
            settings.save();
            if (m_initialLoadComplete) emit settingsChanged();
        }
    }
}

void SettingsWidget::onZStressFactorEditingFinished()
{
    bool ok;
    double value = m_zStressFactorEdit->text().toDouble(&ok);
    if (ok && value >= 1.0) {
        SettingsManager& settings = SettingsManager::instance();
        if (settings.zStressFactor() != value) {
            settings.setZStressFactor(value);
            settings.save();
            if (m_initialLoadComplete) emit settingsChanged();
        }
    }
}

void SettingsWidget::onRegionCountEditingFinished()
{
    bool ok;
    int value = m_regionCountEdit->text().toInt(&ok);
    if (ok && value >= 2 && value <= 10) {
        SettingsManager& settings = SettingsManager::instance();
        if (settings.regionCount() != value) {
            settings.setRegionCount(value);
            settings.save();
            emit regionCountChanged(value);
            if (m_initialLoadComplete) emit settingsChanged();
        }
    }
}

void SettingsWidget::mousePressEvent(QMouseEvent* event)
{
    // Clear focus from input fields by setting focus to this widget
    this->setFocus();
    QWidget::mousePressEvent(event);
}

void SettingsWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Prevent auto-focus on child input widgets when the tab is shown
    this->setFocus();
}

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
#include <QComboBox>
#include <QMessageBox>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_minDensityEdit(nullptr)
    , m_maxDensityEdit(nullptr)
    , m_densityValidator(nullptr)
    , m_slicerComboBox(nullptr)
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

QWidget* SettingsWidget::createDensitySliderGroup()
{
    QWidget* wrapper = new QWidget(this);
    wrapper->setFixedWidth(600);

    QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(5);

    // Title
    QLabel* title = new QLabel("Density Slider", wrapper);
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
}

void SettingsWidget::loadSettings()
{
    SettingsManager& settings = SettingsManager::instance();
    m_minDensityEdit->setText(QString::number(settings.minDensity()));
    m_maxDensityEdit->setText(QString::number(settings.maxDensity()));

    QString currentSlicer = QString::fromStdString(settings.slicerType());
    int index = m_slicerComboBox->findText(currentSlicer);
    if (index != -1) {
        m_slicerComboBox->setCurrentIndex(index);
    }

    m_initialLoadComplete = true;
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

void SettingsWidget::onSlicerTypeChanged(const QString& text)
{
    // Save settings to SettingsManager
    SettingsManager& settings = SettingsManager::instance();
    settings.setSlicerType(text.toStdString());
    settings.save();

    // Show restart required message (only after initial load)
    if (m_initialLoadComplete) {
        QMessageBox::information(
            this,
            tr("Restart Required"),
            tr("The slicer type has been changed to %1.\n"
               "Please restart the application to apply this setting.")
               .arg(text)
        );
    }
}

void SettingsWidget::mousePressEvent(QMouseEvent* event)
{
    // Clear focus from input fields by setting focus to this widget
    this->setFocus();
    QWidget::mousePressEvent(event);
}

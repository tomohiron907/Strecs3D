#include "PropertyWidget.h"
#include <QScrollArea>
#include <QFrame>
#include <QDebug>

PropertyWidget::PropertyWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void PropertyWidget::setupUI()
{
    // Main layout for the widget itself
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // Container Frame to provide the unified rounded border look
    QFrame* containerFrame = new QFrame(this);
    containerFrame->setStyleSheet(R"(
        QFrame {
            background-color: rgba(26, 26, 26, 180);
            border: 1px solid #444;
            border-radius: 4px;
        }
        QLabel {
            border: none;
            background-color: transparent;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(containerFrame);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Title
    m_titleLabel = new QLabel("Properties", containerFrame);
    // Remove specific background and borders so it blends into the container,
    // or style specifically if needed (e.g., bottom border).
    // Adding a bottom border to separate title from content visually if desired, 
    // or keep it clean. Let's keep it clean but maybe bolder.
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 8px; color: #ffffff; border-bottom: 1px solid #444;");
    containerLayout->addWidget(m_titleLabel);

    // Scroll Area for properties
    QScrollArea* scrollArea = new QScrollArea(containerFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    // Valid stylesheet for scrollarea inside the container
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

    m_contentWidget = new QWidget();
    // Ensure content widget is transparent so the frame background shows through
    m_contentWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_contentWidget->setStyleSheet("background-color: transparent;"); 
    
    m_formLayout = new QFormLayout(m_contentWidget);
    m_formLayout->setLabelAlignment(Qt::AlignLeft);
    m_formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_formLayout->setSpacing(10);
    m_formLayout->setContentsMargins(10, 10, 10, 10);

    scrollArea->setWidget(m_contentWidget);
    containerLayout->addWidget(scrollArea);

    // Add the container to the main layout
    m_mainLayout->addWidget(containerFrame);
}

void PropertyWidget::setUIState(UIState* uiState)
{
    m_uiState = uiState;
}

void PropertyWidget::clearProperties()
{
    // Clear form layout
    QLayoutItem* item;
    while ((item = m_formLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void PropertyWidget::addProperty(const QString& label, const QString& value)
{
    QLabel* labelWidget = new QLabel(label);
    labelWidget->setStyleSheet("color: #aaaaaa;");
    QLabel* valueWidget = new QLabel(value);
    valueWidget->setStyleSheet("color: #ffffff;");
    valueWidget->setWordWrap(true);
    m_formLayout->addRow(labelWidget, valueWidget);
}

void PropertyWidget::addSectionHeader(const QString& title)
{
    QLabel* header = new QLabel(title);
    header->setStyleSheet("font-weight: bold; color: #dddddd; margin-top: 10px; margin-bottom: 5px;");
    m_formLayout->addRow(header);
}

void PropertyWidget::onObjectSelected(ObjectType type, const QString& id, int index)
{
    clearProperties();
    
    if (!m_uiState) return;
    
    ObjectListData data = m_uiState->getObjectList();

    switch (type) {
        case ObjectType::ITEM_STEP: {
            m_titleLabel->setText("Model Properties");
            ObjectFileInfo info = data.step;
            addProperty("Filename", info.filename);
            addProperty("Path", info.filePath);
            addProperty("Visible", info.isVisible ? "Yes" : "No");
            addProperty("Transparency", QString::number(info.transparency));
            break;
        }
        case ObjectType::ITEM_SIMULATION: {
            m_titleLabel->setText("Simulation Result");
            ObjectFileInfo info = data.simulationResult;
            addProperty("Filename", info.filename);
            addProperty("Path", info.filePath);
            addProperty("Visible", info.isVisible ? "Yes" : "No");
            break;
        }
        case ObjectType::ITEM_INFILL_REGION: {
            m_titleLabel->setText("Infill Region");
            InfillRegionInfo info = m_uiState->getInfillRegion(id);
            addProperty("Name", info.name);
            addProperty("Key", id);
            addProperty("Visible", info.isVisible ? "Yes" : "No");
            break;
        }
        case ObjectType::ITEM_BC_CONSTRAINT: {
            m_titleLabel->setText("Constraint");
            if (index >= 0 && index < (int)data.boundaryCondition.constraints.size()) {
                const auto& c = data.boundaryCondition.constraints[index];
                addProperty("Name", QString::fromStdString(c.name));
                addProperty("Surface ID", QString::number(c.surface_id));
            }
            break;
        }
        case ObjectType::ITEM_BC_LOAD: {
            m_titleLabel->setText("Load Condition");
            if (index >= 0 && index < (int)data.boundaryCondition.loads.size()) {
                const auto& l = data.boundaryCondition.loads[index];
                addProperty("Name", QString::fromStdString(l.name));
                addProperty("Surface ID", QString::number(l.surface_id));
                addProperty("Magnitude", QString::number(l.magnitude));
                addProperty("Direction", QString("X: %1\nY: %2\nZ: %3")
                                         .arg(l.direction.x)
                                         .arg(l.direction.y)
                                         .arg(l.direction.z));
            }
            break;
        }
        default:
            m_titleLabel->setText("Properties");
            addProperty("Info", "Select an item to view properties.");
            break;
    }
}

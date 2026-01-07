#include "ObjectListWidget.h"
#include <QHeaderView>
#include <QDebug>

ObjectListWidget::ObjectListWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setupUI();
    connect(this, &QTreeWidget::itemSelectionChanged, this, &ObjectListWidget::onSelectionChanged);
}

void ObjectListWidget::setupUI()
{
    setHeaderLabel("Scene Objects");
    header()->setVisible(true);
    setIndentation(20);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAnimated(true);
    
    // スタイル設定（オプション）
    setStyleSheet(
        "QTreeWidget { background-color: #2b2b2b; color: #e0e0e0; border: none; }"
        "QTreeWidget::item:selected { background-color: #4a4a4a; }"
        "QHeaderView::section { background-color: #333333; color: #e0e0e0; border: none; padding: 4px; }"
    );
}

void ObjectListWidget::setUIState(UIState* uiState)
{
    if (m_uiState) {
        disconnect(m_uiState, nullptr, this, nullptr);
    }
    
    m_uiState = uiState;
    
    if (m_uiState) {
        // Initial build
        onObjectListChanged(m_uiState->getObjectList());
        
        // Connect signals
        connect(m_uiState, &UIState::objectListChanged, this, &ObjectListWidget::onObjectListChanged);
        
        connect(m_uiState, &UIState::stepFileInfoChanged, this, &ObjectListWidget::onStepInfoChanged);
        connect(m_uiState, &UIState::simulationResultFileInfoChanged, this, &ObjectListWidget::onSimResultInfoChanged);
        
        connect(m_uiState, &UIState::infillRegionAdded, this, &ObjectListWidget::onInfillRegionAdded);
        connect(m_uiState, &UIState::infillRegionRemoved, this, [this](const QString& key){
            clearAndRebuild();
        });
        
        connect(m_uiState, &UIState::boundaryConditionChanged, this, &ObjectListWidget::onBoundaryConditionChanged);
    }
}

void ObjectListWidget::onObjectListChanged(const ObjectListData& objectList)
{
    clearAndRebuild();
}

void ObjectListWidget::clearAndRebuild()
{
    clear();
    
    if (!m_uiState) return;
    
    ObjectListData data = m_uiState->getObjectList();
    
    // 1. STP Model
    m_stepRoot = new ObjectTreeItem(this);
    m_stepRoot->setText(0, "Model (STEP)");
    m_stepRoot->type = ObjectType::ROOT_STEP;
    m_stepRoot->setExpanded(true);
    
    if (!data.step.filePath.isEmpty()) {
        ObjectTreeItem* item = new ObjectTreeItem(m_stepRoot);
        item->setText(0, data.step.filename.isEmpty() ? "Unknown" : data.step.filename);
        item->type = ObjectType::ITEM_STEP;
    }
    
    // 2. Simulation Result
    m_simRoot = new ObjectTreeItem(this);
    m_simRoot->setText(0, "Simulation Result");
    m_simRoot->type = ObjectType::ROOT_SIMULATION;
    
    if (!data.simulationResult.filePath.isEmpty()) {
        ObjectTreeItem* item = new ObjectTreeItem(m_simRoot);
        item->setText(0, data.simulationResult.filename.isEmpty() ? "Unknown" : data.simulationResult.filename);
        item->type = ObjectType::ITEM_SIMULATION;
        m_simRoot->setExpanded(true);
    }
    
    // 3. Infill Regions
    m_infillRoot = new ObjectTreeItem(this);
    m_infillRoot->setText(0, QString("Infill Regions (%1)").arg(data.infillRegions.size()));
    m_infillRoot->type = ObjectType::ROOT_INFILL;
    
    for (const auto& [key, info] : data.infillRegions) {
        ObjectTreeItem* item = new ObjectTreeItem(m_infillRoot);
        item->setText(0, info.name);
        item->type = ObjectType::ITEM_INFILL_REGION;
        item->id = key;
    }
    
    // 4. Boundary Conditions
    m_bcRoot = new ObjectTreeItem(this);
    m_bcRoot->setText(0, "Boundary Conditions");
    m_bcRoot->type = ObjectType::ROOT_BC;
    m_bcRoot->setExpanded(true);
    
    // Constraints
    m_bcConstraintsRoot = new ObjectTreeItem(m_bcRoot);
    m_bcConstraintsRoot->setText(0, QString("Constraints (%1)").arg(data.boundaryCondition.constraints.size()));
    m_bcConstraintsRoot->setExpanded(true);
    
    for (size_t i = 0; i < data.boundaryCondition.constraints.size(); ++i) {
        const auto& c = data.boundaryCondition.constraints[i];
        ObjectTreeItem* item = new ObjectTreeItem(m_bcConstraintsRoot);
        item->setText(0, QString::fromStdString(c.name));
        item->type = ObjectType::ITEM_BC_CONSTRAINT;
        item->index = static_cast<int>(i);
    }
    
    // Loads
    m_bcLoadsRoot = new ObjectTreeItem(m_bcRoot);
    m_bcLoadsRoot->setText(0, QString("Loads (%1)").arg(data.boundaryCondition.loads.size()));
    m_bcLoadsRoot->setExpanded(true);
    
    for (size_t i = 0; i < data.boundaryCondition.loads.size(); ++i) {
        const auto& l = data.boundaryCondition.loads[i];
        ObjectTreeItem* item = new ObjectTreeItem(m_bcLoadsRoot);
        item->setText(0, QString::fromStdString(l.name));
        item->type = ObjectType::ITEM_BC_LOAD;
        item->index = static_cast<int>(i);
    }
}

void ObjectListWidget::onStepInfoChanged(const ObjectFileInfo& info)
{
    // 簡易実装: ファイルが変わったら全リビルド
    clearAndRebuild();
}

void ObjectListWidget::onSimResultInfoChanged(const ObjectFileInfo& info)
{
    clearAndRebuild();
}

void ObjectListWidget::onInfillRegionAdded(const QString& key, const InfillRegionInfo& info)
{
    clearAndRebuild();
}

void ObjectListWidget::onBoundaryConditionChanged(const BoundaryCondition& bc)
{
    clearAndRebuild();
}

void ObjectListWidget::onSelectionChanged()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.isEmpty()) return;
    
    ObjectTreeItem* item = static_cast<ObjectTreeItem*>(selected.first());
    if (item) {
        emit objectSelected(item->type, item->id, item->index);
    }
}

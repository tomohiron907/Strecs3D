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
    // スタイル設定（オプション）
    // MessageConsoleと同様の角丸ウィジェットスタイルを適用
    setStyleSheet(R"(
        QTreeWidget {
            background-color: rgba(26, 26, 26, 180);
            color: #e0e0e0;
            border: 1px solid #444;
            border-radius: 4px;
            font-size: 12px;
        }
        QTreeWidget::item:selected {
            background-color: #4a4a4a;
        }
        /* ヘッダーの背景を透明にして、親ウィジェット（QTreeWidget）の角丸が見えるようにする */
        QHeaderView::section {
            background-color: transparent;
            color: #ffffff;
            font-weight: bold;
            font-size: 14px;
            border: none;
            padding: 4px 8px;
            border-bottom: 1px solid #444; /* ヘッダー下の区切り線 */
        }
        QHeaderView {
            background-color: transparent;
            border: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
    )");
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
        
        // Connect selection signal
        connect(m_uiState, &UIState::selectedObjectChanged, this, &ObjectListWidget::onSelectedObjectChangedFromState);
    }
}

void ObjectListWidget::onObjectListChanged(const ObjectListData& objectList)
{
    clearAndRebuild();
}

void ObjectListWidget::clearAndRebuild()
{
    // Save current selection
    ObjectType selectedType = ObjectType::ROOT_STEP; // Default dummy
    QString selectedId = "";
    int selectedIndex = -1;
    bool hasSelection = false;

    QList<QTreeWidgetItem*> selectedItemsList = selectedItems();
    if (!selectedItemsList.isEmpty()) {
        ObjectTreeItem* item = static_cast<ObjectTreeItem*>(selectedItemsList.first());
        if (item) {
            selectedType = item->type;
            selectedId = item->id;
            selectedIndex = item->index;
            hasSelection = true;
        }
    }

    clear();
    
    if (!m_uiState) return;
    
    ObjectListData data = m_uiState->getObjectList();
    
    // Reset root pointers
    m_stepRoot = nullptr;
    m_simRoot = nullptr;
    m_infillRoot = nullptr;
    m_bcRoot = nullptr;
    m_bcConstraintsRoot = nullptr;
    m_bcLoadsRoot = nullptr;

    // Display items in order
    for (const QString& key : data.displayOrder) {
        if (key == ObjectListData::KEY_STEP) {
             // 1. STP Model
            if (!data.step.filePath.isEmpty()) {
                m_stepRoot = new ObjectTreeItem(this);
                m_stepRoot->setText(0, "Model (STEP)");
                m_stepRoot->type = ObjectType::ROOT_STEP;
                m_stepRoot->setExpanded(true);
                
                ObjectTreeItem* item = new ObjectTreeItem(m_stepRoot);
                item->setText(0, data.step.filename.isEmpty() ? "Unknown" : data.step.filename);
                item->type = ObjectType::ITEM_STEP;
            }
        } 
        else if (key == ObjectListData::KEY_SIMULATION) {
            // 2. Simulation Result
            if (!data.simulationResult.filePath.isEmpty()) {
                m_simRoot = new ObjectTreeItem(this);
                m_simRoot->setText(0, "Simulation Result");
                m_simRoot->type = ObjectType::ROOT_SIMULATION;
                m_simRoot->setExpanded(true);
                
                ObjectTreeItem* item = new ObjectTreeItem(m_simRoot);
                item->setText(0, data.simulationResult.filename.isEmpty() ? "Unknown" : data.simulationResult.filename);
                item->type = ObjectType::ITEM_SIMULATION;
            }
        }
        else if (key == ObjectListData::KEY_INFILL) {
            // 3. Infill Regions
            if (!data.infillRegions.empty()) {
                m_infillRoot = new ObjectTreeItem(this);
                m_infillRoot->setText(0, QString("Infill Regions (%1)").arg(data.infillRegions.size()));
                m_infillRoot->type = ObjectType::ROOT_INFILL;
                
                for (const auto& [key, info] : data.infillRegions) {
                    ObjectTreeItem* item = new ObjectTreeItem(m_infillRoot);
                    item->setText(0, info.name);
                    item->type = ObjectType::ITEM_INFILL_REGION;
                    item->id = key;
                }
            }
        }
        else if (key == ObjectListData::KEY_BC) {
            // 4. Boundary Conditions
            bool hasConstraints = !data.boundaryCondition.constraints.empty();
            bool hasLoads = !data.boundaryCondition.loads.empty();
            
            if (hasConstraints || hasLoads) {
                m_bcRoot = new ObjectTreeItem(this);
                m_bcRoot->setText(0, "Boundary Conditions");
                m_bcRoot->type = ObjectType::ROOT_BC;
                m_bcRoot->setExpanded(true);
                
                // Constraints
                if (hasConstraints) {
                    m_bcConstraintsRoot = new ObjectTreeItem(m_bcRoot);
                    m_bcConstraintsRoot->setText(0, QString("Constraints (%1)").arg(data.boundaryCondition.constraints.size()));
                    m_bcConstraintsRoot->setExpanded(true);
                    m_bcConstraintsRoot->type = ObjectType::ROOT_BC_CONSTRAINTS;
                    
                    for (size_t i = 0; i < data.boundaryCondition.constraints.size(); ++i) {
                        const auto& c = data.boundaryCondition.constraints[i];
                        ObjectTreeItem* item = new ObjectTreeItem(m_bcConstraintsRoot);
                        item->setText(0, QString::fromStdString(c.name));
                        item->type = ObjectType::ITEM_BC_CONSTRAINT;
                        item->index = static_cast<int>(i);
                    }
                }
                
                // Loads
                if (hasLoads) {
                    m_bcLoadsRoot = new ObjectTreeItem(m_bcRoot);
                    m_bcLoadsRoot->setText(0, QString("Loads (%1)").arg(data.boundaryCondition.loads.size()));
                    m_bcLoadsRoot->setExpanded(true);
                    m_bcLoadsRoot->type = ObjectType::ROOT_BC_LOADS;
                    
                    for (size_t i = 0; i < data.boundaryCondition.loads.size(); ++i) {
                        const auto& l = data.boundaryCondition.loads[i];
                        ObjectTreeItem* item = new ObjectTreeItem(m_bcLoadsRoot);
                        item->setText(0, QString::fromStdString(l.name));
                        item->type = ObjectType::ITEM_BC_LOAD;
                        item->index = static_cast<int>(i);
                    }
                }
            }
        }
    }

    // Restore selection
    if (hasSelection) {
        // Find item recursively
        QTreeWidgetItemIterator it(this);
        while (*it) {
            ObjectTreeItem* item = static_cast<ObjectTreeItem*>(*it);
            if (item->type == selectedType) {
                bool match = true;
                if (selectedType == ObjectType::ITEM_INFILL_REGION) {
                    if (item->id != selectedId) match = false;
                } else if (selectedType == ObjectType::ITEM_BC_CONSTRAINT || selectedType == ObjectType::ITEM_BC_LOAD) {
                    if (item->index != selectedIndex) match = false;
                }
                
                if (match) {
                    // Temporarily block signals to prevent recursive selection resets if any
                    // But here we WANT to trigger onSelectionChanged eventually? 
                    // No, usually we just want to highlight it. onSelectionChanged emits objectSelected.
                    // If we re-select, it might re-emit, effectively refreshing the property widget.
                    // That is actually DESIRED if the object data changed (e.g. name update).
                    setCurrentItem(item);
                    break;
                }
            }
            ++it;
        }
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
        // UIStateを更新
        if (m_uiState) {
            m_uiState->setSelectedObject(item->type, item->id, item->index);
        }
        
        emit objectSelected(item->type, item->id, item->index);
    }
}

void ObjectListWidget::onSelectedObjectChangedFromState(const SelectedObjectInfo& selection)
{
    // 選択なしの場合はツリーの選択を解除
    if (selection.type == ObjectType::NONE) {
        clearSelection();
        return;
    }
    
    // 現在の選択と一致しているかチェック
    QList<QTreeWidgetItem*> selectedItemsList = selectedItems();
    if (!selectedItemsList.isEmpty()) {
        ObjectTreeItem* currentItem = static_cast<ObjectTreeItem*>(selectedItemsList.first());
        if (currentItem->type == selection.type && 
            currentItem->id == selection.id && 
            currentItem->index == selection.index) {
            return; // 既に選択されているので何もしない（ループ防止）
        }
    }

    // リスト内のアイテムを探して選択する
    selectObject(selection.type, selection.id, selection.index);
}

void ObjectListWidget::selectObject(ObjectType type, const QString& id, int index)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ObjectTreeItem* item = static_cast<ObjectTreeItem*>(*it);
        if (item->type == type) {
            bool match = true;
            if (!id.isEmpty() && item->id != id) match = false;
            if (index >= 0 && item->index != index) match = false;
            
            if (match) {
                // Clear existing selection first to ensure clean state
                clearSelection();
                setCurrentItem(item);
                // Also ensure it is visible
                scrollToItem(item);
                return;
            }
        }
        ++it;
    }
}

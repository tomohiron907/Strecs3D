#ifndef OBJECTLISTWIDGET_H
#define OBJECTLISTWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QObject>
#include "../../core/ui/UIState.h"

// オブジェクトの種類を識別するための列挙型
enum class ObjectType {
    ROOT_STEP,
    ROOT_SIMULATION,
    ROOT_INFILL,
    ROOT_BC,
    ROOT_BC_CONSTRAINTS,
    ROOT_BC_LOADS,
    
    ITEM_STEP,
    ITEM_SIMULATION,
    ITEM_INFILL_REGION,
    ITEM_BC_CONSTRAINT,
    ITEM_BC_LOAD
};

// ツリーアイテムにデータを保持させるためのカスタムアイテム
class ObjectTreeItem : public QTreeWidgetItem {
public:
    ObjectTreeItem(QTreeWidget* parent) : QTreeWidgetItem(parent) {}
    ObjectTreeItem(QTreeWidgetItem* parent) : QTreeWidgetItem(parent) {}
    
    ObjectType type;
    QString id; // Infill key, or surface ID, etc.
    int index = -1; // Vector index for constraints/loads
};

class ObjectListWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit ObjectListWidget(QWidget* parent = nullptr);
    ~ObjectListWidget() = default;

    void setUIState(UIState* uiState);
    void selectObject(ObjectType type, const QString& id = "", int index = -1);

signals:
    void objectSelected(ObjectType type, const QString& id, int index);

private slots:
    void onObjectListChanged(const ObjectListData& objectList);
    void onSelectionChanged();

    // Individual updates
    void onStepInfoChanged(const ObjectFileInfo& info);
    void onSimResultInfoChanged(const ObjectFileInfo& info);
    void onInfillRegionAdded(const QString& key, const InfillRegionInfo& info);
    void onBoundaryConditionChanged(const BoundaryCondition& bc);

private:
    void setupUI();
    void clearAndRebuild();
    
    UIState* m_uiState = nullptr;
    
    // Root items
    ObjectTreeItem* m_stepRoot = nullptr;
    ObjectTreeItem* m_simRoot = nullptr;
    ObjectTreeItem* m_infillRoot = nullptr;
    ObjectTreeItem* m_bcRoot = nullptr;
    ObjectTreeItem* m_bcConstraintsRoot = nullptr;
    ObjectTreeItem* m_bcLoadsRoot = nullptr;
};

#endif // OBJECTLISTWIDGET_H

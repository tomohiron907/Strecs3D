#include "BoundaryConditionController.h"
#include "../../core/ui/UIState.h"
#include "../visualization/VisualizationManager.h"
#include "../widgets/process/ProcessManagerWidget.h"
#include "../widgets/process/steps/BoundaryConditionStepWidget.h"
#include "../widgets/process/AddConstraintDialog.h"
#include "../widgets/process/AddLoadDialog.h"
#include "../../core/commands/state/SetConstraintConditionCommand.h"
#include "../../core/commands/state/SetLoadConditionCommand.h"
#include "../../core/commands/state/UpdateConstraintConditionCommand.h"
#include "../../core/commands/state/UpdateLoadConditionCommand.h"

#include <QWidget>
#include <QDebug>

BoundaryConditionController::BoundaryConditionController(
    UIState* uiState,
    VisualizationManager* vizManager,
    ProcessManagerWidget* processManager,
    QWidget* parent)
    : QObject(parent)
    , uiState_(uiState)
    , vizManager_(vizManager)
    , processManager_(processManager)
    , parentWidget_(parent)
{
}

void BoundaryConditionController::showAddConstraintDialog()
{
    if (!uiState_) return;

    BoundaryCondition bc = uiState_->getBoundaryCondition();
    QString newName = generateUniqueName("Constraint", bc, true);

    AddConstraintDialog* dialog = new AddConstraintDialog(newName, parentWidget_);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->move(calculateDialogPosition());

    if (vizManager_) {
        dialog->setVisualizationManager(vizManager_);
    }

    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        ConstraintCondition constraint = dialog->getConstraintCondition();

        auto command = std::make_unique<SetConstraintConditionCommand>(
            uiState_,
            constraint
        );
        command->execute();

        qDebug() << "Added new Constraint Condition:" << QString::fromStdString(constraint.name);
    });

    dialog->show();
}

void BoundaryConditionController::showAddLoadDialog()
{
    if (!uiState_) return;

    BoundaryCondition bc = uiState_->getBoundaryCondition();
    QString newName = generateUniqueName("Load", bc, false);

    AddLoadDialog* dialog = new AddLoadDialog(newName, parentWidget_);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->move(calculateDialogPosition());

    if (vizManager_) {
        dialog->setVisualizationManager(vizManager_);
    }

    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        LoadCondition load = dialog->getLoadCondition();

        auto command = std::make_unique<SetLoadConditionCommand>(
            uiState_,
            load
        );
        command->execute();

        qDebug() << "Added new Load Condition:" << QString::fromStdString(load.name);
    });

    dialog->show();
}

void BoundaryConditionController::handleFaceClicked(int faceId, double nx, double ny, double nz)
{
    // Face click currently does nothing - selection is handled by double-click
    Q_UNUSED(faceId);
    Q_UNUSED(nx);
    Q_UNUSED(ny);
    Q_UNUSED(nz);
}

void BoundaryConditionController::handleFaceDoubleClicked(int faceId, double nx, double ny, double nz)
{
    if (!uiState_) return;

    SelectedObjectInfo selection = uiState_->getSelectedObject();

    if (selection.type == ObjectType::ITEM_BC_CONSTRAINT) {
        if (selection.index >= 0) {
            BoundaryCondition bc = uiState_->getBoundaryCondition();
            if (selection.index < static_cast<int>(bc.constraints.size())) {
                ConstraintCondition c = bc.constraints[selection.index];
                c.surface_id = faceId;

                auto command = std::make_unique<UpdateConstraintConditionCommand>(
                    uiState_,
                    selection.index,
                    c
                );
                command->execute();

                qDebug() << QString("Updated Constraint '%1' to Surface ID: %2")
                    .arg(QString::fromStdString(c.name))
                    .arg(faceId);
            }
        }
    } else if (selection.type == ObjectType::ITEM_BC_LOAD) {
        if (selection.index >= 0) {
            BoundaryCondition bc = uiState_->getBoundaryCondition();
            if (selection.index < static_cast<int>(bc.loads.size())) {
                LoadCondition l = bc.loads[selection.index];
                l.surface_id = faceId;
                l.direction = {-nx, -ny, -nz};
                l.reference_edge_id = 0;

                auto command = std::make_unique<UpdateLoadConditionCommand>(
                    uiState_,
                    selection.index,
                    l
                );
                command->execute();

                qDebug() << QString("Updated Load '%1' to Surface ID: %2, Direction: (%3, %4, %5)")
                    .arg(QString::fromStdString(l.name))
                    .arg(faceId)
                    .arg(-nx, 0, 'f', 2)
                    .arg(-ny, 0, 'f', 2)
                    .arg(-nz, 0, 'f', 2);
            }
        }
    }
}

void BoundaryConditionController::updateVisualization()
{
    if (!uiState_ || !vizManager_) return;

    BoundaryCondition condition = uiState_->getBoundaryCondition();
    vizManager_->displayBoundaryConditions(condition);
}

QString BoundaryConditionController::generateUniqueName(const QString& prefix, const BoundaryCondition& bc, bool isConstraint)
{
    int nextId = 1;
    QString newName;
    while (true) {
        newName = prefix + QString::number(nextId);
        bool exists = false;

        if (isConstraint) {
            for (const auto& c : bc.constraints) {
                if (c.name == newName.toStdString()) {
                    exists = true;
                    break;
                }
            }
        } else {
            for (const auto& l : bc.loads) {
                if (l.name == newName.toStdString()) {
                    exists = true;
                    break;
                }
            }
        }

        if (!exists) break;
        nextId++;
    }
    return newName;
}

QPoint BoundaryConditionController::calculateDialogPosition()
{
    if (processManager_) {
        if (auto* bcWidget = processManager_->getBoundaryConditionStep()) {
            QPoint widgetPos = bcWidget->mapToGlobal(QPoint(0, 0));
            int targetX = widgetPos.x() + bcWidget->width() + 20;
            int targetY = widgetPos.y();
            return QPoint(targetX, targetY);
        }
    }
    return QPoint(100, 100);
}

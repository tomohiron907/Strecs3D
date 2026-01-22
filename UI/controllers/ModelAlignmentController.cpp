#include "ModelAlignmentController.h"
#include "../../core/application/ApplicationController.h"
#include "../../core/application/MainWindowUIAdapter.h"
#include "../visualization/VisualizationManager.h"
#include "../widgets/process/ProcessManagerWidget.h"
#include "../widgets/process/steps/BoundaryConditionStepWidget.h"
#include "../widgets/process/SelectBedSurfaceDialog.h"
#include "../../core/processing/StepReader.h"

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <gp_Vec.hxx>
#include <gp.hxx>

#include <QWidget>
#include <QPoint>
#include <QDebug>
#include <cmath>

ModelAlignmentController::ModelAlignmentController(
    ApplicationController* appController,
    MainWindowUIAdapter* uiAdapter,
    ProcessManagerWidget* processManager,
    QWidget* parent)
    : QObject(parent)
    , appController_(appController)
    , uiAdapter_(uiAdapter)
    , processManager_(processManager)
    , parentWidget_(parent)
{
}

void ModelAlignmentController::showBedSurfaceSelectionDialog()
{
    if (!uiAdapter_ || !uiAdapter_->getVisualizationManager()) return;

    SelectBedSurfaceDialog* dialog = new SelectBedSurfaceDialog(parentWidget_);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->move(calculateDialogPosition());

    dialog->setVisualizationManager(uiAdapter_->getVisualizationManager());

    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        int faceId = dialog->getSelectedFaceId();
        if (faceId > 0) {
            alignModelToFace(faceId);
        }
    });

    dialog->show();
}

void ModelAlignmentController::alignModelToFace(int faceId)
{
    if (!appController_ || !uiAdapter_ || !uiAdapter_->getVisualizationManager()) return;

    auto stepReader = uiAdapter_->getVisualizationManager()->getCurrentStepReader();
    if (!stepReader) {
        QString msg = "Error: No valid STEP reader available.";
        qDebug() << msg;
        uiAdapter_->showCriticalMessage("Error", msg);
        return;
    }

    FaceGeometry geom = stepReader->getFaceGeometry(faceId);
    if (!geom.isValid) {
        QString msg = "Error: Invalid face geometry.";
        qDebug() << msg;
        uiAdapter_->showCriticalMessage("Error", msg);
        return;
    }

    gp_Trsf finalTrsf = calculateAlignmentTransform(geom);

    qDebug() << "Applying transform to align face to bed...";
    if (appController_->applyTransformToStep(finalTrsf, uiAdapter_)) {
        appController_->transformBoundaryConditions(finalTrsf, uiAdapter_);
    }
}

gp_Trsf ModelAlignmentController::calculateAlignmentTransform(const FaceGeometry& geom)
{
    gp_Pnt center(geom.centerX, geom.centerY, geom.centerZ);
    gp_Dir normal(geom.normalX, geom.normalY, geom.normalZ);

    // Transformation 1: Translation to bring center to origin
    gp_Trsf translation;
    translation.SetTranslation(center, gp::Origin());

    // Transformation 2: Rotation to align normal to -Z
    gp_Trsf rotation;
    gp_Dir targetNormal(0, 0, -1);

    if (!normal.IsParallel(targetNormal, 1e-6)) {
        gp_Ax1 rotationAxis(gp::Origin(), normal.Crossed(targetNormal));
        double angle = normal.Angle(targetNormal);
        rotation.SetRotation(rotationAxis, angle);
    } else if (normal.IsOpposite(targetNormal, 1e-6)) {
        // Normal is +Z, need 180 degree rotation around X axis
        gp_Ax1 rotationAxis(gp::Origin(), gp::DX());
        rotation.SetRotation(rotationAxis, M_PI);
    }

    // Composite transform: R * T (translate first, then rotate)
    return rotation * translation;
}

QPoint ModelAlignmentController::calculateDialogPosition()
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

#include "SceneRenderer.h"
#include "SceneDataController.h"
#include "../../core/processing/VtkProcessor.h"
#include "../mainwindowui.h"
#include "../widgets/ObjectDisplayOptionsWidget.h"
#include <QMessageBox>
#include <QString>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <iostream>

SceneRenderer::SceneRenderer(MainWindowUI* ui) : QObject(), ui_(ui) {}

SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::renderObjects(const std::vector<ObjectInfo>& objectList) {
    if (!ui_ || !ui_->getRenderer()) return;
    
    clearRenderer();
    
    for (const auto& obj : objectList) {
        if (obj.visible && obj.actor) {
            obj.actor->SetVisibility(1);
            obj.actor->GetProperty()->SetOpacity(obj.opacity);
            ui_->getRenderer()->AddActor(obj.actor);
        }
    }
    
    render();
}

void SceneRenderer::addActorToRenderer(vtkSmartPointer<vtkActor> actor) {
    if (ui_ && ui_->getRenderer() && actor) {
        ui_->getRenderer()->AddActor(actor);
    }
}

void SceneRenderer::removeActorFromRenderer(vtkSmartPointer<vtkActor> actor) {
    if (ui_ && ui_->getRenderer() && actor) {
        ui_->getRenderer()->RemoveActor(actor);
    }
}

void SceneRenderer::clearRenderer() {
    if (ui_ && ui_->getRenderer()) {
        ui_->getRenderer()->RemoveAllViewProps();
    }
}

void SceneRenderer::render() {
    if (ui_ && ui_->getVtkWidget() && ui_->getVtkWidget()->renderWindow()) {
        ui_->getVtkWidget()->renderWindow()->Render();
    }
}

void SceneRenderer::resetCamera() {
    if (ui_ && ui_->getRenderer()) {
        ui_->getRenderer()->ResetCamera();
    }
}

void SceneRenderer::setupScalarBar(VtkProcessor* vtkProcessor) {
    if (!ui_ || !vtkProcessor) return;
    
    auto lookupTable = vtkProcessor->getCurrentLookupTable();
    if (lookupTable) {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetLookupTable(lookupTable);
        scalarBar->SetTitle(vtkProcessor->getDetectedStressLabel().c_str());
        scalarBar->GetLabelTextProperty()->SetColor(1, 1, 1);
        scalarBar->GetTitleTextProperty()->SetColor(1, 1, 1);
        scalarBar->SetNumberOfLabels(5);
        scalarBar->SetOrientationToHorizontal();
        scalarBar->SetWidth(0.5);
        scalarBar->SetHeight(0.05);
        scalarBar->SetPosition(0.5, 0.05);
        ui_->getRenderer()->AddActor2D(scalarBar);
    }
}

std::vector<ObjectDisplayOptionsWidget*> SceneRenderer::fetchMeshDisplayWidgets() {
    return {
        ui_->getDividedMeshWidget1(),
        ui_->getDividedMeshWidget2(),
        ui_->getDividedMeshWidget3(),
        ui_->getDividedMeshWidget4()
    };
}

void SceneRenderer::updateWidgetAndConnectSignals(
    const std::vector<ObjectDisplayOptionsWidget*>& widgets,
    int& widgetIndex,
    const std::string& filename,
    const std::string& filePath) {
    
    if (widgetIndex < widgets.size() && widgets[widgetIndex]) {
        widgets[widgetIndex]->setFileName(QString::fromStdString(filename));
        connectWidgetSignals(widgets[widgetIndex], filePath);
        widgetIndex++;
    }
}

void SceneRenderer::connectWidgetSignals(ObjectDisplayOptionsWidget* widget, const std::string& filePath) {
    connect(widget, &ObjectDisplayOptionsWidget::visibilityToggled,
            [this, filePath](bool visible) {
                emit objectVisibilityChanged(filePath, visible);
            });
    
    connect(widget, &ObjectDisplayOptionsWidget::opacityChanged,
            [this, filePath](double opacity) {
                emit objectOpacityChanged(filePath, opacity);
            });
}

void SceneRenderer::handleStlFileLoadError(const std::exception& e, QWidget* parent) {
    std::cerr << "Error loading STL files: " << e.what() << std::endl;
    if (parent) {
        QMessageBox::critical(parent, "Error", QString("Failed to load STL files: ") + e.what());
    }
}
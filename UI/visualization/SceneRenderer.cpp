#include "SceneRenderer.h"
#include "VisualizationManager.h"
#include "TurntableInteractorStyle.h"
#include "StepFacePickerStyle.h"
#include "../../core/processing/VtkProcessor.h"
#include "../mainwindowui.h"
#include "../widgets/ObjectDisplayOptionsWidget.h"
#include <QMessageBox>
#include <QString>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkActorCollection.h>
#include <vtkActor2DCollection.h>
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTextActor.h>
#include <iostream>

SceneRenderer::SceneRenderer(MainWindowUI* ui) : QObject(), ui_(ui) {
    turntableStyle_ = vtkSmartPointer<TurntableInteractorStyle>::New();
    stepFacePickerStyle_ = vtkSmartPointer<StepFacePickerStyle>::New();
    enableTurntableMode(true);
}

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
        auto renderer = ui_->getRenderer();
        vtkActorCollection* actors = renderer->GetActors();

        if (actors) {
            actors->InitTraversal();
            vtkActor* actor;
            std::vector<vtkActor*> actorsToRemove;

            while ((actor = actors->GetNextActor()) != nullptr) {
                actorsToRemove.push_back(actor);
            }

            for (auto* actorToRemove : actorsToRemove) {
                renderer->RemoveActor(actorToRemove);
            }
        }

        // Remove 2D actors (scalar bars, etc.) but preserve StepFacePickerStyle label
        vtkActor2DCollection* actors2D = renderer->GetActors2D();
        if (actors2D) {
            vtkTextActor* labelToPreserve = nullptr;
            if (stepFacePickerStyle_) {
                labelToPreserve = stepFacePickerStyle_->GetLabel();
            }

            actors2D->InitTraversal();
            vtkActor2D* actor2D;
            std::vector<vtkActor2D*> actors2DToRemove;

            while ((actor2D = actors2D->GetNextActor2D()) != nullptr) {
                if (actor2D != labelToPreserve) {
                    actors2DToRemove.push_back(actor2D);
                }
            }

            for (auto* actor2DToRemove : actors2DToRemove) {
                renderer->RemoveActor2D(actor2DToRemove);
            }
        }
    }
}

void SceneRenderer::render() {
    if (ui_ && ui_->getVtkWidget() && ui_->getVtkWidget()->renderWindow()) {
        ui_->getVtkWidget()->renderWindow()->Render();
    }
}

void SceneRenderer::resetCamera() {
    if (ui_ && ui_->getRenderer()) {
        auto camera = ui_->getRenderer()->GetActiveCamera();
        if (camera) {
            // Set camera to view from diagonal top
            camera->SetPosition(150, -150, 100);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 0, 1);
            ui_->getRenderer()->ResetCameraClippingRange();
        }
    }
}

void SceneRenderer::setCameraPosition(double x, double y, double z) {
    if (ui_ && ui_->getRenderer()) {
        auto camera = ui_->getRenderer()->GetActiveCamera();
        if (camera) {
            camera->SetPosition(x, y, z);
            ui_->getRenderer()->ResetCameraClippingRange();
        }
    }
}

void SceneRenderer::setCameraFocalPoint(double x, double y, double z) {
    if (ui_ && ui_->getRenderer()) {
        auto camera = ui_->getRenderer()->GetActiveCamera();
        if (camera) {
            camera->SetFocalPoint(x, y, z);
        }
    }
}

void SceneRenderer::enableTurntableMode(bool enable) {
    if (!ui_ || !ui_->getVtkWidget()) return;

    auto interactor = ui_->getVtkWidget()->interactor();
    if (!interactor) return;

    if (enable) {
        interactor->SetInteractorStyle(turntableStyle_);
    } else {
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> defaultStyle =
            vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        interactor->SetInteractorStyle(defaultStyle);
    }
}

void SceneRenderer::setTurntableRotationSpeed(double speed) {
    if (turntableStyle_) {
        turntableStyle_->SetRotationSpeed(speed);
    }
}

void SceneRenderer::setupStepFacePicker(const std::vector<vtkSmartPointer<vtkActor>>& faceActors) {
    if (!ui_ || !ui_->getVtkWidget()) return;

    auto interactor = ui_->getVtkWidget()->interactor();
    if (!interactor || !ui_->getRenderer()) return;

    stepFacePickerStyle_->SetFaceActors(faceActors);
    stepFacePickerStyle_->SetRenderer(ui_->getRenderer());
    stepFacePickerStyle_->SetOnFaceClicked([this](int faceId, const double* normal) {
        emit faceClicked(faceId, normal[0], normal[1], normal[2]);
    });
    interactor->SetInteractorStyle(stepFacePickerStyle_);
}

void SceneRenderer::setupScalarBar(VtkProcessor* vtkProcessor) {
    if (!ui_ || !vtkProcessor) return;

    auto lookupTable = vtkProcessor->getCurrentLookupTable();
    if (lookupTable) {
        scalarBarActor_ = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBarActor_->SetLookupTable(lookupTable);
        scalarBarActor_->SetTitle(vtkProcessor->getDetectedStressLabel().c_str());
        scalarBarActor_->GetLabelTextProperty()->SetColor(1, 1, 1);
        scalarBarActor_->GetTitleTextProperty()->SetColor(1, 1, 1);
        scalarBarActor_->SetNumberOfLabels(5);
        scalarBarActor_->SetOrientationToHorizontal();
        scalarBarActor_->SetWidth(0.5);
        scalarBarActor_->SetHeight(0.05);
        scalarBarActor_->SetPosition(0.5, 0.05);
        ui_->getRenderer()->AddActor2D(scalarBarActor_);
    }
}

void SceneRenderer::removeScalarBar() {
    if (ui_ && ui_->getRenderer() && scalarBarActor_) {
        ui_->getRenderer()->RemoveActor2D(scalarBarActor_);
        scalarBarActor_ = nullptr;
    }
}

std::vector<ObjectDisplayOptionsWidget*> SceneRenderer::fetchMeshDisplayWidgets() {
    // DisplayOptionsContainerが削除されたため、空のベクターを返す
    // メッシュの表示機能自体は保持されるが、UIウィジェットとの接続は行わない
    return {};
}

void SceneRenderer::updateWidgetAndConnectSignals(
    const std::vector<ObjectDisplayOptionsWidget*>& widgets,
    int& widgetIndex,
    const std::string& filename,
    const std::string& filePath)
{
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
    QMessageBox::critical(parent, "エラー",
                          QString("STLファイルの読み込みに失敗しました:\n%1")
                              .arg(QString::fromStdString(e.what())));
}

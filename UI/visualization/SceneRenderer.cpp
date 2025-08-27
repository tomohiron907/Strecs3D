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
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkLine.h>
#include <vtkActorCollection.h>
#include <vtkActor2DCollection.h>
#include <iostream>

SceneRenderer::SceneRenderer(MainWindowUI* ui) : QObject(), ui_(ui) {
    createGrid();
}

SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::renderObjects(const std::vector<ObjectInfo>& objectList) {
    if (!ui_ || !ui_->getRenderer()) return;
    
    clearRenderer();
    
    // グリッドが確実に表示されているかチェック
    if (gridActor_ && ui_->getRenderer()->GetActors()->IsItemPresent(gridActor_) == 0) {
        ui_->getRenderer()->AddActor(gridActor_);
    }
    
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
        // グリッド以外のすべてのアクターを削除
        auto renderer = ui_->getRenderer();
        vtkActorCollection* actors = renderer->GetActors();
        
        if (actors) {
            actors->InitTraversal();
            vtkActor* actor;
            std::vector<vtkActor*> actorsToRemove;
            
            while ((actor = actors->GetNextActor()) != nullptr) {
                // グリッドアクター以外を削除対象に追加
                if (actor != gridActor_) {
                    actorsToRemove.push_back(actor);
                }
            }
            
            // 削除対象のアクターを削除
            for (auto* actorToRemove : actorsToRemove) {
                renderer->RemoveActor(actorToRemove);
            }
        }
        
        // 2Dアクター（スカラーバーなど）も削除
        vtkActor2DCollection* actors2D = renderer->GetActors2D();
        if (actors2D) {
            actors2D->InitTraversal();
            vtkActor2D* actor2D;
            std::vector<vtkActor2D*> actors2DToRemove;
            
            while ((actor2D = actors2D->GetNextActor2D()) != nullptr) {
                actors2DToRemove.push_back(actor2D);
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

void SceneRenderer::showGrid(bool show) {
    if (!ui_ || !ui_->getRenderer()) return;
    
    if (show && gridActor_) {
        gridActor_->SetVisibility(1);
        ui_->getRenderer()->AddActor(gridActor_);
        render();
    } else if (!show && gridActor_) {
        hideGrid();
    }
}

void SceneRenderer::hideGrid() {
    if (!ui_ || !ui_->getRenderer() || !gridActor_) return;
    
    gridActor_->SetVisibility(0);
    ui_->getRenderer()->RemoveActor(gridActor_);
    render();
}

void SceneRenderer::createGrid() {
    const int gridSize = 400;
    const int halfGridSize = gridSize / 2;
    const int gridInterval = 10;
    const int numLines = gridSize / gridInterval + 1; // -halfGridSizeからhalfGridSizeまで10間隔で41本の線
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    
    int pointId = 0;
    
    // X方向の線（Y軸に平行）
    for (int i = 0; i < numLines; i++) {
        double x = -halfGridSize + (i * gridInterval);
        
        // 線の開始点と終了点
        points->InsertNextPoint(x, -halfGridSize, 0.0);
        points->InsertNextPoint(x, halfGridSize, 0.0);
        
        // 線を作成
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, pointId);
        line->GetPointIds()->SetId(1, pointId + 1);
        lines->InsertNextCell(line);
        
        pointId += 2;
    }
    
    // Y方向の線（X軸に平行）
    for (int i = 0; i < numLines; i++) {
        double y = -halfGridSize + (i * gridInterval);
        
        // 線の開始点と終了点
        points->InsertNextPoint(-halfGridSize, y, 0.0);
        points->InsertNextPoint(halfGridSize, y, 0.0);
        
        // 線を作成
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, pointId);
        line->GetPointIds()->SetId(1, pointId + 1);
        lines->InsertNextCell(line);
        
        pointId += 2;
    }
    
    // PolyDataを作成
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);
    
    // マッパーを作成
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    
    // アクターを作成
    gridActor_ = vtkSmartPointer<vtkActor>::New();
    gridActor_->SetMapper(mapper);
    
    // グリッドの外観を設定（薄いグレー）
    gridActor_->GetProperty()->SetColor(0.5, 0.5, 0.5);
    gridActor_->GetProperty()->SetLineWidth(1.0);
    gridActor_->GetProperty()->SetOpacity(0.3);
    
    // デフォルトでグリッドを表示
    showGrid(true);
}

void SceneRenderer::handleStlFileLoadError(const std::exception& e, QWidget* parent) {
    std::cerr << "Error loading STL files: " << e.what() << std::endl;
    if (parent) {
        QMessageBox::critical(parent, "Error", QString("Failed to load STL files: ") + e.what());
    }
}
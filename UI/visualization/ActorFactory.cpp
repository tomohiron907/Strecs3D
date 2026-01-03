#include "ActorFactory.h"
#include "../../core/processing/VtkProcessor.h"
#include "../../core/processing/StepReader.h"
#include "../../core/ui/UIState.h"
#include "../ColorManager.h"
#include <vtkProperty.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkLine.h>
#include <vtkSphereSource.h>
#include <QColor>
#include <regex>
#include <algorithm>

// --- VTK File Actors ---
vtkSmartPointer<vtkActor> ActorFactory::createVtkActor(
    const std::string& vtkFile,
    VtkProcessor* vtkProcessor)
{
    if (!vtkProcessor) return nullptr;
    return vtkProcessor->getVtuActor(vtkFile);
}

// --- STL File Actors ---
vtkSmartPointer<vtkActor> ActorFactory::createStlActorWithStress(
    const std::string& stlFile,
    double stressValue,
    double minStress,
    double maxStress,
    VtkProcessor* vtkProcessor)
{
    if (!vtkProcessor) return nullptr;
    return vtkProcessor->getColoredStlActorByStress(stlFile, stressValue, minStress, maxStress);
}

// --- STEP File Actors ---
ActorFactory::StepActors ActorFactory::createStepActors(const std::string& stepFile) {
    StepActors result;

    StepReader reader;
    if (!reader.readStepFile(stepFile)) {
        return result;  // Empty
    }

    result.faceActors = reader.getFaceActors();
    result.edgesActor = reader.getEdgesActor();

    return result;
}

// --- Divided Mesh Actors ---
std::vector<vtkSmartPointer<vtkActor>> ActorFactory::createDividedMeshActors(
    const std::vector<std::pair<std::filesystem::path, int>>& stlFiles,
    VtkProcessor* vtkProcessor,
    double minStress,
    double maxStress,
    const std::vector<MeshInfo>& meshInfos,
    UIState* uiState)
{
    std::vector<vtkSmartPointer<vtkActor>> actors;

    for (const auto& [filePath, meshNumber] : stlFiles) {
        DividedMeshActorParams params;
        params.filePath = filePath;
        params.meshNumber = meshNumber;
        params.minStress = minStress;
        params.maxStress = maxStress;
        params.uiState = uiState;
        params.vtkProcessor = vtkProcessor;

        // Parse stress range from filename (not full path)
        std::string filename = filePath.filename().string();
        auto stressRange = parseStressRange(filename, meshInfos);
        if (stressRange) {
            params.stressRange = *stressRange;
        } else {
            params.stressRange = {0.0, 0.0};
        }

        auto actor = createSingleDividedMeshActor(params);
        if (actor) {
            actors.push_back(actor);
        }
    }

    return actors;
}

// --- Scene Element Actors ---
vtkSmartPointer<vtkActor> ActorFactory::createGridActor(
    int gridSize,
    int gridInterval,
    double opacity)
{
    const int halfGridSize = gridSize / 2;
    const int numLines = gridSize / gridInterval + 1;

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    int pointId = 0;

    // X direction lines (parallel to Y axis)
    for (int i = 0; i < numLines; i++) {
        double x = -halfGridSize + (i * gridInterval);

        points->InsertNextPoint(x, -halfGridSize, 0.0);
        points->InsertNextPoint(x, halfGridSize, 0.0);

        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, pointId);
        line->GetPointIds()->SetId(1, pointId + 1);
        lines->InsertNextCell(line);

        pointId += 2;
    }

    // Y direction lines (parallel to X axis)
    for (int i = 0; i < numLines; i++) {
        double y = -halfGridSize + (i * gridInterval);

        points->InsertNextPoint(-halfGridSize, y, 0.0);
        points->InsertNextPoint(halfGridSize, y, 0.0);

        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, pointId);
        line->GetPointIds()->SetId(1, pointId + 1);
        lines->InsertNextCell(line);

        pointId += 2;
    }

    // Create PolyData
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    // Create Mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    // Create Actor
    vtkSmartPointer<vtkActor> gridActor = vtkSmartPointer<vtkActor>::New();
    gridActor->SetMapper(mapper);
    gridActor->GetProperty()->SetColor(0.8, 0.8, 0.8);  // Light gray
    gridActor->GetProperty()->SetOpacity(opacity);
    gridActor->GetProperty()->SetLineWidth(1.0);

    return gridActor;
}

ActorFactory::AxisActors ActorFactory::createAxisActors(double axisLength) {
    AxisActors result;
    const double lineWidth = 2.0;

    // Create X axis (red)
    {
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(0.0, 0.0, 0.0);
        points->InsertNextPoint(axisLength, 0.0, 0.0);

        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, 0);
        line->GetPointIds()->SetId(1, 1);
        lines->InsertNextCell(line);

        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        polyData->SetLines(lines);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);

        result.xAxis = vtkSmartPointer<vtkActor>::New();
        result.xAxis->SetMapper(mapper);
        QColor xColor = ColorManager::X_AXIS_COLOR;
        result.xAxis->GetProperty()->SetColor(xColor.redF(), xColor.greenF(), xColor.blueF());
        result.xAxis->GetProperty()->SetLineWidth(lineWidth);
        result.xAxis->GetProperty()->SetOpacity(xColor.alphaF());
    }

    // Create Y axis (green)
    {
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(0.0, 0.0, 0.0);
        points->InsertNextPoint(0.0, axisLength, 0.0);

        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, 0);
        line->GetPointIds()->SetId(1, 1);
        lines->InsertNextCell(line);

        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        polyData->SetLines(lines);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);

        result.yAxis = vtkSmartPointer<vtkActor>::New();
        result.yAxis->SetMapper(mapper);
        QColor yColor = ColorManager::Y_AXIS_COLOR;
        result.yAxis->GetProperty()->SetColor(yColor.redF(), yColor.greenF(), yColor.blueF());
        result.yAxis->GetProperty()->SetLineWidth(lineWidth);
        result.yAxis->GetProperty()->SetOpacity(yColor.alphaF());
    }

    // Create Z axis (blue)
    {
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(0.0, 0.0, 0.0);
        points->InsertNextPoint(0.0, 0.0, axisLength);

        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, 0);
        line->GetPointIds()->SetId(1, 1);
        lines->InsertNextCell(line);

        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        polyData->SetLines(lines);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);

        result.zAxis = vtkSmartPointer<vtkActor>::New();
        result.zAxis->SetMapper(mapper);
        QColor zColor = ColorManager::Z_AXIS_COLOR;
        result.zAxis->GetProperty()->SetColor(zColor.redF(), zColor.greenF(), zColor.blueF());
        result.zAxis->GetProperty()->SetLineWidth(lineWidth);
        result.zAxis->GetProperty()->SetOpacity(zColor.alphaF());
    }

    // Create origin marker (white sphere)
    {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(0.0, 0.0, 0.0);
        sphere->SetRadius(1.0);
        sphere->SetPhiResolution(10);
        sphere->SetThetaResolution(10);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(sphere->GetOutputPort());

        result.origin = vtkSmartPointer<vtkActor>::New();
        result.origin->SetMapper(mapper);
        QColor originColor = ColorManager::ORIGIN_COLOR;
        result.origin->GetProperty()->SetColor(originColor.redF(), originColor.greenF(), originColor.blueF());
        result.origin->GetProperty()->SetOpacity(originColor.alphaF());
    }

    return result;
}

// --- Actor Update Methods ---
void ActorFactory::updateActorColor(vtkActor* actor, double r, double g, double b) {
    if (actor) {
        actor->GetProperty()->SetColor(r, g, b);
    }
}

void ActorFactory::updateActorOpacity(vtkActor* actor, double opacity) {
    if (actor) {
        actor->GetProperty()->SetOpacity(opacity);
    }
}

void ActorFactory::updateActorVisibility(vtkActor* actor, bool visible) {
    if (actor) {
        actor->SetVisibility(visible ? 1 : 0);
    }
}

// --- Private Helper Methods ---
void ActorFactory::calculateStressColor(
    double stressValue,
    double minStress,
    double maxStress,
    double& r, double& g, double& b)
{
    // Normalize stress value to [0, 1]
    double normalized = 0.0;
    if (maxStress > minStress) {
        normalized = (stressValue - minStress) / (maxStress - minStress);
        normalized = std::clamp(normalized, 0.0, 1.0);
    }

    calculateGradientColor(normalized, r, g, b);
}

void ActorFactory::calculateGradientColor(
    double normalizedPos,
    double& r, double& g, double& b)
{
    // Blue (low) -> Cyan -> Green -> Yellow -> Red (high)
    if (normalizedPos < 0.25) {
        // Blue to Cyan
        double t = normalizedPos / 0.25;
        r = 0.0;
        g = t;
        b = 1.0;
    } else if (normalizedPos < 0.5) {
        // Cyan to Green
        double t = (normalizedPos - 0.25) / 0.25;
        r = 0.0;
        g = 1.0;
        b = 1.0 - t;
    } else if (normalizedPos < 0.75) {
        // Green to Yellow
        double t = (normalizedPos - 0.5) / 0.25;
        r = t;
        g = 1.0;
        b = 0.0;
    } else {
        // Yellow to Red
        double t = (normalizedPos - 0.75) / 0.25;
        r = 1.0;
        g = 1.0 - t;
        b = 0.0;
    }
}

std::optional<std::pair<double, double>> ActorFactory::parseStressRange(
    const std::string& filename,
    const std::vector<MeshInfo>& meshInfos)
{
    // Extract mesh number from filename using regex
    // Pattern: modifierMesh<number>.stl
    std::regex pattern(R"(^modifierMesh(\d+)\.stl$)");
    std::smatch match;

    if (std::regex_search(filename, match, pattern)) {
        try {
            int meshID = std::stoi(match[1].str());
            // Look up stress values from meshInfos
            for (const auto& meshInfo : meshInfos) {
                if (meshInfo.meshID == meshID) {
                    return std::make_pair(
                        static_cast<double>(meshInfo.stressMin),
                        static_cast<double>(meshInfo.stressMax)
                    );
                }
            }
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

vtkSmartPointer<vtkActor> ActorFactory::createSingleDividedMeshActor(
    const DividedMeshActorParams& params)
{
    if (!params.vtkProcessor) return nullptr;

    // Calculate average stress for this mesh
    double avgStress = (params.stressRange.first + params.stressRange.second) / 2.0;

    // Create actor with stress-based coloring
    return createStlActorWithStress(
        params.filePath.string(),
        avgStress,
        params.minStress,
        params.maxStress,
        params.vtkProcessor
    );
}

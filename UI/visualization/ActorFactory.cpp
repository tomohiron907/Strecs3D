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
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkConeSource.h>
#include <vtkPlaneSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkMath.h>
#include <QColor>
#include <regex>
#include <algorithm>
#include <cmath>
#include "../../core/types/BoundaryCondition.h"

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

// --- Boundary Condition Actors ---
ActorFactory::BoundaryConditionActors ActorFactory::createBoundaryConditionActors(
    const BoundaryCondition& condition,
    const StepReader* stepReader)
{
    BoundaryConditionActors result;

    if (!stepReader || !stepReader->isValid()) {
        return result;
    }

    // Constraint Visualization
    for (const auto& constraint : condition.constraints) {
        FaceGeometry geom = stepReader->getFaceGeometry(constraint.surface_id);

        if (!geom.isValid) {
            continue;
        }

        auto actor = createConstraintActor(geom.centerX, geom.centerY, geom.centerZ);
        if (actor) {
            result.constraintActors.push_back(actor);
        }
    }

    // Load Visualization
    for (const auto& load : condition.loads) {
        FaceGeometry geom = stepReader->getFaceGeometry(load.surface_id);

        if (!geom.isValid) {
            continue;
        }

        auto actor = createLoadArrowActor(
            geom.centerX, geom.centerY, geom.centerZ,
            load.direction.x, load.direction.y, load.direction.z,
            geom.normalX, geom.normalY, geom.normalZ
        );

        if (actor) {
            result.loadActors.push_back(actor);
        }
    }

    return result;
}

vtkSmartPointer<vtkActor> ActorFactory::createConstraintActor(
    double centerX, double centerY, double centerZ)
{
    vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
    cube->SetXLength(CONSTRAINT_CUBE_SIZE);
    cube->SetYLength(CONSTRAINT_CUBE_SIZE);
    cube->SetZLength(CONSTRAINT_CUBE_SIZE);
    cube->SetCenter(centerX, centerY, centerZ);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cube->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Green color for constraints
    actor->GetProperty()->SetColor(0.0, 1.0, 0.0);
    actor->GetProperty()->SetOpacity(0.8);

    return actor;
}

vtkSmartPointer<vtkActor> ActorFactory::createLoadArrowActor(
    double centerX, double centerY, double centerZ,
    double dirX, double dirY, double dirZ,
    double normalX, double normalY, double normalZ)
{
    // Validate direction
    double length = std::sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    if (length < 1e-6) {
        return nullptr;
    }

    dirX /= length;
    dirY /= length;
    dirZ /= length;

    // Check dot product with normal to decide placement
    double dotProduct = normalX * dirX + normalY * dirY + normalZ * dirZ;

    // If result is tension (dot > 0), shift arrow base to face center
    if (dotProduct > 0) {
        double arrowLength = ARROW_CONE_HEIGHT + ARROW_CYLINDER_LENGTH;
        centerX += dirX * arrowLength;
        centerY += dirY * arrowLength;
        centerZ += dirZ * arrowLength;
    }

    // Create Cylinder Body
    vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    cylinder->SetRadius(ARROW_CYLINDER_RADIUS);
    cylinder->SetHeight(ARROW_CYLINDER_LENGTH);
    cylinder->SetResolution(16);

    // Place cylinder (local Y axis)
    double cylinderLocalY = -(ARROW_CONE_HEIGHT / 2.0 + ARROW_CYLINDER_LENGTH / 2.0);
    cylinder->SetCenter(0, cylinderLocalY, 0);

    // Create Cone Head
    vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
    cone->SetRadius(ARROW_CONE_RADIUS);
    cone->SetHeight(ARROW_CONE_HEIGHT);
    cone->SetResolution(16);
    cone->SetDirection(0, 1, 0);

    // Place cone (local Y axis)
    cone->SetCenter(0, -ARROW_CONE_HEIGHT / 2.0, 0);

    // Append Cylinder and Cone
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    appendFilter->AddInputConnection(cylinder->GetOutputPort());
    appendFilter->AddInputConnection(cone->GetOutputPort());
    appendFilter->Update();

    // Create Transform
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(centerX, centerY, centerZ);

    // Calculate Rotation
    double yAxis[3] = {0, 1, 0};
    double targetDir[3] = {dirX, dirY, dirZ};
    double rotationAxis[3];
    vtkMath::Cross(yAxis, targetDir, rotationAxis);
    double rotationAxisLength = vtkMath::Norm(rotationAxis);

    if (rotationAxisLength > 1e-6) {
        double angle = std::acos(vtkMath::Dot(yAxis, targetDir)) * 180.0 / vtkMath::Pi();
        vtkMath::Normalize(rotationAxis);
        transform->RotateWXYZ(angle, rotationAxis);
    } else if (vtkMath::Dot(yAxis, targetDir) < 0) {
        // Anti-parallel
        transform->RotateWXYZ(180.0, 1, 0, 0); // Or z axis
    }

    // Apply Transform
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(appendFilter->GetOutputPort());
    transformFilter->SetTransform(transform);

    // Create Actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(transformFilter->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Red color for loads
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    actor->GetProperty()->SetOpacity(0.8);

    return actor;
}

vtkSmartPointer<vtkActor> ActorFactory::createBedPreviewActor(
    const FaceGeometry& geom)
{
    if (!geom.isValid) {
        return nullptr;
    }

    // Create Plane
    vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetOrigin(-50, -50, 0);
    planeSource->SetPoint1(50, -50, 0);
    planeSource->SetPoint2(-50, 50, 0);
    planeSource->Update();

    // Create Transform
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(geom.centerX, geom.centerY, geom.centerZ);

    double normal[3] = {geom.normalX, geom.normalY, geom.normalZ};
    double zAxis[3] = {0.0, 0.0, 1.0};
    double rotationAxis[3];
    vtkMath::Cross(zAxis, normal, rotationAxis);

    double dot = vtkMath::Dot(zAxis, normal);
    // Clamp dot product to avoid acos domain error
    if (dot > 1.0) dot = 1.0;
    if (dot < -1.0) dot = -1.0;
    
    double angle = std::acos(dot) * 180.0 / vtkMath::Pi();

    if (vtkMath::Norm(rotationAxis) > 1e-6) {
         transform->RotateWXYZ(angle, rotationAxis);
    } else if (dot < 0) {
         // Anti-parallel (180 degrees)
         transform->RotateWXYZ(180, 1, 0, 0);
    }

    // Apply Transform
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(planeSource->GetOutputPort());
    transformFilter->SetTransform(transform);
    transformFilter->Update();

    // Create Actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(transformFilter->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Light Blue color, semi-transparent
    actor->GetProperty()->SetColor(0.2, 0.6, 1.0);
    actor->GetProperty()->SetOpacity(0.5);
    actor->GetProperty()->SetLighting(false);

    return actor;
}

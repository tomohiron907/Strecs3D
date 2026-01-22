#pragma once

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include <optional>

class VtkProcessor;
class StepReader;
class UIState;
struct MeshInfo;
class QColor;
struct BoundaryCondition;
struct FaceGeometry;

class ActorFactory {
public:
    ActorFactory() = default;
    ~ActorFactory() = default;

    // --- VTK File Actors ---
    vtkSmartPointer<vtkActor> createVtkActor(
        const std::string& vtkFile,
        VtkProcessor* vtkProcessor);

    // --- STL File Actors (for divided meshes only - stress-based coloring) ---
    vtkSmartPointer<vtkActor> createStlActorWithStress(
        const std::string& stlFile,
        double stressValue,
        double minStress,
        double maxStress,
        VtkProcessor* vtkProcessor);

    // --- STEP File Actors ---
    struct StepActors {
        std::vector<vtkSmartPointer<vtkActor>> faceActors;
        vtkSmartPointer<vtkActor> edgesActor;
    };

    StepActors createStepActors(const std::string& stepFile);

    // --- Divided Mesh Actors ---
    struct DividedMeshActorParams {
        std::filesystem::path filePath;
        int meshNumber;
        std::pair<double, double> stressRange;
        double minStress;
        double maxStress;
        UIState* uiState;
        VtkProcessor* vtkProcessor;
    };

    std::vector<vtkSmartPointer<vtkActor>> createDividedMeshActors(
        const std::vector<std::pair<std::filesystem::path, int>>& stlFiles,
        VtkProcessor* vtkProcessor,
        double minStress,
        double maxStress,
        const std::vector<MeshInfo>& meshInfos,
        UIState* uiState);

    // --- Scene Element Actors ---
    vtkSmartPointer<vtkActor> createGridActor(
        int gridSize = 400,
        int gridInterval = 10,
        double opacity = 0.3);

    struct AxisActors {
        vtkSmartPointer<vtkActor> xAxis;
        vtkSmartPointer<vtkActor> yAxis;
        vtkSmartPointer<vtkActor> zAxis;
        vtkSmartPointer<vtkActor> origin;
    };

    AxisActors createAxisActors(double axisLength = 50.0);

    // --- Actor Update Methods ---
    void updateActorColor(vtkActor* actor, double r, double g, double b);
    void updateActorOpacity(vtkActor* actor, double opacity);
    void updateActorVisibility(vtkActor* actor, bool visible);

private:
    // Helper methods for color calculation
    void calculateStressColor(
        double stressValue,
        double minStress,
        double maxStress,
        double& r, double& g, double& b);

    void calculateGradientColor(
        double normalizedPos,
        double& r, double& g, double& b);

    // Helper for divided mesh stress parsing
    std::optional<std::pair<double, double>> parseStressRange(
        const std::string& filename,
        const std::vector<MeshInfo>& meshInfos);

    // Helper for creating single divided mesh actor
    vtkSmartPointer<vtkActor> createSingleDividedMeshActor(
        const DividedMeshActorParams& params);

public:
    // --- Boundary Condition Actors ---
    struct BoundaryConditionActors {
        std::vector<vtkSmartPointer<vtkActor>> constraintActors;
        std::vector<vtkSmartPointer<vtkActor>> loadActors;
    };

    BoundaryConditionActors createBoundaryConditionActors(
        const BoundaryCondition& condition,
        const StepReader* stepReader);

    // Individual actor creation methods (exposed for flexibility and unit testing)
    vtkSmartPointer<vtkActor> createConstraintActor(
        double centerX, double centerY, double centerZ);

    vtkSmartPointer<vtkActor> createLoadArrowActor(
        double centerX, double centerY, double centerZ,
        double dirX, double dirY, double dirZ,
        double normalX, double normalY, double normalZ);

    vtkSmartPointer<vtkActor> createBedPreviewActor(
        const FaceGeometry& geom);

private:
    static constexpr double CONSTRAINT_CUBE_SIZE = 5.0;
    static constexpr double ARROW_CYLINDER_RADIUS = 1.0;
    static constexpr double ARROW_CYLINDER_LENGTH = 20.0;
    static constexpr double ARROW_CONE_RADIUS = 2.0;
    static constexpr double ARROW_CONE_HEIGHT = 5.0;
};

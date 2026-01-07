#pragma once

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vector>
#include "../../core/types/BoundaryCondition.h"
#include "../../core/processing/StepReader.h"

struct BoundaryConditionActors {
    std::vector<vtkSmartPointer<vtkActor>> constraintActors;
    std::vector<vtkSmartPointer<vtkActor>> loadActors;
};

class BoundaryConditionVisualizer {
public:
    BoundaryConditionVisualizer();
    ~BoundaryConditionVisualizer() = default;

    BoundaryConditionActors createBoundaryConditionActors(
        const BoundaryCondition& condition,
        const StepReader* stepReader);

private:
    vtkSmartPointer<vtkActor> createConstraintActor(
        double centerX, double centerY, double centerZ);

    vtkSmartPointer<vtkActor> createLoadArrowActor(
        double centerX, double centerY, double centerZ,
        double dirX, double dirY, double dirZ,
        double normalX, double normalY, double normalZ);

    static constexpr double CONSTRAINT_CUBE_SIZE = 5.0;
    static constexpr double ARROW_CYLINDER_RADIUS = 1.0;
    static constexpr double ARROW_CYLINDER_LENGTH = 20.0;
    static constexpr double ARROW_CONE_RADIUS = 2.0;
    static constexpr double ARROW_CONE_HEIGHT = 5.0;
};

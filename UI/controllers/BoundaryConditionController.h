#pragma once

#include <QObject>
#include <QString>
#include <QPoint>

class UIState;
class VisualizationManager;
class ProcessManagerWidget;
class QWidget;
struct BoundaryCondition;

/**
 * @brief Controller for managing boundary condition operations
 *
 * This controller handles all boundary condition related operations including:
 * - Adding constraints and loads via dialogs
 * - Handling face click/double-click events
 * - Updating boundary condition visualization
 */
class BoundaryConditionController : public QObject {
    Q_OBJECT
public:
    explicit BoundaryConditionController(
        UIState* uiState,
        VisualizationManager* vizManager,
        ProcessManagerWidget* processManager,
        QWidget* parent = nullptr);
    ~BoundaryConditionController() = default;

    /**
     * @brief Show dialog to add a new constraint condition
     */
    void showAddConstraintDialog();

    /**
     * @brief Show dialog to add a new load condition
     */
    void showAddLoadDialog();

    /**
     * @brief Handle face click event (currently a no-op, selection handled elsewhere)
     */
    void handleFaceClicked(int faceId, double nx, double ny, double nz);

    /**
     * @brief Handle face double-click event to update selected constraint/load
     */
    void handleFaceDoubleClicked(int faceId, double nx, double ny, double nz);

    /**
     * @brief Update boundary condition visualization in the 3D view
     */
    void updateVisualization();

private:
    /**
     * @brief Generate a unique name with the given prefix
     */
    QString generateUniqueName(const QString& prefix, const BoundaryCondition& bc, bool isConstraint);

    /**
     * @brief Calculate dialog position relative to boundary condition step widget
     */
    QPoint calculateDialogPosition();

    UIState* uiState_;
    VisualizationManager* vizManager_;
    ProcessManagerWidget* processManager_;
    QWidget* parentWidget_;
};

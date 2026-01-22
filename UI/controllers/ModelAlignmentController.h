#pragma once

#include <QObject>
#include <gp_Trsf.hxx>

class ApplicationController;
class MainWindowUIAdapter;
class ProcessManagerWidget;
class QWidget;
struct FaceGeometry;

/**
 * @brief Controller for model alignment operations
 *
 * This controller handles:
 * - Showing bed surface selection dialog
 * - Aligning model to selected bed surface
 */
class ModelAlignmentController : public QObject {
    Q_OBJECT
public:
    explicit ModelAlignmentController(
        ApplicationController* appController,
        MainWindowUIAdapter* uiAdapter,
        ProcessManagerWidget* processManager,
        QWidget* parent = nullptr);
    ~ModelAlignmentController() = default;

    /**
     * @brief Show dialog to select bed surface for alignment
     */
    void showBedSurfaceSelectionDialog();

private:
    /**
     * @brief Align model to the specified face
     */
    void alignModelToFace(int faceId);

    /**
     * @brief Calculate the transformation matrix to align face to bed
     */
    gp_Trsf calculateAlignmentTransform(const FaceGeometry& geom);

    /**
     * @brief Calculate dialog position relative to boundary condition step widget
     */
    QPoint calculateDialogPosition();

    ApplicationController* appController_;
    MainWindowUIAdapter* uiAdapter_;
    ProcessManagerWidget* processManager_;
    QWidget* parentWidget_;
};

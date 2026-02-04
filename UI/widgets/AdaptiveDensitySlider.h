#pragma once
#include <QWidget>
#include <vector>
#include <QLineEdit>
#include "../../core/types/StressDensityMapping.h"

class AdaptiveDensitySlider : public QWidget {
    Q_OBJECT
public:
    explicit AdaptiveDensitySlider(QWidget* parent = nullptr);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    std::vector<int> handlePositions() const;
    void setStressRange(double minStress, double maxStress);
    void setOriginalStressRange(double minStress, double maxStress);
    std::vector<double> regionPercents() const;
    void setRegionPercents(const std::vector<double>& percents);
    std::vector<StressDensityMapping> stressDensityMappings() const;
    std::vector<int> stressThresholds() const;
    std::vector<QColor> getRegionColors() const;
    int countMaxDensityRegions() const;

    // Region count management
    void setRegionCount(int count);
    int regionCount() const { return m_regionCount; }

    // Adaptive density slider specific
    static constexpr int VOLUME_DIVISIONS = 20;
    static constexpr int MIN_REGION_HEIGHT = 2;
    static constexpr int MAX_REGION_HEIGHT = 100;

    void setVolumeFractions(const std::vector<double>& fractions);
    const std::vector<double>& volumeFractions() const { return m_volumeFractions; }

signals:
    void handlePositionsChanged(const std::vector<int>& positions);
    void regionPercentsChanged(const std::vector<double>& percents);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // Layout constants
    static constexpr int SLIDER_WIDTH = 30;
    static constexpr int GRADIENT_WIDTH = 30;
    static constexpr int GRADIENT_GAP = 20;
    static constexpr int LABEL_EXTRA_WIDTH = 28;
    static constexpr int LABEL_GAP = 16;
    static constexpr int TRIANGLE_SIZE = 13;
    static constexpr int PERCENT_EDIT_GAP = 20;
    static constexpr int VERTICAL_LABEL_DISTANCE = 70;
    static constexpr int RIGHT_LABEL_DISTANCE = 70;

    // Helper structure for slider bounds
    struct SliderBounds {
        int left;
        int right;
        int top;
        int bottom;
        int gradLeft;
        int gradRight;
    };

    SliderBounds getSliderBounds() const;
    std::vector<int> getRegionPositions() const;
    double yToStress(int y) const;
    int stressToY(double stress) const;
    int calculateDensityFromStress(double stress) const;

    // Adaptive specific
    void updateRegionBoundaries();
    std::vector<double> m_volumeFractions;
    std::vector<int> m_regionBoundaries;

    // Drawing helper functions
    void drawGradientBar(QPainter& painter, const SliderBounds& bounds);
    void drawStressLabels(QPainter& painter, const SliderBounds& bounds);
    void drawSliderBody(QPainter& painter, const SliderBounds& bounds);
    void drawRegions(QPainter& painter, const SliderBounds& bounds);
    void drawHandles(QPainter& painter, const SliderBounds& bounds);
    void drawAxisLabels(QPainter& painter, const SliderBounds& bounds);

    std::vector<int> m_handles;
    int m_draggedHandle = -1;
    int m_handleRadius = 8;
    int m_margin = 20;
    int m_minDistance = 20;
    int handleAtPosition(const QPoint& pos) const;
    void clampHandles();
    void updatePercentEditPositions();
    void onPercentEditChanged();
    double m_minStress = 0.0;
    double m_maxStress = 1.0;
    double m_originalMinStress = 0.0;
    double m_originalMaxStress = 1.0;
    std::vector<QLineEdit*> m_percentEdits;
    std::vector<double> m_regionPercents;
    std::vector<StressDensityMapping> m_stressDensityMappings;
    void updateStressDensityMappings();
    void updateInitialHandles();
    void rebuildForRegionCount(int count);
    int m_regionCount;
    int handleCount() const { return m_regionCount - 1; }
};

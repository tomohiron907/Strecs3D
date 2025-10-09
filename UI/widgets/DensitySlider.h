#pragma once
#include <QWidget>
#include <vector>
#include <QLineEdit>
#include "../../core/types/StressDensityMapping.h"

class DensitySlider : public QWidget {
    Q_OBJECT
public:
    explicit DensitySlider(QWidget* parent = nullptr);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    std::vector<int> handlePositions() const;
    void setStressRange(double minStress, double maxStress);
    void setOriginalStressRange(double minStress, double maxStress);
    std::vector<double> regionPercents() const;
    void setRegionPercents(const std::vector<double>& percents);
    std::vector<StressDensityMapping> stressDensityMappings() const;
    std::vector<int> stressThresholds() const;

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
    std::vector<int> m_handles; // 3つのハンドル位置（Y座標）
    int m_draggedHandle = -1;
    int m_handleRadius = 8;
    int m_margin = 20;
    int m_minDistance = 20; // ハンドル間の最小距離
    int handleAtPosition(const QPoint& pos) const;
    void clampHandles();
    void updatePercentEditPositions();
    void onPercentEditChanged();
    double m_minStress = 0.0;
    double m_maxStress = 1.0;
    double m_originalMinStress = 0.0; // vtuファイルの元々の最小値
    double m_originalMaxStress = 1.0; // vtuファイルの元々の最大値
    std::vector<QLineEdit*> m_percentEdits; // 4つの領域のパーセント入力欄
    std::vector<double> m_regionPercents = {20, 40, 60, 80}; // デフォルト値
    std::vector<StressDensityMapping> m_stressDensityMappings;
    void updateStressDensityMappings();
    void updateInitialHandles();
}; 
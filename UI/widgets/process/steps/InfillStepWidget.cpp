#include "InfillStepWidget.h"
#include "../../AdaptiveDensitySlider.h"
#include "../../StressRangeWidget.h"
#include "../../Button.h"
#include "../../../core/ui/UIState.h"
#include <QVBoxLayout>
#include <QTimer>
#include <iostream>

InfillStepWidget::InfillStepWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Adaptive Density Slider
    m_densitySlider = new AdaptiveDensitySlider(this);
    layout->addWidget(m_densitySlider);

    // Stress Range Widget (Below slider)
    m_stressRangeWidget = new StressRangeWidget(this);
    layout->addWidget(m_stressRangeWidget);

    layout->addSpacing(10);

    // Process Button
    m_processButton = new Button("Process", this);
    m_processButton->setIcon(":/resources/icons/process.png");
    m_processButton->setEmphasized(true);

    connect(m_processButton, &Button::clicked, this, &InfillStepWidget::processClicked);

    layout->addWidget(m_processButton);
    layout->addStretch();
}

void InfillStepWidget::setUIState(UIState* uiState) {
    if (!uiState) return;

    // DensitySliderの変更をUIStateに反映
    connect(m_densitySlider, &AdaptiveDensitySlider::regionPercentsChanged,
            this, [this, uiState](const std::vector<double>& percents) {
                // スライダーの色をUIStateに登録
                auto colors = m_densitySlider->getRegionColors();
                uiState->setDensitySliderColors(colors);
            });

    // DensitySliderのハンドル位置変更時も色を更新
    connect(m_densitySlider, &AdaptiveDensitySlider::handlePositionsChanged,
            this, [this, uiState](const std::vector<int>& positions) {
                // スライダーの色をUIStateに登録
                auto colors = m_densitySlider->getRegionColors();
                uiState->setDensitySliderColors(colors);
            });

    // 初期色をUIStateに設定
    QTimer::singleShot(0, this, [this, uiState]() {
        auto colors = m_densitySlider->getRegionColors();
        uiState->setDensitySliderColors(colors);
    });
}

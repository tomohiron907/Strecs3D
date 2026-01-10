#include "ColorManager.h"

// Static color definitions
const QColor ColorManager::HIGH_COLOR = QColor("#FF4D00");
const QColor ColorManager::MIDDLE_COLOR = QColor("#FFFFFF");
const QColor ColorManager::LOW_COLOR = QColor("#004ca1");
const QColor ColorManager::BACKGROUND_COLOR = QColor("#1a1a1a"); 
const QColor ColorManager::BUTTON_COLOR = QColor(26, 26, 26, 180);           // 通常時
const QColor ColorManager::BUTTON_HOVER_COLOR = QColor(58, 58, 58, 200);     // ホバー時
const QColor ColorManager::BUTTON_PRESSED_COLOR = QColor(26, 26, 26, 220);   // 押下時
const QColor ColorManager::BUTTON_TEXT_COLOR = QColor(255, 255, 255, 255);   // テキスト 
const QColor ColorManager::BUTTON_EDGE_COLOR = QColor(255, 255, 255, 100); // 枠線色（白半透明） 
const QColor ColorManager::BUTTON_DISABLED_COLOR = QColor(40, 40, 40, 120);   // 無効状態背景色
const QColor ColorManager::BUTTON_DISABLED_TEXT_COLOR = QColor(120, 120, 120, 180); // 無効状態テキスト色
const QColor ColorManager::BUTTON_EMPHASIZED_COLOR = QColor(255, 255, 255, 220); // 強調表示背景色（白）
const QColor ColorManager::HEADER_COLOR = QColor("#222222");
const QColor ColorManager::HANDLE_COLOR = QColor(200, 200, 200, 255);           // ハンドル色（黒）
const QColor ColorManager::X_AXIS_COLOR = QColor(255, 77, 0, 255);               // X軸色（赤）
const QColor ColorManager::Y_AXIS_COLOR = QColor(0, 150, 0, 255);               // Y軸色（緑）
const QColor ColorManager::Z_AXIS_COLOR = QColor(0, 76, 161, 255);               // Z軸色（青）
const QColor ColorManager::ORIGIN_COLOR = QColor(255, 255, 255, 255);           // 原点色（白）
const QColor ColorManager::INPUT_BACKGROUND_COLOR = QColor("#242424");
const QColor ColorManager::INPUT_TEXT_COLOR = QColor("white");
const QColor ColorManager::INPUT_BORDER_COLOR = QColor("#4b4b4b");
const QColor ColorManager::ACCENT_COLOR = QColor("#0078D7");

// Gradient color calculation
QColor ColorManager::getGradientColor(double t) {
    // グラデーションストップの定義（staticで一度だけ初期化）
    static const struct GradStop {
        double pos;
        QColor color;
    } gradStops[] = {
        {0.0, HIGH_COLOR},   // 赤
        {0.5, MIDDLE_COLOR}, // 白
        {1.0, LOW_COLOR}     // 青
    };

    if (t <= gradStops[0].pos) return gradStops[0].color;
    if (t >= gradStops[2].pos) return gradStops[2].color;
    for (int i = 0; i < 2; ++i) {
        if (t >= gradStops[i].pos && t <= gradStops[i+1].pos) {
            double localT = (t - gradStops[i].pos) / (gradStops[i+1].pos - gradStops[i].pos);
            QColor c1 = gradStops[i].color;
            QColor c2 = gradStops[i+1].color;
            int r = c1.red()   + (c2.red()   - c1.red())   * localT;
            int g = c1.green() + (c2.green() - c1.green()) * localT;
            int b = c1.blue()  + (c2.blue()  - c1.blue())  * localT;
            return QColor(r, g, b);
        }
    }
    return QColor(); // fallback
} 
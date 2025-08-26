#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include <QColor>

class ColorManager {
public:
    // Stress gradient colors
    static const QColor HIGH_COLOR;    // #FF4D00 (red)
    static const QColor MIDDLE_COLOR;  // #FFFFFF (white)
    static const QColor LOW_COLOR;     // #004ca1 (blue)
    
    // Background color
    static const QColor BACKGROUND_COLOR; // #1a1a1a (dark gray)

    // Button colors
    static const QColor BUTTON_COLOR;         // 通常時のボタン色
    static const QColor BUTTON_HOVER_COLOR;   // ホバー時のボタン色
    static const QColor BUTTON_PRESSED_COLOR; // 押下時のボタン色
    static const QColor BUTTON_TEXT_COLOR;    // ボタンテキスト色
    static const QColor BUTTON_EDGE_COLOR;     // ボタン枠線色
    static const QColor BUTTON_DISABLED_COLOR; // 無効状態ボタン色
    static const QColor BUTTON_DISABLED_TEXT_COLOR; // 無効状態テキスト色
    static const QColor BUTTON_EMPHASIZED_COLOR; // 強調表示ボタン色
    static const QColor HEADER_COLOR;         // ヘッダー用色
    
    // Density slider handle colors
    static const QColor HANDLE_COLOR;         // ハンドル色（線と三角形）
};

#endif // COLORMANAGER_H 
#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

class StyleManager {
public:
    // Base Metrics
    static const int BASE_MARGIN; // 4px

    // Radius
    static const int RADIUS_SMALL;  // 4px (3pxも統一)
    static const int RADIUS_MEDIUM; // 8px
    static const int RADIUS_LARGE;  // 12px (10pxも統一)

    // Margins/Paddings
    static const int MARGIN_SMALL;  // 4px
    static const int MARGIN_MEDIUM; // 8px
    static const int MARGIN_LARGE;  // 16px

    // Button Style
    static const int BUTTON_RADIUS; // 4px
    static const int BUTTON_MARGIN; // 4px

    // Container Style (Container Radius = Button Radius + Container Padding)
    static const int CONTAINER_PADDING; // 8px
    static const int CONTAINER_RADIUS;  // 12px

    // Input Field Style
    static const int INPUT_HEIGHT;      // 28px
    static const int INPUT_RADIUS;      // 4px

    // Panel/Widget Style
    static const int PANEL_RADIUS;      // 8px
    static const int PANEL_MARGIN;      // 8px

    // ===== 追加定数 =====
    // Font Sizes
    static const int FONT_SIZE_SMALL;   // 11px - ヒントテキスト
    static const int FONT_SIZE_NORMAL;  // 12px - 一般テキスト
    static const int FONT_SIZE_LARGE;   // 14px - タイトル (13pxも統一)
    static const int FONT_SIZE_XLARGE;  // 17px - 特殊見出し

    // Padding
    static const int PADDING_SMALL;     // 4px - 入力内部
    static const int PADDING_MEDIUM;    // 8px - 一般 (5px, 6pxも統一)
    static const int PADDING_LARGE;     // 16px - ダイアログ (15pxも統一)

    // Button Padding
    static const int BUTTON_PADDING_V;  // 8px (5pxも統一)
    static const int BUTTON_PADDING_H;  // 16px (15pxも統一)

    // Special Radius
    static const int TAB_CONTAINER_RADIUS; // 22px

    // Heights
    static const int INPUT_HEIGHT_SMALL;   // 20px

    // Form/Dialog
    static const int FORM_SPACING;         // 12px (10pxも統一)
};

#endif // STYLEMANAGER_H

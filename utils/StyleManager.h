#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

class StyleManager {
public:
    // Base Metrics
    static const int BASE_MARGIN; // 4px

    // Radius
    static const int RADIUS_SMALL;  // 4px
    static const int RADIUS_MEDIUM; // 8px
    static const int RADIUS_LARGE;  // 12px

    // Margins/Paddings
    static const int MARGIN_SMALL;  // 4px
    static const int MARGIN_MEDIUM; // 8px
    static const int MARGIN_LARGE;  // 16px

    // Button Style
    static const int BUTTON_RADIUS; // n
    static const int BUTTON_MARGIN;

    // Container Style (Container Radius = Button Radius + Container Padding)
    static const int CONTAINER_PADDING; // m
    static const int CONTAINER_RADIUS;  // n + m

    // Input Field Style
    static const int INPUT_HEIGHT;
    static const int INPUT_RADIUS;

    // Panel/Widget Style
    static const int PANEL_RADIUS;
    static const int PANEL_MARGIN;
};

#endif // STYLEMANAGER_H

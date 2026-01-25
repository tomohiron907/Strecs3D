#include "StyleManager.h"

// Base Metrics
const int StyleManager::BASE_MARGIN = 4;

// Radius definitions
const int StyleManager::RADIUS_SMALL = 4;
const int StyleManager::RADIUS_MEDIUM = 8;
const int StyleManager::RADIUS_LARGE = 12;

// Margin definitions
const int StyleManager::MARGIN_SMALL = 4;
const int StyleManager::MARGIN_MEDIUM = 8;
const int StyleManager::MARGIN_LARGE = 16;

// Button Style (n = 4px)
const int StyleManager::BUTTON_RADIUS = StyleManager::RADIUS_SMALL;
const int StyleManager::BUTTON_MARGIN = StyleManager::MARGIN_SMALL;

// Container Style
// n px (Button Radius) の角丸要素に対して、余白 m px (Container Padding) を追加するとき、
// コンテナ要素は n + m px の角丸にする
const int StyleManager::CONTAINER_PADDING = StyleManager::MARGIN_MEDIUM; // m = 8px
const int StyleManager::CONTAINER_RADIUS = StyleManager::BUTTON_RADIUS + StyleManager::CONTAINER_PADDING; // 4 + 8 = 12px

// Input Field Style
const int StyleManager::INPUT_HEIGHT = 28;
const int StyleManager::INPUT_RADIUS = StyleManager::RADIUS_SMALL;

// Panel/Widget Style
const int StyleManager::PANEL_RADIUS = StyleManager::RADIUS_MEDIUM;
const int StyleManager::PANEL_MARGIN = StyleManager::MARGIN_MEDIUM;

#ifndef UI_PLATFORM_WINDOWUTILS_H
#define UI_PLATFORM_WINDOWUTILS_H

#include <QWidget>

namespace Platform {
namespace WindowUtils {

/**
 * @brief Customizes the window title bar according to platform guidelines.
 * For macOS: Seamless title bar.
 * For Windows: Standard behavior (or valid customization if added later).
 */
void customizeTitleBar(QWidget* window);

} // namespace WindowUtils
} // namespace Platform

#endif // UI_PLATFORM_WINDOWUTILS_H

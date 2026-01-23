#include "../WindowUtils.h"
#include <Cocoa/Cocoa.h>
#include <QWindow>

namespace Platform {
namespace WindowUtils {

void customizeTitleBar(QWidget* window) {
    // Ensure the window handle is created
    WId winId = window->winId();
    NSView* view = (NSView*)winId;
    NSWindow* nsWindow = [view window];

    if (!nsWindow) {
        return;
    }

    // Hide the title bar
    nsWindow.titlebarAppearsTransparent = YES;
    
    // Extend content into title bar area
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;

    // Hide the title text and icon
    nsWindow.titleVisibility = NSWindowTitleHidden;

    // Set background color to #1a1a1a
    // 0x1a = 26. 26/255.0 ~= 0.10196
    CGFloat component = 26.0 / 255.0;
    nsWindow.backgroundColor = [NSColor colorWithSRGBRed:component green:component blue:component alpha:1.0];
}

} // namespace WindowUtils
} // namespace Platform

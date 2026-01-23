#include "../WindowUtils.h"
#include <Cocoa/Cocoa.h>
#include <QWindow>

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

namespace Platform {
namespace WindowUtils {

class DraggableWindowFilter : public QObject {
public:
    explicit DraggableWindowFilter(QWidget* parent) : QObject(parent), window(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // Ensure we are clicking on the main window directly (or a container that propagates)
                // and within the top area (simulated title bar height ~30px)
                if (mouseEvent->position().y() < 30) { 
                     // Check if we hit a button or interactive widget?
                     // The event filter is on the window, so child widgets usually handle their own events first.
                     // If we are here, it likely bubbled up or was on the window background.
                    if (window->windowHandle()->startSystemMove()) {
                        return true;
                    }
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QWidget* window;
};

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
    
    // Window dragging logic:
    // With full size content view, standard title bar dragging is lost.
    // [nsWindow setMovableByWindowBackground:YES] allows dragging by background,
    // but that means clicking ANYWHERE in the background drags the window.
    // Instead, we use a Qt event filter to restrict it to the top area.
    auto* filter = new DraggableWindowFilter(window);
    window->installEventFilter(filter);

    // Hide the title text and icon
    nsWindow.titleVisibility = NSWindowTitleHidden;

    // Set background color to #1a1a1a
    // 0x1a = 26. 26/255.0 ~= 0.10196
    CGFloat component = 26.0 / 255.0;
    nsWindow.backgroundColor = [NSColor colorWithSRGBRed:component green:component blue:component alpha:1.0];
}

} // namespace WindowUtils
} // namespace Platform

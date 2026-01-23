#include "../WindowUtils.h"
#include <Cocoa/Cocoa.h>
#include <QWindow>

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

namespace Platform {
namespace WindowUtils {

// Helper to apply the native style
static void applyNativeStyle(QWidget* window) {
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

@interface WindowObserver : NSObject {
    QWidget* _qWidget;
}
- (instancetype)initWithQWidget:(QWidget*)widget;
@end

@implementation WindowObserver
- (instancetype)initWithQWidget:(QWidget*)widget {
    self = [super init];
    if (self) {
        _qWidget = widget;
        NSView* view = (NSView*)widget->winId();
        NSWindow* nsWindow = [view window];
        if (nsWindow) {
             [[NSNotificationCenter defaultCenter] addObserver:self
                                                      selector:@selector(windowStateDidChange:)
                                                          name:NSWindowDidExitFullScreenNotification
                                                        object:nsWindow];
             [[NSNotificationCenter defaultCenter] addObserver:self
                                                      selector:@selector(windowStateDidChange:)
                                                          name:NSWindowDidDeminiaturizeNotification
                                                        object:nsWindow];
             // Also Listen for live resize end, just in case
             [[NSNotificationCenter defaultCenter] addObserver:self
                                                      selector:@selector(windowStateDidChange:)
                                                          name:NSWindowDidEndLiveResizeNotification
                                                        object:nsWindow];
        }
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)windowStateDidChange:(NSNotification *)notification {
    Platform::WindowUtils::applyNativeStyle(_qWidget);
}
@end

namespace Platform {
namespace WindowUtils {

class DraggableWindowFilter : public QObject {
public:
    explicit DraggableWindowFilter(QWidget* parent) : QObject(parent), window(parent) {
        observer = [[WindowObserver alloc] initWithQWidget:parent];
    }
    
    ~DraggableWindowFilter() {
        if (observer) {
            [observer release];
            observer = nullptr;
        }
    }

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
                    if (window->windowHandle() && window->windowHandle()->startSystemMove()) {
                        return true;
                    }
                }
            }
        }
        else if (event->type() == QEvent::WindowStateChange || event->type() == QEvent::Show) {
            // Re-apply style when state changes (e.g. un-maximize) or window is shown
            applyNativeStyle(window);
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QWidget* window;
    WindowObserver* observer;
};

void customizeTitleBar(QWidget* window) {
    // Apply initial style
    applyNativeStyle(window);
    
    // Install filter for dragging and style persistence
    auto* filter = new DraggableWindowFilter(window);
    window->installEventFilter(filter);
}

} // namespace WindowUtils
} // namespace Platform

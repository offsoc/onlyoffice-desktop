#include "widget.h"
#include "application.h"
#include "metrics.h"
#include "palette.h"
#include "drawningengine.h"
#ifdef _WIN32
# include <CommCtrl.h>
#else

#endif


static bool isAllocOnHeap(void *addr) {
#ifdef _WIN32
    if (HANDLE procHeap = GetProcessHeap()) {
        if (HeapLock(procHeap)) {
            bool res = false;
            PROCESS_HEAP_ENTRY entry = {0};
            while (HeapWalk(procHeap, &entry)) {
                if ((entry.wFlags & PROCESS_HEAP_REGION) && addr >= (void*)entry.Region.lpFirstBlock && addr <= (void*)entry.Region.lpLastBlock) {
                    res = true;
                    break;
                }
            }
            if (!HeapUnlock(procHeap))
                res = false;
            return res;
        }
    }
#else
    if (FILE *maps = fopen("/proc/self/maps", "r")) {
        char *line = NULL;
        size_t line_size = 0;
        uintptr_t start_address = 0, end_address = 0;
        int name_start = 0, name_end = 0;
        while (getline(&line, &line_size, maps) > 0) {
            if (sscanf(line, "%lx-%lx %*s %*lx %*u:%*u %*lu %n%*[^\n]%n", &start_address, &end_address, &name_start, &name_end) == 2) {
                if (name_end > name_start) {
                    line[name_end] = '\0';
                    if (strcmp(&line[name_start], "[heap]") == 0 && (uintptr_t)addr >= start_address && (uintptr_t)addr <= end_address) {
                        free(line);
                        fclose(maps);
                        return true;
                    }
                }
            }
        }
        free(line);
        fclose(maps);
    }
#endif
    return false;
}


Widget::Widget(Widget *parent) :
    Widget(parent, ObjectType::WidgetType)
{}

Widget::Widget(Widget *parent, WindowHandle hwnd) :
    Object(parent),
    DrawningSurface(),
    m_hWnd(hwnd),
    m_layout(nullptr),
    m_disabled(false),
    m_is_created(false),
    m_is_destroyed(false),
    m_is_class_destroyed(false),
    m_mouse_entered(false)
{
#ifdef _WIN32
    LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_CHILD;
    ::SetWindowLong(m_hWnd, GWL_STYLE, style);
#else
#endif
    m_properties[Properties::HSizeBehavior] = SizeBehavior::Expanding;
    m_properties[Properties::VSizeBehavior] = SizeBehavior::Expanding;
#ifdef _WIN32
    SetParent(hwnd, parent->nativeWindowHandle());
#else
#endif
}

Widget::Widget(Widget *parent, ObjectType type, const Rect &rc) :
    Object(parent),
    DrawningSurface(),
    m_hWnd(nullptr),
    m_layout(nullptr),
    m_disabled(false),
    m_is_created(false),
    m_is_destroyed(false),
    m_is_class_destroyed(false),
    m_mouse_entered(false)
{
    m_properties[Properties::HSizeBehavior] = SizeBehavior::Expanding;
    m_properties[Properties::VSizeBehavior] = SizeBehavior::Expanding;
    Application::instance()->registerWidget(this, type, rc);
}

Widget::~Widget()
{
    m_is_class_destroyed = true;
    if (m_layout) {
        if (isAllocOnHeap(m_layout))
            delete m_layout;
        m_layout = nullptr;
    }
#ifdef _WIN32
    if (!m_is_destroyed)
        DestroyWindow(m_hWnd);
#else

#endif
}

void Widget::setGeometry(int x, int y, int width, int height)
{
#ifdef _WIN32
    SetWindowPos(m_hWnd, NULL, x, y, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER /*| SWP_NOSENDCHANGING*/);
#else

#endif
}

void Widget::setDisabled(bool disable)
{
    m_disabled = disable;
    palette()->setCurrentState(disable ? Palette::Disabled : Palette::Normal);
    update();
}

void Widget::close()
{
#ifdef _WIN32
    PostMessage(m_hWnd, WM_CLOSE, 0, 0);
#else

#endif
}

void Widget::move(int x, int y)
{
#ifdef _WIN32
    SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER /*| SWP_NOSENDCHANGING*/);
#else

#endif
}

void Widget::resize(int w, int h)
{
#ifdef _WIN32
    SetWindowPos(m_hWnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER /*| SWP_NOSENDCHANGING*/);
#else

#endif
}

Widget *Widget::parentWidget()
{
    return dynamic_cast<Widget*>(parent());
}

tstring Widget::title()
{
    return m_title;
}

Size Widget::size()
{
#ifdef _WIN32
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    return Size(rc.right - rc.left, rc.bottom - rc.top);
#else
    return Size();
#endif
}

void Widget::size(int *width, int *height)
{
#ifdef _WIN32
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    *width = rc.right - rc.left;
    *height =  rc.bottom - rc.top;
#else

#endif
}

void Widget::setWindowTitle(const tstring &title)
{
    m_title = title;
#ifdef _WIN32
    SetWindowText(m_hWnd, title.c_str());
#else
    gtk_window_set_title(GTK_WINDOW(m_hWnd), title.c_str());
#endif
}

void Widget::setProperty(Properties property, int val)
{
    m_properties[property] = val;
}

void Widget::show()
{
#ifdef _WIN32
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
#else
    gtk_widget_show(m_hWnd);
#endif
}

void Widget::hide()
{
#ifdef _WIN32
    ShowWindow(m_hWnd, SW_HIDE);
#else

#endif
}

void Widget::repaint()
{
#ifdef _WIN32
    if (IsWindowVisible(m_hWnd))
        RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT | RDW_UPDATENOW);
#else
    gtk_widget_queue_draw(m_hWnd);
    gdk_window_process_all_updates();
#endif
}

void Widget::update()
{
#ifdef _WIN32
    if (IsWindowVisible(m_hWnd))
        RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT);
#else
    gtk_widget_queue_draw(m_hWnd);
#endif
}

void Widget::setLayout(Layout *layout)
{
    if (m_layout) {
        // TODO: error: trying to add a layout when the widget contains a layout
    } else {
        m_layout = layout;
    }
}

bool Widget::isCreated()
{
    return m_is_created;
}

bool Widget::underMouse()
{
#ifdef _WIN32
    POINT pt;
    GetCursorPos(&pt);
    return WindowFromPoint(pt) == m_hWnd;
#else
    return false;
#endif
}

int Widget::property(Properties property)
{
    return m_properties[property];
}

Layout *Widget::layout()
{
    return m_layout;
}

WindowHandle Widget::nativeWindowHandle()
{
    return m_hWnd;
}

Widget *Widget::widgetFromHwnd(Widget *parent, WindowHandle hwnd)
{
    return new Widget(parent, hwnd);
}

int Widget::onResize(const FnVoidIntInt &callback)
{
    m_resize_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int Widget::onMove(const FnVoidIntInt &callback)
{
    m_move_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int Widget::onAboutToDestroy(const FnVoidVoid &callback)
{
    m_destroy_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int Widget::onCreate(const FnVoidVoid &callback)
{
    m_create_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

int Widget::onClose(const FnVoidBoolPtr &callback)
{
    m_close_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

void Widget::disconnect(int connectionId)
{
    {
        auto it = m_resize_callbacks.find(connectionId);
        if (it != m_resize_callbacks.end()) {
            m_resize_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_move_callbacks.find(connectionId);
        if (it != m_move_callbacks.end()) {
            m_move_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_destroy_callbacks.find(connectionId);
        if (it != m_destroy_callbacks.end()) {
            m_destroy_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_create_callbacks.find(connectionId);
        if (it != m_create_callbacks.end()) {
            m_create_callbacks.erase(it);
            return;
        }
    }
    {
        auto it = m_close_callbacks.find(connectionId);
        if (it != m_close_callbacks.end()) {
            m_close_callbacks.erase(it);
            return;
        }
    }
}

#ifdef _WIN32
bool Widget::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_ACTIVATE:
        break;

    case WM_CREATE: {
        m_is_created = true;
        for (auto it = m_create_callbacks.begin(); it != m_create_callbacks.end(); it++)
            if (it->second)
                (it->second)();
        break;
    }

    case WM_SIZE:
        if (m_layout)
            m_layout->onResize(LOWORD(lParam), HIWORD(lParam));
        for (auto it = m_resize_callbacks.begin(); it != m_resize_callbacks.end(); it++)
            if (it->second)
                (it->second)(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_MOVE:
        for (auto it = m_move_callbacks.begin(); it != m_move_callbacks.end(); it++)
            if (it->second)
                (it->second)(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        //DrawRoundedRect();
        engine()->End();
        *result = FALSE;
        return true;
    }

    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN: {
        break;
    }

    case WM_ERASEBKGND: {
        *result = FALSE;
        return true;
    }

    case WM_LBUTTONDBLCLK: {
        break;
    }

    case WM_LBUTTONUP: {
        break;
    }

    case WM_MOUSEMOVE: {
        if (!m_mouse_entered) {
            m_mouse_entered = true;
            PostMessage(m_hWnd, WM_MOUSEENTER, 0, 0);
        }
        // add here impl onMouseMove
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE /*| TME_HOVER*/;
        tme.dwHoverTime = HOVER_DEFAULT;
        _TrackMouseEvent(&tme);
        break;
    }

    case WM_NCMOUSEMOVE: {
        if (!m_mouse_entered) {
            m_mouse_entered = true;
            PostMessage(m_hWnd, WM_MOUSEENTER, 0, 0);
        }
        // add here impl onMouseMove
        break;
    }

    case WM_MOUSEHOVER:
    case WM_NCMOUSEHOVER: {
        break;
    }

    case WM_MOUSEENTER: {
        // palette()->setCurrentState(Palette::Hover);
        // repaint();
        break;
    }

    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE: {
        if (m_mouse_entered) {
            m_mouse_entered = false;
        }
        // palette()->setCurrentState(Palette::Normal);
        // repaint();
        break;
    }

    case WM_CLOSE: {
        bool accept = true;
        for (auto it = m_close_callbacks.begin(); it != m_close_callbacks.end(); it++)
            if (it->second)
                (it->second)(&accept);
        if (accept)
            DestroyWindow(m_hWnd);
        *result = TRUE;
        return true;
    }

    case WM_DESTROY: {
        m_is_destroyed = true;
        for (auto it = m_destroy_callbacks.begin(); it != m_destroy_callbacks.end(); it++)
            if (it->second)
                (it->second)();

        if (!m_is_class_destroyed) {
            if (isAllocOnHeap(this)) {
                SetWindowLongPtr(m_hWnd, GWLP_USERDATA, 0);
                delete this;
            }
        }
        break;
    }

    default:
        break;
    }
    return false;
}
#else
bool Widget::event(GdkEvent *ev)
{
    switch (ev->type) {
    case GDK_EXPOSE: {

        break;
    }
    case GDK_DELETE: {
        m_is_destroyed = true;
        for (auto it = m_destroy_callbacks.begin(); it != m_destroy_callbacks.end(); it++)
            if (it->second)
                (it->second)();

        if (!m_is_class_destroyed) {
            if (isAllocOnHeap(this)) {
                delete this;
            }
        }
        break;
    }

    default:
        break;
    }
    return false;
}
#endif

void Widget::setNativeWindowHandle(WindowHandle hWnd)
{
    m_hWnd = hWnd;
}

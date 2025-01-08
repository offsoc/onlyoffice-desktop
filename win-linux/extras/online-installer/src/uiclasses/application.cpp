#include "application.h"
#include "widget.h"
#include "src/resource.h"
#ifdef _WIN32
# include <gdiplus.h>
#else

#endif


class Application::ApplicationPrivate
{
public:
    ApplicationPrivate();
    ~ApplicationPrivate();

#ifdef _WIN32
    ULONG_PTR gdi_token;
    HINSTANCE hInstance;
    ATOM registerClass(LPCWSTR className, HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else
    int exit_code;
    void registerEvents(Widget *wgt);
    static gboolean EventProc(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void EventAfterProc(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void RealizeEventProc(GtkWidget *wgt, gpointer data);
    static gboolean ConfigEventProc(GtkWidget *wgt, GdkEventConfigure *ev, gpointer data);
    static void SizeEventProc(GtkWidget *wgt, GtkAllocation *alc, gpointer data);
    static gboolean DrawProc(GtkWidget *wgt, cairo_t *cr, gpointer data);
    static void DestroyProc(GtkWidget *wgt, gpointer data);
#endif
    LayoutDirection layoutDirection;
    int windowId;
};

Application::ApplicationPrivate::ApplicationPrivate() :
#ifdef _WIN32
    gdi_token(0),
    hInstance(nullptr),
#else
    exit_code(0),
#endif
    layoutDirection(LayoutDirection::LeftToRight),
    windowId(0)
{
#ifdef _WIN32
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdi_token, &gdiplusStartupInput, nullptr);
#endif
}

Application::ApplicationPrivate::~ApplicationPrivate()
{
#ifdef _WIN32
    Gdiplus::GdiplusShutdown(gdi_token);
#endif
}

#ifdef _WIN32
ATOM Application::ApplicationPrivate::registerClass(LPCWSTR className, HINSTANCE hInstance)
{
    WNDCLASSEX wcx;
    memset(&wcx, 0, sizeof(wcx));
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    // wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    // wcx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wcx.lpszClassName = className;
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassEx(&wcx);
}

LRESULT CALLBACK Application::ApplicationPrivate::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE) {
        if (CREATESTRUCT *cs = (CREATESTRUCT*)lParam) {
            if (Widget *wgt = (Widget*)cs->lpCreateParams) {
                wgt->setNativeWindowHandle(hWnd);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
                LRESULT result = 0;
                if (wgt->event(msg, wParam, lParam, &result))
                    return result;
            }
        }
    } else
    if (Widget *wgt = (Widget*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
         LRESULT result = 0;
         if (wgt->event(msg, wParam, lParam, &result))
             return result;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

#else

void Application::ApplicationPrivate::registerEvents(Widget *wgt)
{
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "event", G_CALLBACK(EventProc), wgt);
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "event-after", G_CALLBACK(EventAfterProc), wgt);
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "realize", G_CALLBACK(RealizeEventProc), wgt);
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "configure-event", G_CALLBACK(ConfigEventProc), wgt);
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "size-allocate", G_CALLBACK(SizeEventProc), wgt);
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "draw", G_CALLBACK(DrawProc), wgt);
    g_signal_connect(G_OBJECT(wgt->nativeWindowHandle()), "destroy", G_CALLBACK(DestroyProc), wgt);
}

gboolean Application::ApplicationPrivate::EventProc(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        return w->event(ev->type, (void*)ev);
    }
    return FALSE;
}

void Application::ApplicationPrivate::EventAfterProc(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        switch (ev->type) {
        case GDK_CONFIGURE: {
            w->event((GdkEventType)GDK_CONFIGURE_AFTER, NULL);
            break;
        }
        case GDK_FOCUS_CHANGE: {
            w->event((GdkEventType)GDK_FOCUS_CHANGE_AFTER, (void*)ev);
            break;
        }
        case GDK_WINDOW_STATE: {
            w->event((GdkEventType)GDK_WINDOW_STATE_AFTER, (void*)ev);
            break;
        }
        default:
            break;
        }
    }
}

void Application::ApplicationPrivate::RealizeEventProc(GtkWidget *wgt, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        w->event((GdkEventType)GDK_REALIZE_CUSTOM, NULL);
    }
}

gboolean Application::ApplicationPrivate::ConfigEventProc(GtkWidget *wgt, GdkEventConfigure *ev, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        return w->event((GdkEventType)GDK_CONFIG_CUSTOM, ev);
    }
    return FALSE;
}

void Application::ApplicationPrivate::SizeEventProc(GtkWidget *wgt, GtkAllocation *alc, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        w->event((GdkEventType)GDK_SIZING_CUSTOM, (void*)alc);
    }
}

gboolean Application::ApplicationPrivate::DrawProc(GtkWidget *wgt, cairo_t *cr, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        return w->event((GdkEventType)GDK_DRAW_CUSTOM, cr);
    }
    return FALSE;
}

void Application::ApplicationPrivate::DestroyProc(GtkWidget *wgt, gpointer data)
{
    if (Widget *w = (Widget*)data) {
        w->event((GdkEventType)GDK_DESTROY_CUSTOM, NULL);
    }
}
#endif

Application *Application::inst = nullptr;

#ifdef _WIN32
Application::Application(HINSTANCE hInstance, PWSTR cmdline, int cmdshow) :
#else
Application::Application(int argc, char *argv[]) :
#endif
    Application()
{
#ifdef _WIN32
    d_ptr->hInstance = hInstance;
    if (!d_ptr->hInstance)
        d_ptr->hInstance = GetModuleHandle(NULL);
#else
    gtk_init(&argc, &argv);
#endif
    inst = this;
}

Application::Application() :
    Object(nullptr),
    d_ptr(new ApplicationPrivate)
{

}

Application *Application::instance()
{
    return inst;
}

#ifdef _WIN32
HINSTANCE Application::moduleHandle()
{
    return d_ptr->hInstance;
}
#endif

void Application::setLayoutDirection(LayoutDirection layoutDirection)
{
    d_ptr->layoutDirection = layoutDirection;
}

Application::~Application()
{
    delete d_ptr, d_ptr = nullptr;
}

int Application::exec()
{
#ifdef _WIN32
    MSG msg;
    BOOL res;
    while ((res = GetMessage(&msg, NULL, 0, 0)) != 0 && res != -1) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
#else
    gtk_main();
    return d_ptr->exit_code;
#endif
}

void Application::exit(int code)
{
#ifdef _WIN32
    PostQuitMessage(code);
#else
    d_ptr->exit_code = code;
    gtk_main_quit();
#endif
}

void Application::registerWidget(Widget *wgt, ObjectType objType, const Rect &rc)
{
#ifdef _WIN32
    std::wstring className;
    DWORD style = WS_CLIPCHILDREN;
    DWORD exStyle = d_ptr->layoutDirection == LayoutDirection::RightToLeft ? WS_EX_LAYOUTRTL : 0;
    HWND hWndParent = wgt->parentWidget() ? wgt->parentWidget()->nativeWindowHandle() : HWND_DESKTOP;

    switch (objType) {
    case ObjectType::WindowType:
        className = L"MainWindow " + std::to_wstring(++d_ptr->windowId);
        style |= WS_OVERLAPPEDWINDOW;
        exStyle |= WS_EX_APPWINDOW;
        break;    

    case ObjectType::DialogType:
        className = L"Dialog " + std::to_wstring(++d_ptr->windowId);
        style |= WS_CAPTION | WS_SYSMENU /*| DS_MODALFRAME*/;
        exStyle |= WS_EX_DLGMODALFRAME;
        break;

    case ObjectType::PopupType:
        className = L"Popup " + std::to_wstring(++d_ptr->windowId);
        style |= WS_POPUP;
        exStyle |= WS_EX_TOOLWINDOW | WS_EX_LAYERED;
        break;

    case ObjectType::WidgetType:
    default:
        className = L"Widget " + std::to_wstring(++d_ptr->windowId);
        style |= WS_CHILD;
        break;
    }

    // if (wgt->parent()) {
    //     if (wgt->parentWidget()->isCreated()) {
    //         d_ptr->registerClass(className.c_str(), hInstance);
    //         wgt->m_hWnd = CreateWindowEx(exStyle, className.c_str(), wgt->title().c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, NULL, hInstance, NULL);
    //         SetWindowLongPtr(wgt->m_hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
    //     } else {
    //         wgt->connectOnCreate([=]() {
    //             wgt->m_hWnd = CreateWindowEx(exStyle, className.c_str(), wgt->title().c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, NULL, hInstance, NULL);
    //             SetWindowLongPtr(wgt->m_hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
    //         });
    //     }
    // } else {
        d_ptr->registerClass(className.c_str(), d_ptr->hInstance);
        CreateWindowEx(exStyle, className.c_str(), wgt->title().c_str(), style, rc.x, rc.y, rc.width, rc.height,
                       hWndParent, NULL, d_ptr->hInstance, (LPVOID)wgt);
    // }
#else
    std::string className;
    GtkWidget *gtkParent = wgt->parentWidget() ? wgt->parentWidget()->nativeWindowHandle() : nullptr;
    GtkWidget *gtkWgt = nullptr;

    switch (objType) {
    case ObjectType::WindowType:
        className = "MainWindow_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_resize(GTK_WINDOW(gtkWgt), rc.width, rc.height);
        gtk_window_move(GTK_WINDOW(gtkWgt), rc.x, rc.y);
        break;

    case ObjectType::DialogType:
        className = "Dialog_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_dialog_new();
        if (gtkParent)
            gtk_widget_set_parent(gtkWgt, gtkParent);
        gtk_window_resize(GTK_WINDOW(gtkWgt), rc.width, rc.height);
        gtk_window_move(GTK_WINDOW(gtkWgt), rc.x, rc.y);
        break;

    case ObjectType::PopupType:
        className = "Popup_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_window_new(GTK_WINDOW_POPUP);
        gtk_window_resize(GTK_WINDOW(gtkWgt), rc.width, rc.height);
        gtk_window_move(GTK_WINDOW(gtkWgt), rc.x, rc.y);
        break;

    case ObjectType::WidgetType:
    default:
        className = "Widget_" + std::to_string(++d_ptr->windowId);
        gtkWgt = gtk_layout_new(NULL, NULL);
        gtk_widget_set_size_request(gtkWgt, rc.width, rc.height);
        gtk_widget_add_events(gtkWgt, gtk_widget_get_events(gtkWgt) | GDK_ALL_EVENTS_MASK);
        if (gtkParent)
            gtk_layout_put((GtkLayout*)wgt->parentWidget()->gtkLayout(), gtkWgt, rc.x, rc.y);
        break;
    }

    wgt->setNativeWindowHandle(gtkWgt);
    d_ptr->registerEvents(wgt);
    gtk_widget_set_name(gtkWgt, className.c_str());
    if (d_ptr->layoutDirection == LayoutDirection::RightToLeft)
        gtk_widget_set_direction(gtkWgt, GTK_TEXT_DIR_RTL);
#endif
}

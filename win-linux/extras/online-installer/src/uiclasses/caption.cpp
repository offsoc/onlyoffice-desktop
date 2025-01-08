#include "caption.h"
#include "baseutils.h"
#include "metrics.h"
#include "drawningengine.h"
#ifdef _WIN32
# include <windowsx.h>
#endif

#define RESIZE_AREA_PART 0.14


Caption::Caption(Widget *parent) :
    Label(parent),
    m_isResizingAvailable(true)
{
#ifdef _WIN32
    m_hwndRoot = GetAncestor(m_hWnd, GA_ROOT);
#else

#endif
}

Caption::~Caption()
{

}

void Caption::setResizingAvailable(bool isResizingAvailable)
{
    m_isResizingAvailable = isResizingAvailable;
}

#ifdef _WIN32
bool Caption::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        //    DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (!m_title.empty())
            engine()->DrawText(rc, m_title);

        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN: {
        if (isResizingAvailable()) {
            int y = GET_Y_LPARAM(lParam);
            if (HCURSOR hCursor = LoadCursor(NULL, isPointInResizeArea(y) ? IDC_SIZENS : IDC_ARROW))
                SetCursor(hCursor);
        }
        if (postMsg(WM_NCLBUTTONDOWN)) {
            *result = TRUE;
            return true;
        }
        return false;
    }

    case WM_LBUTTONDBLCLK: {
        if (postMsg(WM_NCLBUTTONDBLCLK)) {
            *result = TRUE;
            return true;
        }
        return false;
    }

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE: {
        if (isResizingAvailable()) {
            int y = GET_Y_LPARAM(lParam);
            if (HCURSOR hCursor = LoadCursor(NULL, isPointInResizeArea(y) ? IDC_SIZENS : IDC_ARROW))
                SetCursor(hCursor);
        }
        break;
    }

    case WM_MOUSEENTER: {
        //palette()->setCurrentState(Palette::Hover);
        repaint();
        break;
    }

    case WM_MOUSELEAVE: {
        //palette()->setCurrentState(Palette::Normal);
        repaint();
        break;
    }

    default:
        break;
    }
    return Label::event(msg, wParam, lParam, result);
}
#else
gboolean on_dbl_button_press(GtkWindow *root)
{
    if (gtk_window_is_maximized(GTK_WINDOW(root))) {
        gtk_window_unmaximize(GTK_WINDOW(root));
    } else {
        gtk_window_maximize(GTK_WINDOW(root));
    }
    return G_SOURCE_REMOVE;
}

bool Caption::event(GdkEventType ev_type, void *param)
{
    switch (ev_type) {
    case GDK_DRAW_CUSTOM: {
        Rect rc(0, 0, gtk_widget_get_allocated_width(m_hWnd), gtk_widget_get_allocated_height(m_hWnd));

        engine()->Begin(this, (cairo_t*)param, &rc);
        engine()->FillBackground();
        //    DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (!m_title.empty())
            engine()->DrawText(rc, m_title);

        engine()->End();
        return false;
    }

    case GDK_BUTTON_PRESS: {
        GdkEventButton *bev = (GdkEventButton*)param;
        GtkWidget *root = gtk_widget_get_toplevel(m_hWnd);
        if (root && bev->button == GDK_BUTTON_PRIMARY) {
            gtk_window_begin_move_drag(GTK_WINDOW(root), bev->button, bev->x_root, bev->y_root, bev->time);
            return true;
        }
        return false;
    }

    case GDK_DOUBLE_BUTTON_PRESS_AFTER: {
        GdkEventButton *bev = (GdkEventButton*)param;
        GtkWidget *root = gtk_widget_get_toplevel(m_hWnd);
        if (root && bev->button == GDK_BUTTON_PRIMARY) {
            g_timeout_add(150, (GSourceFunc)on_dbl_button_press, root);
            return true;
        }
        return false;
    }

    case GDK_ENTER_NOTIFY: {
        //palette()->setCurrentState(Palette::Hover);
        repaint();
        break;
    }

    case GDK_LEAVE_NOTIFY: {
        //palette()->setCurrentState(Palette::Normal);
        repaint();
        break;
    }

    default:
        break;
    }
    return Label::event(ev_type, param);
}
#endif

bool Caption::isResizingAvailable()
{
#ifdef _WIN32
    return m_isResizingAvailable && Utils::getWinVersion() >= Utils::WinVer::Win10 && !IsZoomed(m_hwndRoot);
#else
    return m_isResizingAvailable;
#endif
}

#ifdef _WIN32
bool Caption::isPointInResizeArea(int posY)
{
    int w = 0, h = 0;
    size(&w, &h);
    return posY <= RESIZE_AREA_PART * h;
}

bool Caption::postMsg(DWORD cmd) {
    POINT pt;
    ::GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);
    ::ReleaseCapture();
    ::PostMessage(m_hwndRoot, cmd, isResizingAvailable() && isPointInResizeArea(pt.y) ? HTTOP : HTCAPTION, POINTTOPOINTS(pt));
    return true;
}
#endif

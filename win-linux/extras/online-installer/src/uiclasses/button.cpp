#include "button.h"
#include "baseutils.h"
#include "drawningengine.h"
#include "metrics.h"
#include "palette.h"
#ifdef _WIN32
# include <windowsx.h>


static bool isArrangingAllowed() {
    BOOL arranging = FALSE;
    SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
    return (arranging == TRUE);
}
#endif

Button::Button(Widget *parent, const tstring &text) :
    AbstractButton(parent, text),
    m_stockIcon(StockIcon::None),
#ifdef _WIN32
    m_hIcon(nullptr),
    m_hMetaFile(nullptr),
    supportSnapLayouts(false),
    snapLayoutAllowed(false),
    snapLayoutTimerIsSet(false)
#else
    m_pb(nullptr)
#endif
{

}

Button::~Button()
{
#ifdef _WIN32
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    if (m_hMetaFile) {
        //delete m_hMetaFile;
        DeleteEnhMetaFile(m_hMetaFile);
        m_hMetaFile = nullptr;
    }
#else
    if (m_pb) {
        g_object_unref(m_pb);
        m_pb = nullptr;
    }
#endif
}

void Button::setIcon(const tstring &path, int w, int h)
{
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
#ifdef _WIN32
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    m_hIcon = (HICON)LoadImage(NULL, path.c_str(), IMAGE_ICON, w, h, LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_SHARED);
#else
    if (m_pb) {
        g_object_unref(m_pb);
        m_pb = nullptr;
    }
    m_pb = gdk_pixbuf_new_from_resource_at_scale(path.c_str(), w, h, TRUE, NULL);
#endif
    update();
}

#ifdef _WIN32
void Button::setIcon(int id, int w, int h)
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    m_hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, w, h, LR_COPYFROMRESOURCE | LR_DEFAULTCOLOR | LR_SHARED);
    update();
}

void Button::setEMFIcon(const std::wstring &path, int w, int h)
{
    if (m_hMetaFile) {
        //delete m_hMetaFile;
        DeleteEnhMetaFile(m_hMetaFile);
        m_hMetaFile = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    m_hMetaFile = GetEnhMetaFile(path.c_str());
    //m_hMetaFile = new Metafile(path.c_str());
    update();
}

void Button::setEMFIcon(int id, int w, int h)
{
    if (m_hMetaFile) {
        //delete m_hMetaFile;
        DeleteEnhMetaFile(m_hMetaFile);
        m_hMetaFile = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(id), RT_RCDATA)) {
        if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
            if (LPVOID pData = LockResource(hResData)) {
                DWORD dataSize = SizeofResource(hInst, hRes);
                if (dataSize > 0)
                    m_hMetaFile = SetEnhMetaFileBits(dataSize, (BYTE*)pData);
            }
            FreeResource(hResData);
        }
    }
    update();
}

void Button::setSupportSnapLayouts()
{
    if (Utils::getWinVersion() > Utils::WinVer::Win10) {
        snapLayoutAllowed = isArrangingAllowed();
        supportSnapLayouts = true;
    }
}
#endif

void Button::setIconSize(int w, int h)
{
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    update();
}

void Button::setStockIcon(StockIcon stockIcon)
{
    m_stockIcon = stockIcon;
    update();
}

#ifdef _WIN32
bool Button::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        // engine()->DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (m_hIcon)
            engine()->DrawIcon(m_hIcon);
        if (m_hMetaFile)
            engine()->DrawEmfIcon(m_hMetaFile);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text);

        if (m_stockIcon == StockIcon::CloseIcon)
            engine()->DrawStockCloseIcon();
        else
        if (m_stockIcon == StockIcon::RestoreIcon)
            engine()->DrawStockRestoreIcon();
        else
        if (m_stockIcon == StockIcon::MinimizeIcon)
            engine()->DrawStockMinimizeIcon();
        else
        if (m_stockIcon == StockIcon::MaximizeIcon)
            engine()->DrawStockMaximizeIcon();

        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_NCHITTEST: {
        if (supportSnapLayouts && snapLayoutAllowed) {
            if (!snapLayoutTimerIsSet) {
                snapLayoutTimerIsSet = true;
                palette()->setCurrentState(Palette::Hover);
                SetTimer(m_hWnd, SNAP_LAYOUTS_TIMER_ID, 100, NULL);
                repaint();
            }
            *result = HTMAXBUTTON;
            return true;
        }
        return false;
    }

    case WM_TIMER: {
        if (wParam == SNAP_LAYOUTS_TIMER_ID) {
            if (!underMouse()) {
                KillTimer(m_hWnd, wParam);
                snapLayoutTimerIsSet = false;
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case WM_CAPTURECHANGED: {
        if (Utils::getWinVersion() > Utils::WinVer::Win10) {
            click();
        }
        break;
    }

    default:
        break;
    }
    return AbstractButton::event(msg, wParam, lParam, result);
}
#else
bool Button::event(GdkEventType ev_type, void *param)
{
    switch (ev_type) {
    case GDK_DRAW_CUSTOM: {
        Rect rc(0, 0, gtk_widget_get_allocated_width(m_hWnd), gtk_widget_get_allocated_height(m_hWnd));

        engine()->Begin(this, (cairo_t*)param, &rc);
        engine()->FillBackground();
        // engine()->DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (m_pb)
            engine()->DrawIcon(m_pb);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text);

        if (m_stockIcon == StockIcon::CloseIcon)
            engine()->DrawStockCloseIcon();
        else
        if (m_stockIcon == StockIcon::RestoreIcon)
            engine()->DrawStockRestoreIcon();
        else
        if (m_stockIcon == StockIcon::MinimizeIcon)
            engine()->DrawStockMinimizeIcon();
        else
        if (m_stockIcon == StockIcon::MaximizeIcon)
            engine()->DrawStockMaximizeIcon();

        engine()->End();
        return false;
    }

    default:
        break;
    }
    return AbstractButton::event(ev_type, param);
}
#endif

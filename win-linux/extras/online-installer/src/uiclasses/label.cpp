#include "label.h"
#include "metrics.h"
#include "drawningengine.h"


Label::Label(Widget *parent) :
    Widget(parent, ObjectType::WidgetType),
#ifdef _WIN32
    m_hIcon(nullptr),
    m_hMetaFile(nullptr),
    m_hBmp(nullptr),
#else
    m_pb(nullptr),
#endif
    m_multiline(false)
{

}

Label::~Label()
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
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
#else
    if (m_pb) {
        g_object_unref(m_pb);
        m_pb = nullptr;
    }
#endif
}

void Label::setText(const tstring &text, bool multiline)
{
    m_text = text;
    m_multiline = multiline;
    update();
}

void Label::setIcon(const tstring &path, int w, int h)
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
void Label::setIcon(int id, int w, int h)
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

void Label::setEMFIcon(const std::wstring &path, int w, int h)
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

void Label::setEMFIcon(int id, int w, int h)
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

void Label::setImage(int id, int w, int h)
{
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(id), L"PNG")) {
        if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
            if (LPVOID pData = LockResource(hResData)) {
                DWORD dataSize = SizeofResource(hInst, hRes);
                if (dataSize > 0) {
                    if (HGLOBAL hGlobal = GlobalAlloc(GHND, dataSize)) {
                        if (LPVOID pBuffer = GlobalLock(hGlobal)) {
                            memcpy(pBuffer, pData, dataSize);
                            IStream *pStream = nullptr;
                            HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
                            if (SUCCEEDED(hr)) {
                                m_hBmp = new Gdiplus::Bitmap(pStream);
                                pStream->Release();
                            }
                            GlobalUnlock(hGlobal);
                        }
                        GlobalFree(hGlobal);
                    }
                }
            }
            FreeResource(hResData);
        }
    }
    update();
}
#endif

void Label::setImage(const tstring &path, int w, int h)
{
#ifdef _WIN32
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_hBmp = new Gdiplus::Bitmap(path.c_str());
    update();
#else
    setIcon(path, w, h);
#endif
}

void Label::setIconSize(int w, int h)
{
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    update();
}

#ifdef _WIN32
bool Label::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
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
        if (m_hBmp)
            engine()->DrawImage(m_hBmp);
        if (m_hIcon)
            engine()->DrawIcon(m_hIcon);
        if (m_hMetaFile)
            engine()->DrawEmfIcon(m_hMetaFile);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text, m_multiline);

        engine()->End();

        *result = FALSE;
        return true;
    }

    // case WM_MOUSEENTER: {
    //     palette()->setCurrentState(Palette::Hover);
    //     repaint();
    //     break;
    // }

    // case WM_MOUSELEAVE: {
    //     palette()->setCurrentState(Palette::Normal);
    //     repaint();
    //     break;
    // }

    default:
        break;
    }
    return Widget::event(msg, wParam, lParam, result);
}
#else
bool Label::event(GdkEventType ev_type, void *param)
{
    switch (ev_type) {
    case GDK_DRAW_CUSTOM: {
        Rect rc(0, 0, gtk_widget_get_allocated_width(m_hWnd), gtk_widget_get_allocated_height(m_hWnd));

        engine()->Begin(this, (cairo_t*)param, &rc);
        engine()->FillBackground();
        //    DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        // if (m_pb)
        //     engine()->DrawImage(m_pb);
        if (m_pb)
            engine()->DrawIcon(m_pb);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text, m_multiline);

        engine()->End();
        return false;
    }

    default:
        break;
    }
    return Widget::event(ev_type, param);
}
#endif

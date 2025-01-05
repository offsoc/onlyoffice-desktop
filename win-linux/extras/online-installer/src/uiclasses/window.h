#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"
#ifdef _WIN32
#else
# include <cstdint>
  typedef unsigned char BYTE;
  typedef uint16_t WORD;
  typedef uint32_t DWORD;
  typedef DWORD COLORREF;
#endif

#define DEFAULT_WINDOW_RECT Rect(100,100,1368,768)

#ifdef _WIN32
struct FRAME {
    FRAME() : left(0), top(0)
    {}
    FRAME(FRAME &frame) {
        left = frame.left;
        top = frame.top;
    }
    int left, top;
};
#endif

class WindowPrivate;
class Window : public Widget
{
public:
    Window(Widget *parent = nullptr, const Rect &rc = DEFAULT_WINDOW_RECT);
    virtual ~Window();

    void setCentralWidget(Widget*);
    void setContentsMargins(int, int, int, int);
    void setResizable(bool);
    void showAll();
    void showNormal();
    void showMinimized();
    void showMaximized();
    void setIcon(int);
    void setLayout(Layout*) = delete;
    bool isMinimized();
    bool isMaximized();
    Widget *centralWidget();
    Layout *layout() = delete;

    /* callback */
    int onStateChanged(const FnVoidInt &callback);

    virtual void disconnect(int) override;

protected:    
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(GdkEventType ev_type, void *param) override;
#endif

private:
    WindowPrivate *pimpl;
    Widget  *m_centralWidget;
    Margins  m_contentMargins;
    COLORREF m_brdColor;
    int      m_brdWidth,
             m_resAreaWidth,
             m_state;
#ifdef _WIN32
    double   m_dpi;
    FRAME    m_frame;
#endif
    bool     m_borderless,
             m_isResizable,
             m_isMaximized,
#ifdef _WIN32
             m_isThemeActive,
             m_isTaskbarAutoHideOn,
#endif
             m_scaleChanged;
    Size     m_init_size;

    std::unordered_map<int, FnVoidInt> m_state_callbacks;
};

#endif // WINDOW_H

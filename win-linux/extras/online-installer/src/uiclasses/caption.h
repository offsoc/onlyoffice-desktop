#ifndef CAPTION_H
#define CAPTION_H

#include "label.h"
#ifdef _WIN32
# include <Windows.h>
#endif


class Caption : public Label
{
public:
    Caption(Widget *parent = nullptr);
    ~Caption();

    void setResizingAvailable(bool);

    /* callback */

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(GdkEventType ev_type, void *param) override;
#endif

private:
    bool isResizingAvailable();
#ifdef _WIN32
    bool isPointInResizeArea(int posY);
    bool postMsg(DWORD cmd);

    HWND m_hwndRoot;
#endif
    bool m_isResizingAvailable;
};

#endif // CAPTION_H

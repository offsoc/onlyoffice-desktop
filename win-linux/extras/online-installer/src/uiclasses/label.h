#ifndef LABEL_H
#define LABEL_H

#include "widget.h"
#ifdef _WIN32
# include <Windows.h>
# include <gdiplus.h>
#else

#endif


class Label : public Widget
{
public:
    Label(Widget *parent = nullptr);
    virtual ~Label();

    void setText(const tstring &text, bool multiline = false);
    void setIcon(const tstring &path, int w, int h);
#ifdef _WIN32
    void setIcon(int id, int w, int h);
    void setEMFIcon(const tstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setImage(int id, int w, int h);
#endif
    void setImage(const tstring &path, int w, int h);
    void setIconSize(int w, int h);
    /* callback */

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(GdkEventType ev_type, void *param) override;
#endif

private:
    tstring m_text;
#ifdef _WIN32
    HICON m_hIcon;
    HENHMETAFILE m_hMetaFile;
    Gdiplus::Bitmap *m_hBmp;
#else
    GdkPixbuf *m_pb;
#endif
    bool  m_multiline;
};

#endif // LABEL_H

#ifndef BUTTON_H
#define BUTTON_H

#include "abstractbutton.h"
#ifdef _WIN32
# include <gdiplus.h>
#endif


class Button : public AbstractButton
{
public:
    Button(Widget *parent = nullptr, const tstring &text = tstring());
    virtual ~Button();

    enum StockIcon : BYTE {
        None,
        MinimizeIcon,
        MaximizeIcon,
        RestoreIcon,
        CloseIcon
    };

    void setIcon(const tstring &path, int w, int h);
#ifdef _WIN32
    void setIcon(int id, int w, int h);
    void setEMFIcon(const tstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setSupportSnapLayouts();
#endif
    void setIconSize(int w, int h);
    void setStockIcon(StockIcon stockIcon);

    /* callback */

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(GdkEventType ev_type, void *param) override;
#endif

private:
    int  m_stockIcon;
#ifdef _WIN32
    HICON m_hIcon;
    HENHMETAFILE m_hMetaFile;
    //Gdiplus::Metafile *m_hMetaFile;
    bool supportSnapLayouts,
         snapLayoutAllowed;
    bool snapLayoutTimerIsSet;
#else
    GdkPixbuf *m_pb;
#endif
};

#endif // BUTTON_H

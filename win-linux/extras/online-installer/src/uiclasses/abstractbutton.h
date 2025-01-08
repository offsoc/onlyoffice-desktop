#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "widget.h"
#include <unordered_map>


class AbstractButton : public Widget
{
public:
    AbstractButton(Widget *parent = nullptr, const tstring &text = tstring());
    virtual ~AbstractButton();

    void setText(const tstring &text);

    /* callback */
    int onClick(const FnVoidVoid &callback);
    virtual void disconnect(int) override;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(GdkEventType ev_type, void *param) override;
#endif
    virtual void click();

    tstring m_text;

private:
    std::unordered_map<int, FnVoidVoid> m_click_callbacks;
};

#endif // ABSTRACTBUTTON_H

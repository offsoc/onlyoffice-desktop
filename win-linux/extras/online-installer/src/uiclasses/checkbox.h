#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "abstractbutton.h"


class CheckBox : public AbstractButton
{
public:
    CheckBox(Widget *parent = nullptr, const tstring &text = tstring());
    virtual ~CheckBox();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(GdkEventType ev_type, void *param) override;
#endif
    virtual void click() override;

private:
    bool m_checked;
};

#endif // CHECKBOX_H

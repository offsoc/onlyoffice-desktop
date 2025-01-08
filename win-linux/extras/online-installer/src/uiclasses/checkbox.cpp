#include "checkbox.h"
#include "drawningengine.h"
#include "metrics.h"
#ifdef _WIN32
# include <windowsx.h>
#endif


CheckBox::CheckBox(Widget *parent, const tstring &text) :
    AbstractButton(parent, text),
    m_checked(false)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

CheckBox::~CheckBox()
{

}

void CheckBox::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

bool CheckBox::isChecked()
{
    return m_checked;
}

#ifdef _WIN32
bool CheckBox::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->DrawCheckBox(m_text, m_checked);
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();

        engine()->End();

        *result = FALSE;
        return true;
    }

    default:
        break;
    }
    return AbstractButton::event(msg, wParam, lParam, result);
}
#else
bool CheckBox::event(GdkEventType ev_type, void *param)
{
    switch (ev_type) {
    case GDK_DRAW_CUSTOM: {
        Rect rc(0, 0, gtk_widget_get_allocated_width(m_hWnd), gtk_widget_get_allocated_height(m_hWnd));

        engine()->Begin(this, (cairo_t*)param, &rc);
        engine()->DrawCheckBox(m_text, m_checked);
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();

        engine()->End();
        return false;
    }

    default:
        break;
    }
    return AbstractButton::event(ev_type, param);
}
#endif

void CheckBox::click()
{
    m_checked = !m_checked;
    update();
    AbstractButton::click();
}

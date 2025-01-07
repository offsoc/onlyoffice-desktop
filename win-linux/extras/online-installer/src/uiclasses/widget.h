#ifndef WIDGET_H
#define WIDGET_H

#include "object.h"
#include "drawingsurface.h"
#include "layout.h"
#include "commondefines.h"
#include <unordered_map>
#ifdef _WIN32
# include <Windows.h>
# define tstring std::wstring
  typedef HWND WindowHandle;
#else
# include <gtk/gtk.h>
# define CW_USEDEFAULT 100
# define tstring std::string
  typedef unsigned char BYTE;
  typedef GtkWidget* WindowHandle;
#endif

class Widget : public Object, public DrawningSurface
{
public:
    Widget(Widget *parent = nullptr);
    virtual ~Widget();

    enum Properties : BYTE {
        HSizeBehavior,
        VSizeBehavior,
        PROPERTIES_COUNT
    };

    enum SizeBehavior : BYTE {
        Fixed,
        Expanding,
        //Preferred
    };

    virtual void setGeometry(int, int, int, int);
    virtual void move(int, int);
    virtual void resize(int, int);
    void setDisabled(bool);
    void close();
    Widget* parentWidget();
    tstring title();
    Size size();
    void size(int*, int*);
    void setWindowTitle(const tstring &title);
    void setProperty(Properties, int);
    void show();
    void hide();
    void repaint();
    void update();
    void setLayout(Layout *lut);
    bool isCreated();
    bool underMouse();
    int  property(Properties);
    Layout* layout();
    WindowHandle nativeWindowHandle();
    static Widget* widgetFromHwnd(Widget *parent, WindowHandle);

    /* callback */
    int onResize(const FnVoidIntInt &callback);
    int onMove(const FnVoidIntInt &callback);
    int onAboutToDestroy(const FnVoidVoid &callback);
    int onCreate(const FnVoidVoid &callback);
    int onClose(const FnVoidBoolPtr &callback);

    virtual void disconnect(int) override;

protected:
    friend class Application;
    Widget(Widget *parent, WindowHandle);
    Widget(Widget *parent, ObjectType type, const Rect &rc = Rect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT));
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*);
#else
    virtual bool event(GdkEventType ev_type, void *param);
#endif

    WindowHandle m_hWnd;
    Layout      *m_layout;
    tstring      m_title;
    bool         m_disabled;

private:
#ifdef __linux
    friend class Widget;
    virtual GtkWidget *gtkLayout();
    Size m_size;
#endif
    void setNativeWindowHandle(WindowHandle);

    int m_properties[PROPERTIES_COUNT];
    std::unordered_map<int, FnVoidIntInt> m_resize_callbacks,
                                          m_move_callbacks;
    std::unordered_map<int, FnVoidVoid>   m_create_callbacks,
                                          m_destroy_callbacks;
    std::unordered_map<int, FnVoidBoolPtr> m_close_callbacks;

    bool    m_is_created,
            m_is_destroyed,
            m_is_class_destroyed,
            m_mouse_entered;
};

#endif // WIDGET_H

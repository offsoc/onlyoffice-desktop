#ifndef APPLICATION_H
#define APPLICATION_H

#include "object.h"
#include "common.h"
#ifdef _WIN32
# include <Windows.h>
#else
# include <gtk/gtk.h>
#endif

class Widget;
class Application : public Object
{
public:
#ifdef _WIN32
    Application(HINSTANCE hInstance, PWSTR cmdline, int cmdshow);
#else
    Application(int argc, char *argv[]);
#endif
    Application(const Application&) = delete;
    ~Application();

    Application& operator=(const Application&) = delete;
    static Application *instance();
#ifdef _WIN32
    HINSTANCE moduleHandle();
#endif
    void setLayoutDirection(LayoutDirection);

    int exec();
    void exit(int);

private:
    Application();
    friend class Widget;
    void registerWidget(Widget*, ObjectType, const Rect &rc);
    class ApplicationPrivate;
    ApplicationPrivate *d_ptr;
    static Application *inst;
};

#endif // APPLICATION_H

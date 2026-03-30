#include "fourtitudecpp.h"
#include "fourtitudec17.h"

namespace Fourtitude
{
    FourtitudeWindow::FourtitudeWindow()
    {
        running = false;
    }

    FourtitudeWindow::~FourtitudeWindow()
    {
        destroy();
    }

    void FourtitudeWindow::create(int width, int height, const char* title, int flags)
    {
        win.width  = width;
        win.height = height;
        win.title  = title;
        FOUR_Window_create(&win, width, height, title, flags);
        running = true;
    }

    void FourtitudeWindow::destroy()
    {
        if (!running) return;
        FOUR_Window_destroy(&win);
        running = false;
    }

    void FourtitudeWindow::pollEvents()
    {
        if (!running) return;
        FOUR_Window_pollEvents(&win);
        if (win.should_close) running = false;
    }

    bool FourtitudeWindow::shouldClose() const
    {
        if (!running) return true;
        return FOUR_Window_shouldClose(const_cast<FOUR_Window*>(&win));
    }
}

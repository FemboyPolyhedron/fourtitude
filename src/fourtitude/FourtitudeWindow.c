#include "fourtitudec17.h"

#ifdef _WIN32

#include <windows.h>
#include <shellapi.h>
#include <stdbool.h>
#include <string.h>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    FOUR_Window* self = (FOUR_Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
        case WM_NCCREATE:
        {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            if (cs) SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            break;
        }

        case WM_NCHITTEST:
        {
            if (!self || !(self->flags & FOUR_WINDOW_TOPLESS)) break;

            POINT pt = { (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam) };
            ScreenToClient(hwnd, &pt);

            RECT rc;
            GetClientRect(hwnd, &rc);
            
            const int border = 8; 

            bool left = pt.x < border;
            bool right = pt.x > rc.right - border;
            bool top = pt.y < border;
            bool bottom = pt.y > rc.bottom - border;

            if (top && left) return HTTOPLEFT;
            if (top && right) return HTTOPRIGHT;
            if (bottom && left) return HTBOTTOMLEFT;
            if (bottom && right) return HTBOTTOMRIGHT;

            if (left) return HTLEFT;
            if (right) return HTRIGHT;
            if (top) return HTTOP;
            if (bottom) return HTBOTTOM;

            if (pt.y < 28)
            {
                if (self->form && self->form->documents) for (int d = 0; self->form->documents[d].elements != NULL; d++) for (int i = 0; i < self->form->documents[d].element_count; i++)
                {
                    FOUR_Element* el = self->form->documents[d].elements[i];
                    if (!el || !el->enabled || !el->type || strcmp(el->type, "button") != 0) continue;
                    if (pt.x >= el->x && pt.x < el->x + el->width &&
                        pt.y >= el->y && pt.y < el->y + el->height) return HTCLIENT;
                }
                return HTCAPTION;
            }

            return HTCLIENT;
        }

        case WM_NCCALCSIZE:
        {
            if (wParam && self && (self->flags & FOUR_WINDOW_TOPLESS)) return 0;
            break;
        }

        case WM_GETMINMAXINFO:
        {
            if (self && (self->flags & (FOUR_WINDOW_BORDERLESS | FOUR_WINDOW_TOPLESS)))
            {
                MINMAXINFO* mmi = (MINMAXINFO*)lParam;
                HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi = { sizeof(mi) };
                if (GetMonitorInfo(hMonitor, &mi))
                {
                    mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
                    mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
                    mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
                    mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
                }
                return 0;
            }
            break;
        }

        case WM_SETCURSOR:
        {
            if (!self || LOWORD(lParam) != HTCLIENT) break;
            if (!self->form || !self->form->documents) break;

            int mx = self->mouse_x, my = self->mouse_y;
            FCS_Cursor cur = FCS_CURSOR_DEFAULT;
            const char* cur_img = NULL;
            void** cur_cache = NULL;
            bool found = false;
            int best_area = INT_MAX;

            for (int d = 0; self->form->documents[d].elements != NULL; d++) for (int i = 0; i < self->form->documents[d].element_count; i++)
            {
                FOUR_Element* el = self->form->documents[d].elements[i];
                if (!el || !el->enabled || !el->computed_style) continue;
                if ((el->computed_style->flags & FCS_F_DISPLAY) && el->computed_style->display == FCS_DISPLAY_NONE) continue;
                if (!(el->computed_style->flags & FCS_F_CURSOR)) continue;
                if (mx < el->x || mx >= el->x + el->width) continue;
                if (my < el->y || my >= el->y + el->height) continue;
                int area = el->width * el->height;
                if (area >= best_area) continue;
                cur       = el->computed_style->cursor;
                cur_img   = el->computed_style->cursor_image;
                cur_cache = &el->computed_style->cursor_cache;
                best_area = area;
                found = true;
            }

            if (!found) break;

            if (cur == FCS_CURSOR_NONE) { SetCursor(NULL); return TRUE; }
            if (cur == FCS_CURSOR_CUSTOM && cur_img)
            {
                HCURSOR hc = *cur_cache ? (HCURSOR)*cur_cache : LoadCursorFromFileA(cur_img);
                if (hc) { *cur_cache = hc; SetCursor(hc); return TRUE; }
                break;
            }

            LPCSTR id = IDC_ARROW;
            if (cur == FCS_CURSOR_POINTER) id = IDC_HAND;
            else if (cur == FCS_CURSOR_TEXT) id = IDC_IBEAM;
            else if (cur == FCS_CURSOR_MOVE) id = IDC_SIZEALL;
            else if (cur == FCS_CURSOR_CROSSHAIR) id = IDC_CROSS;
            else if (cur == FCS_CURSOR_WAIT) id = IDC_WAIT;
            else if (cur == FCS_CURSOR_HELP) id = IDC_HELP;
            else if (cur == FCS_CURSOR_NOT_ALLOWED) id = IDC_NO;
            else if (cur == FCS_CURSOR_EW_RESIZE) id = IDC_SIZEWE;
            else if (cur == FCS_CURSOR_NS_RESIZE) id = IDC_SIZENS;
            else if (cur == FCS_CURSOR_GRAB) id = IDC_SIZEALL;
            SetCursor(LoadCursor(NULL, id));
            return TRUE;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (self && self->render_dc)
            {
                BitBlt(hdc, 0, 0, self->width, self->height, self->render_dc, 0, 0, SRCCOPY);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE:
        {
            if (!self) return 0;
            int w = (int)LOWORD(lParam);
            int h = (int)HIWORD(lParam);
            if (w <= 0 || h <= 0) return 0;

            self->width  = w;
            self->height = h;

            if (self->render_dc)  { DeleteDC(self->render_dc); }
            if (self->render_bmp) { DeleteObject(self->render_bmp); }
            
            HDC screen_dc = GetDC(hwnd);
            self->render_dc  = CreateCompatibleDC(screen_dc);
            self->render_bmp = CreateCompatibleBitmap(screen_dc, w, h);
            SelectObject(self->render_dc, self->render_bmp);
            
            RECT rect = { 0, 0, w, h };
            FillRect(self->render_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            ReleaseDC(hwnd, screen_dc);
            
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (self)
            {
                self->mouse_x = (int)(short)LOWORD(lParam);
                self->mouse_y = (int)(short)HIWORD(lParam);
            }
            break;
        }

        case WM_LBUTTONDOWN:
        {
            if (self)
            {
                self->mouse_down = 1;
                SetCapture(hwnd);
            }
            break;
        }

        case WM_LBUTTONUP:
        {
            if (self)
            {
                self->mouse_down = 0;
                self->mouse_clicked = 1;
                ReleaseCapture();
            }
            break;
        }

        case WM_DROPFILES:
        {
            if (self)
            {
                HDROP hDrop = (HDROP)wParam;
                DragQueryFileA(hDrop, 0, self->dropped_file, sizeof(self->dropped_file));
                DragFinish(hDrop);
            }
            return 0;
        }

        case WM_CLOSE:
        {
            if (self) self->should_close = 1;
            return 0;
        }

        case WM_DESTROY:
            if (self) self->should_close = 1;
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void FOUR_Window_create(FOUR_Window* self, int width, int height, const char* title, int flags)
{
    self->flags    = flags;
    self->instance = GetModuleHandle(NULL);

    const char* className = "FOUR_WindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = self->instance;
    wc.lpszClassName = className;
    RegisterClass(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = 0;
    int x = CW_USEDEFAULT, y = CW_USEDEFAULT, w = width, h = height;

    if (flags & FOUR_WINDOW_FULLSCREEN)
    {
        style = WS_POPUP;
        x = 0;
        y = 0;
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);
        self->width  = w;
        self->height = h;
    }
    else if (flags & FOUR_WINDOW_TOPLESS)
    {
        style = WS_OVERLAPPEDWINDOW;
    }
    else if (flags & FOUR_WINDOW_BORDERLESS)
    {
        style = WS_POPUP;
    }

    if (flags & FOUR_WINDOW_TOPMOST) ex_style |= WS_EX_TOPMOST;

    self->handle = CreateWindowEx(
        ex_style, className, title, style,
        x, y, w, h,
        NULL, NULL, self->instance, (LPVOID)self
    );

    SetWindowLongPtr(self->handle, GWLP_USERDATA, (LONG_PTR)self);

    if (flags & FOUR_WINDOW_TOPLESS) SetWindowPos(self->handle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    HDC screen_dc = GetDC(self->handle);
    RECT rect;
    GetClientRect(self->handle, &rect);
    if (rect.right  == 0) rect.right  = w;
    if (rect.bottom == 0) rect.bottom = h;
    self->render_dc  = CreateCompatibleDC(screen_dc);
    self->render_bmp = CreateCompatibleBitmap(screen_dc, rect.right, rect.bottom);
    SelectObject(self->render_dc, self->render_bmp);
    FillRect(self->render_dc, &rect, GetStockObject(WHITE_BRUSH));
    ReleaseDC(self->handle, screen_dc);

    DragAcceptFiles(self->handle, TRUE);
    self->should_close = 0;
    if (!(flags & FOUR_WINDOW_HIDDEN)) ShowWindow(self->handle, SW_SHOW);
}

void FOUR_Window_setPos(FOUR_Window* self, int x, int y)
{
    SetWindowPos(self->handle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void FOUR_Window_center(FOUR_Window* self)
{
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    RECT rect;
    GetWindowRect(self->handle, &rect);
    int w = rect.right  - rect.left;
    int h = rect.bottom - rect.top;
    SetWindowPos(self->handle, NULL, (screen_w - w) / 2, (screen_h - h) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void FOUR_Window_destroy(FOUR_Window* self)
{
    if (self->render_dc) { DeleteDC(self->render_dc); self->render_dc = NULL; }
    if (self->render_bmp) { DeleteObject(self->render_bmp); self->render_bmp = NULL; }
    if (self->handle) { DestroyWindow(self->handle); self->handle = NULL; }
}

void FOUR_Window_pollEvents(FOUR_Window* self)
{
    self->last_key = 0;
    self->key_down = 0;
    self->mouse_clicked = 0;
    self->last_char = 0;

    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT) { self->should_close = 1; return; }
        if (msg.message == WM_CHAR) self->last_char = (int)msg.wParam;
        if (msg.message == WM_MOUSEMOVE) { self->mouse_x = (int)LOWORD(msg.lParam); self->mouse_y = (int)HIWORD(msg.lParam); }
        if (msg.message == WM_LBUTTONDOWN) self->mouse_down = 1;
        if (msg.message == WM_LBUTTONUP) { self->mouse_down = 0; self->mouse_clicked = 1; }
        if (msg.message == WM_KEYDOWN) { self->last_key = (int)msg.wParam; self->key_down = 1; }
        if (msg.message == WM_KEYUP) { self->last_key = (int)msg.wParam; self->key_down = 0; }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int FOUR_Window_shouldClose(FOUR_Window* self)
{
    return self->should_close;
}

#else

#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>

void FOUR_Window_create(FOUR_Window* self, int width, int height, const char* title, int flags)
{
    self->flags = flags;
    self->display = XOpenDisplay(NULL);
    self->screen = DefaultScreen(self->display);

    int w = width, h = height;

    if (flags & FOUR_WINDOW_FULLSCREEN)
    {
        w = DisplayWidth(self->display, self->screen);
        h = DisplayHeight(self->display, self->screen);
        self->width  = w;
        self->height = h;
    }

    self->handle = XCreateSimpleWindow(
        self->display,
        RootWindow(self->display, self->screen),
        0, 0,
        (unsigned int)w,
        (unsigned int)h,
        (flags & FOUR_WINDOW_BORDERLESS) ? 0 : 1,
        BlackPixel(self->display, self->screen),
        WhitePixel(self->display, self->screen)
    );

    if (flags & FOUR_WINDOW_BORDERLESS)
    {
        struct { unsigned long flags, functions, decorations; long input_mode; unsigned long status; } hints = {2, 0, 0, 0, 0};
        Atom motif = XInternAtom(self->display, "_MOTIF_WM_HINTS", False);
        XChangeProperty(self->display, self->handle, motif, motif, 32, PropModeReplace, (unsigned char*)&hints, 5);
    }

    if (flags & FOUR_WINDOW_TOPMOST)
    {
        Atom above = XInternAtom(self->display, "_NET_WM_STATE_ABOVE", False);
        Atom state = XInternAtom(self->display, "_NET_WM_STATE", False);
        XChangeProperty(self->display, self->handle, state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&above, 1);
    }

    XStoreName(self->display, self->handle, title);
    XSelectInput(self->display, self->handle, ExposureMask | KeyPressMask);
    if (!(flags & FOUR_WINDOW_HIDDEN)) XMapWindow(self->display, self->handle);
}

void FOUR_Window_setPos(FOUR_Window* self, int x, int y)
{
    XMoveWindow(self->display, self->handle, x, y);
}

void FOUR_Window_center(FOUR_Window* self)
{
    int screen_w = DisplayWidth(self->display, self->screen);
    int screen_h = DisplayHeight(self->display, self->screen);
    XMoveWindow(self->display, self->handle, (screen_w - self->width) / 2, (screen_h - self->height) / 2);
}

void FOUR_Window_destroy(FOUR_Window* self)
{
    if (self->display)
    {
        XDestroyWindow(self->display, self->handle);
        XCloseDisplay(self->display);
        self->display = NULL;
    }
}

void FOUR_Window_pollEvents(FOUR_Window* self)
{
    self->last_key     = 0;
    self->key_down     = 0;
    self->mouse_clicked = 0;

    XSelectInput(self->display, self->handle, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
    XEvent event;
    while (XPending(self->display))
    {
        XNextEvent(self->display, &event);
        if (event.type == MotionNotify)  { self->mouse_x = event.xmotion.x; self->mouse_y = event.xmotion.y; }
        if (event.type == ButtonPress)   self->mouse_down = 1;
        if (event.type == ButtonRelease) { self->mouse_down = 0; self->mouse_clicked = 1; }
        if (event.type == KeyPress)      { self->last_key = (int)XLookupKeysym(&event.xkey, 0); self->key_down = 1; }
        if (event.type == KeyRelease)    { self->last_key = (int)XLookupKeysym(&event.xkey, 0); self->key_down = 0; }
        if (event.type == ConfigureNotify && (event.xconfigure.width != self->width || event.xconfigure.height != self->height))
        {
            self->width  = event.xconfigure.width;
            self->height = event.xconfigure.height;
        }
    }
}

int FOUR_Window_shouldClose(FOUR_Window* self)
{
    return 0;
}

#endif
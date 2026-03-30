#include "fourtitudec17.h"
#include "FourtitudeStyle.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static FOUR_Logger* s_elem_log = NULL;
#define ELEM_LOG() (s_elem_log ? s_elem_log : (s_elem_log = FOUR_GetLogger("Elements")))

static int is_hidden(FOUR_Element* self)
{
    FCS_ComputedStyle* s = self->computed_style;
    if (!s) return 0;
    return (s->flags & FCS_F_DISPLAY) && s->display == FCS_DISPLAY_NONE;
}

#ifdef _WIN32

#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

static HBITMAP wic_load(const char* path)
{
    if (!path || !path[0]) { FOUR_WARN(ELEM_LOG(), "wic_load: null/empty path"); return NULL; }

    int wlen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    if (wlen <= 0) { FOUR_WARN(ELEM_LOG(), "wic_load: MultiByteToWideChar failed for: %s", path); return NULL; }
    wchar_t* wpath = (wchar_t*)malloc((size_t)wlen * sizeof(wchar_t));
    if (!wpath) { FOUR_ERROR(ELEM_LOG(), "wic_load: out of memory"); return NULL; }
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, wlen);

    CoInitialize(NULL);

    IWICImagingFactory* factory = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (void**)&factory);
    if (FAILED(hr) || !factory) { FOUR_ERROR(ELEM_LOG(), "wic_load: CoCreateInstance failed hr=0x%08lX path=%s", hr, path); free(wpath); return NULL; }

    IWICBitmapDecoder* decoder = NULL;
    hr = factory->lpVtbl->CreateDecoderFromFilename(factory, wpath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    free(wpath);
    if (FAILED(hr) || !decoder) { FOUR_ERROR(ELEM_LOG(), "wic_load: CreateDecoderFromFilename failed hr=0x%08lX path=%s", hr, path); factory->lpVtbl->Release(factory); return NULL; }

    IWICBitmapFrameDecode* frame = NULL;
    hr = decoder->lpVtbl->GetFrame(decoder, 0, &frame);
    if (FAILED(hr) || !frame) { FOUR_ERROR(ELEM_LOG(), "wic_load: GetFrame failed hr=0x%08lX", hr); decoder->lpVtbl->Release(decoder); factory->lpVtbl->Release(factory); return NULL; }

    IWICFormatConverter* converter = NULL;
    hr = factory->lpVtbl->CreateFormatConverter(factory, &converter);
    if (FAILED(hr) || !converter) { FOUR_ERROR(ELEM_LOG(), "wic_load: CreateFormatConverter failed hr=0x%08lX", hr); frame->lpVtbl->Release(frame); decoder->lpVtbl->Release(decoder); factory->lpVtbl->Release(factory); return NULL; }

    hr = converter->lpVtbl->Initialize(converter, (IWICBitmapSource*)frame, &GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) { FOUR_ERROR(ELEM_LOG(), "wic_load: converter Initialize failed hr=0x%08lX", hr); converter->lpVtbl->Release(converter); frame->lpVtbl->Release(frame); decoder->lpVtbl->Release(decoder); factory->lpVtbl->Release(factory); return NULL; }

    UINT w = 0, h = 0;
    converter->lpVtbl->GetSize(converter, &w, &h);
    if (w == 0 || h == 0) { FOUR_WARN(ELEM_LOG(), "wic_load: image has zero size: %s", path); converter->lpVtbl->Release(converter); frame->lpVtbl->Release(frame); decoder->lpVtbl->Release(decoder); factory->lpVtbl->Release(factory); return NULL; }

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = (LONG)w;
    bmi.bmiHeader.biHeight = -(LONG)h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = NULL;
    HBITMAP bmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!bmp || !bits) { FOUR_ERROR(ELEM_LOG(), "wic_load: CreateDIBSection failed for: %s", path); converter->lpVtbl->Release(converter); frame->lpVtbl->Release(frame); decoder->lpVtbl->Release(decoder); factory->lpVtbl->Release(factory); return NULL; }

    UINT stride = w * 4;
    UINT size = stride * h;
    hr = converter->lpVtbl->CopyPixels(converter, NULL, stride, size, (BYTE*)bits);
    if (FAILED(hr)) { FOUR_ERROR(ELEM_LOG(), "wic_load: CopyPixels failed hr=0x%08lX", hr); DeleteObject(bmp); converter->lpVtbl->Release(converter); frame->lpVtbl->Release(frame); decoder->lpVtbl->Release(decoder); factory->lpVtbl->Release(factory); return NULL; }

    converter->lpVtbl->Release(converter);
    frame->lpVtbl->Release(frame);
    decoder->lpVtbl->Release(decoder);
    factory->lpVtbl->Release(factory);

    FOUR_INFO(ELEM_LOG(), "wic_load: loaded %ux%u: %s", w, h, path);
    return bmp;
}

typedef struct { HBITMAP bmp; int w, h; } CachedImage;

static CachedImage* img_ensure(void** cache_slot, const char* path)
{
    if (*cache_slot) return (CachedImage*)*cache_slot;
    if (!path || !*path) return NULL;
    HBITMAP bmp = wic_load(path);
    if (!bmp) return NULL;
    BITMAP bm = {0};
    GetObject(bmp, sizeof(bm), &bm);
    CachedImage* ci = (CachedImage*)malloc(sizeof(CachedImage));
    ci->bmp = bmp;
    ci->w = bm.bmWidth;
    ci->h = bm.bmHeight;
    *cache_slot = ci;
    return ci;
}

static void img_free_cache(void** cache_slot)
{
    if (!*cache_slot) return;
    CachedImage* ci = (CachedImage*)*cache_slot;
    DeleteObject(ci->bmp);
    free(ci);
    *cache_slot = NULL;
}

void FOUR_FreeImageCache(void** cache_slot) { img_free_cache(cache_slot); }

static void draw_image(HDC hdc, CachedImage* ci, int x, int y, int w, int h)
{
    if (!ci) return;
    if (w <= 0) w = ci->w;
    if (h <= 0) h = ci->h;
    HDC src = CreateCompatibleDC(hdc);
    HGDIOBJ old = SelectObject(src, ci->bmp);
    SetStretchBltMode(hdc, HALFTONE);
    SetBrushOrgEx(hdc, 0, 0, NULL);
    StretchBlt(hdc, x, y, w, h, src, 0, 0, ci->w, ci->h, SRCCOPY);
    SelectObject(src, old);
    DeleteDC(src);
}

static COLORREF fcs_colorref(FCS_Color c) { return RGB(c.r, c.g, c.b); }

static HRGN create_rounded_rgn(int x, int y, int w, int h, FCS_BorderRadius r)
{
    int tl = r.top_left, tr = r.top_right, bl = r.bottom_left, br = r.bottom_right;
    if (tl == tr && tl == bl && tl == br) return tl > 0 ? CreateRoundRectRgn(x, y, x+w, y+h, tl*2, tl*2) : CreateRectRgn(x, y, x+w, y+h);

    HRGN rgn = CreateRectRgn(x, y, x+w, y+h);
    if (tl > 0)
    {
        HRGN cr = CreateRectRgn(x, y, x+tl, y+tl);
        HRGN er = CreateEllipticRgn(x, y, x+tl*2, y+tl*2);
        CombineRgn(cr, cr, er, RGN_DIFF); CombineRgn(rgn, rgn, cr, RGN_DIFF);
        DeleteObject(cr); DeleteObject(er);
    }
    if (tr > 0) 
    {
        HRGN cr = CreateRectRgn(x+w-tr, y, x+w, y+tr);
        HRGN er = CreateEllipticRgn(x+w-tr*2, y, x+w, y+tr*2);
        CombineRgn(cr, cr, er, RGN_DIFF); CombineRgn(rgn, rgn, cr, RGN_DIFF);
        DeleteObject(cr); DeleteObject(er);
    }
    if (bl > 0) 
    {
        HRGN cr = CreateRectRgn(x, y+h-bl, x+bl, y+h);
        HRGN er = CreateEllipticRgn(x, y+h-bl*2, x+bl*2, y+h);
        CombineRgn(cr, cr, er, RGN_DIFF); CombineRgn(rgn, rgn, cr, RGN_DIFF);
        DeleteObject(cr); DeleteObject(er);
    }
    if (br > 0) 
    {
        HRGN cr = CreateRectRgn(x+w-br, y+h-br, x+w, y+h);
        HRGN er = CreateEllipticRgn(x+w-br*2, y+h-br*2, x+w, y+h);
        CombineRgn(cr, cr, er, RGN_DIFF); CombineRgn(rgn, rgn, cr, RGN_DIFF);
        DeleteObject(cr); DeleteObject(er);
    }
    return rgn;
}

static void fill_bg(HDC hdc, int x, int y, int w, int h, FCS_BorderRadius radius, FCS_Color c)
{
    if (w <= 0 || h <= 0 || c.a == 0) return;
    int any_r = radius.top_left | radius.top_right | radius.bottom_left | radius.bottom_right;

    if (c.a == 255)
    {
        HBRUSH br = CreateSolidBrush(fcs_colorref(c));
        if (any_r)
        {
            HRGN rgn = create_rounded_rgn(x, y, w, h, radius);
            FillRgn(hdc, rgn, br);
            DeleteObject(rgn);
        }
        else
        {
            RECT r = {x, y, x + w, y + h};
            FillRect(hdc, &r, br);
        }
        DeleteObject(br);
        return;
    }

    BITMAPINFOHEADER bih = {sizeof(bih), w, -h, 1, 32, BI_RGB, 0, 0, 0, 0, 0};
    void* bits = NULL;
    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!bmp) { DeleteDC(mem); return; }
    HGDIOBJ old = SelectObject(mem, bmp);

    uint8_t pr = (uint8_t)((int)c.r * c.a / 255);
    uint8_t pg = (uint8_t)((int)c.g * c.a / 255);
    uint8_t pb = (uint8_t)((int)c.b * c.a / 255);
    uint32_t pix = ((uint32_t)c.a << 24) | ((uint32_t)pr << 16) | ((uint32_t)pg << 8) | pb;
    uint32_t* px = (uint32_t*)bits;
    for (int i = 0; i < w * h; i++) px[i] = pix;

    if (any_r)
    {
        HRGN full    = CreateRectRgn(0, 0, w, h);
        HRGN rounded = create_rounded_rgn(0, 0, w, h, radius);
        CombineRgn(full, full, rounded, RGN_DIFF);
        DWORD sz = GetRegionData(full, 0, NULL);
        RGNDATA* rd = (RGNDATA*)malloc(sz);
        if (rd)
        {
            GetRegionData(full, sz, rd);
            RECT* rects = (RECT*)rd->Buffer;
            for (DWORD ri = 0; ri < rd->rdh.nCount; ri++) for (int py = rects[ri].top; py < rects[ri].bottom; py++) for (int qx = rects[ri].left; qx < rects[ri].right; qx++) px[py * w + qx] = 0;
            free(rd);
        }
        DeleteObject(full);
        DeleteObject(rounded);
    }

    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    AlphaBlend(hdc, x, y, w, h, mem, 0, 0, w, h, bf);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
}

static FCS_ComputedStyle get_effective_style(FOUR_Element* self)
{
    FCS_ComputedStyle base = self->computed_style ? *self->computed_style : (FCS_ComputedStyle){0};
    FCS_ComputedStyle* ov = NULL;
    if (self->active && self->active_style) ov = self->active_style;
    else if (self->hovered && self->hover_style) ov = self->hover_style;
    if (!ov) return base;

    if (ov->flags & FCS_F_COLOR) { base.color = ov->color; base.flags |= FCS_F_COLOR; }
    if (ov->flags & FCS_F_BG_COLOR) { base.bg_color = ov->bg_color; base.flags |= FCS_F_BG_COLOR; }
    if (ov->flags & FCS_F_BG_GRADIENT) { base.bg_gradient = ov->bg_gradient; base.flags |= FCS_F_BG_GRADIENT; }
    if (ov->flags & FCS_F_FONT_FAMILY) { base.font_family = ov->font_family; base.flags |= FCS_F_FONT_FAMILY; }
    if (ov->flags & FCS_F_FONT_WEIGHT) { base.font_weight = ov->font_weight; base.flags |= FCS_F_FONT_WEIGHT; }
    if (ov->flags & FCS_F_FONT_SIZE) { base.font_size = ov->font_size; base.flags |= FCS_F_FONT_SIZE; }
    if (ov->flags & FCS_F_TEXT_ALIGN) { base.text_align = ov->text_align; base.flags |= FCS_F_TEXT_ALIGN; }
    if (ov->flags & FCS_F_BORDER) { base.border = ov->border; base.flags |= FCS_F_BORDER; }
    if (ov->flags & FCS_F_BORDER_RADIUS) { base.border_radius = ov->border_radius; base.flags |= FCS_F_BORDER_RADIUS; }
    if (ov->flags & FCS_F_BORDER_TOP) { base.border_top = ov->border_top; base.flags |= FCS_F_BORDER_TOP; }
    if (ov->flags & FCS_F_BORDER_RIGHT) { base.border_right = ov->border_right; base.flags |= FCS_F_BORDER_RIGHT; }
    if (ov->flags & FCS_F_BORDER_BOTTOM) { base.border_bottom = ov->border_bottom; base.flags |= FCS_F_BORDER_BOTTOM; }
    if (ov->flags & FCS_F_BORDER_LEFT) { base.border_left = ov->border_left; base.flags |= FCS_F_BORDER_LEFT; }
    if (ov->flags & FCS_F_BOX_SHADOW) { base.box_shadow = ov->box_shadow; base.flags |= FCS_F_BOX_SHADOW; }
    if (ov->flags & FCS_F_DISPLAY) { base.display = ov->display; base.flags |= FCS_F_DISPLAY; }
    if (ov->flags & FCS_F_PADDING) { base.padding = ov->padding; base.flags |= FCS_F_PADDING; }
    if (ov->flags & FCS_F_IMAGE) { base.image = ov->image; base.image_cache = ov->image_cache; base.flags |= FCS_F_IMAGE; }
    if (ov->flags & FCS_F_BG_IMAGE) { base.bg_image = ov->bg_image; base.bg_image_cache = ov->bg_image_cache; base.flags |= FCS_F_BG_IMAGE; }
    return base;
}

static void draw_styled_bg_border(FOUR_Element* self, HDC hdc)
{
    FCS_ComputedStyle _eff = get_effective_style(self);
    FCS_ComputedStyle* s = &_eff;
    FCS_BorderRadius r = (s->flags & FCS_F_BORDER_RADIUS) ? s->border_radius : (FCS_BorderRadius){0,0,0,0};
    
    if (s->flags & FCS_F_BG_COLOR)
        fill_bg(hdc, self->x, self->y, self->width, self->height, r, s->bg_color);
        
    if (s->flags & FCS_F_BG_IMAGE)
    {
        CachedImage* ci = img_ensure(&s->bg_image_cache, s->bg_image);
        draw_image(hdc, ci, self->x, self->y, self->width, self->height);
    }

    if ((s->flags & FCS_F_BORDER) && s->border.style != FCS_BORDER_NONE)
    {
        int bw = s->border.width;
        if (bw <= 0) bw = 1;

        HRGN outer = create_rounded_rgn(self->x, self->y, self->width, self->height, r);
        HBRUSH br = CreateSolidBrush(fcs_colorref(s->border.color));
        FrameRgn(hdc, outer, br, bw, bw);
        DeleteObject(br);
        DeleteObject(outer);
    }

    struct { uint32_t flag; FCS_Border* b; int x1,y1,x2,y2; } sides[4] = {
        { FCS_F_BORDER_TOP,    &s->border_top,    self->x, self->y, self->x + self->width, self->y },
        { FCS_F_BORDER_RIGHT, &s->border_right, self->x + self->width - 1,self->y, self->x + self->width - 1, self->y + self->height },
        { FCS_F_BORDER_BOTTOM, &s->border_bottom, self->x, self->y + self->height - 1,self->x + self->width, self->y + self->height - 1 },
        { FCS_F_BORDER_LEFT, &s->border_left, self->x, self->y, self->x, self->y + self->height },
    };
    for (int i = 0; i < 4; i++)
    {
        if (!(s->flags & sides[i].flag)) continue;
        if (sides[i].b->style == FCS_BORDER_NONE) continue;
        int bw = sides[i].b->width > 0 ? sides[i].b->width : 1;
        HPEN pen = CreatePen(PS_SOLID, bw, fcs_colorref(sides[i].b->color));
        HPEN old = SelectObject(hdc, pen);
        MoveToEx(hdc, sides[i].x1, sides[i].y1, NULL);
        LineTo(hdc, sides[i].x2, sides[i].y2);
        SelectObject(hdc, old);
        DeleteObject(pen);
    }
}

static void draw_text_scaled(FOUR_Element* self, FOUR_Window* window, int point_size, int default_weight)
{
    if (is_hidden(self)) return;

    FCS_ComputedStyle _eff    = get_effective_style(self);
    FCS_ComputedStyle* s = &_eff;
    const char* content = self->content ? self->content : "";
    HDC hdc = window->render_dc;

    draw_styled_bg_border(self, hdc);

    const char* family = (s && (s->flags & FCS_F_FONT_FAMILY) && s->font_family) ? s->font_family : "Arial";
    int weight = (s && (s->flags & FCS_F_FONT_WEIGHT)) ? s->font_weight : default_weight;
    int size = (s && (s->flags & FCS_F_FONT_SIZE)) ? s->font_size : point_size;
    HFONT font = CreateFontA(-MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, family);
    HFONT old_font = (HFONT)SelectObject(hdc, font);

    if (s && (s->flags & FCS_F_COLOR)) SetTextColor(hdc, fcs_colorref(s->color));

    RECT rect = { self->x, self->y, self->x + self->width, self->y + self->height };
    if (s && (s->flags & FCS_F_PADDING))
    {
        rect.left += s->padding.left;
        rect.top += s->padding.top;
        rect.right -= s->padding.right;
        rect.bottom -= s->padding.bottom;
    }

    UINT dt_flags = DT_TOP | DT_WORDBREAK;
    if (s && (s->flags & FCS_F_TEXT_ALIGN))
    {
        switch (s->text_align)
        {
            case FCS_TEXTALIGN_RIGHT: dt_flags |= DT_RIGHT; break;
            case FCS_TEXTALIGN_CENTER: dt_flags |= DT_CENTER; break;
            default: dt_flags |= DT_LEFT; break;
        }
    }
    else dt_flags |= DT_LEFT;

    SetBkMode(hdc, TRANSPARENT);
    DrawTextA(hdc, content, -1, &rect, dt_flags);
    SelectObject(hdc, old_font);
    DeleteObject(font);
}

#else

static void draw_styled_bg_border(FOUR_Element* self, void* gc_unused) { (void)self; (void)gc_unused; }

static void draw_text_scaled(FOUR_Element* self, FOUR_Window* window, int point_size, int default_weight)
{
    if (is_hidden(self)) return;
    (void)point_size; (void)default_weight;
    const char* content = self->content ? self->content : "";
    GC gc = XCreateGC(window->display, window->handle, 0, NULL);
    XDrawString(window->display, window->handle, gc, self->x, self->y + self->height, content, (int)strlen(content));
    XFreeGC(window->display, gc);
}

#endif

FOUR_DEFINE_ELEMENT(container, "container")
{
    if (is_hidden(self)) return;
    #ifdef _WIN32
    HDC hdc = window->render_dc;
    RECT rect = { self->x, self->y, self->x + self->width, self->y + self->height };
    draw_styled_bg_border(self, hdc);
    if (!self->computed_style) Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    #else
    GC gc = XCreateGC(window->display, window->handle, 0, NULL);
    XDrawRectangle(window->display, window->handle, gc, self->x, self->y, (unsigned)self->width, (unsigned)self->height);
    XFreeGC(window->display, gc);
    #endif
}

void FOUR_Element_initContainer(FOUR_Element* element) { FOUR_Element_init_container(element); }

FOUR_DEFINE_ELEMENT(text, "text")
{
    if (is_hidden(self)) return;
    const char* content = self->content ? self->content : "";
    #ifdef _WIN32
    HDC hdc = window->render_dc;
    {
    FCS_ComputedStyle _teff = get_effective_style(self);
    draw_styled_bg_border(self, hdc);
    if (_teff.flags & FCS_F_COLOR) SetTextColor(hdc, fcs_colorref(_teff.color));
    SetBkMode(hdc, TRANSPARENT);
    RECT rect = { self->x, self->y, self->x + self->width, self->y + self->height };
    DrawTextA(hdc, content, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }
    #else
    GC gc = XCreateGC(window->display, window->handle, 0, NULL);
    XDrawString(window->display, window->handle, gc, self->x, self->y + self->height, content, (int)strlen(content));
    XFreeGC(window->display, gc);
    #endif
}

void FOUR_Element_initText(FOUR_Element* element) { FOUR_Element_init_text(element); }

FOUR_DEFINE_ELEMENT(button, "button")
{
    if (is_hidden(self)) return;
    const char* content = self->content ? self->content : "";
    #ifdef _WIN32
    HDC hdc = window->render_dc;
    RECT rect = { self->x, self->y, self->x + self->width, self->y + self->height };
    FCS_ComputedStyle _beff = get_effective_style(self);
    FCS_ComputedStyle* beff = &_beff;
    draw_styled_bg_border(self, hdc);
    if (beff->flags & FCS_F_COLOR) SetTextColor(hdc, fcs_colorref(beff->color));
    {
        const char* family = (beff->flags & FCS_F_FONT_FAMILY && beff->font_family) ? beff->font_family : "Arial";
        int weight = (beff->flags & FCS_F_FONT_WEIGHT) ? beff->font_weight : FW_NORMAL;
        int size = (beff->flags & FCS_F_FONT_SIZE) ? beff->font_size : 11;
        wchar_t wfamily[128];
        MultiByteToWideChar(CP_UTF8, 0, family, -1, wfamily, 128);
        HFONT font = CreateFontW(-MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, wfamily);
        HFONT old_font = (HFONT)SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        wchar_t wbuf[512];
        int wlen = MultiByteToWideChar(CP_UTF8, 0, content, -1, wbuf, 512);
        DrawTextW(hdc, wbuf, wlen - 1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, old_font);
        DeleteObject(font);
    }
    #else
    GC gc = XCreateGC(window->display, window->handle, 0, NULL);
    XDrawRectangle(window->display, window->handle, gc, self->x, self->y, (unsigned)self->width, (unsigned)self->height);
    XDrawString(window->display, window->handle, gc, self->x + 4, self->y + self->height / 2, content, (int)strlen(content));
    XFreeGC(window->display, gc);
    #endif
}

void FOUR_Element_initButton(FOUR_Element* element) { FOUR_Element_init_button(element); }

FOUR_DEFINE_ELEMENT(image, "image")
{
    if (is_hidden(self)) return;
    #ifdef _WIN32
    HDC hdc = window->render_dc;
    draw_styled_bg_border(self, hdc);
    const char* path = self->src ? self->src : (self->content ? self->content : NULL);
    if (path)
    {
        CachedImage* ci = img_ensure(&self->image_cache, path);
        int w = self->width > 0 ? self->width : (ci ? ci->w : 0);
        int h = self->height > 0 ? self->height : (ci ? ci->h : 0);

        if (self->rotation != 0.0f && ci)
        {
            float angle = self->rotation * 3.14159265f / 180.0f;
            float ca = cosf(angle), sa = sinf(angle);
            float cx = (float)(self->x + w / 2), cy = (float)(self->y + h / 2);
            XFORM xf = { ca, sa, -sa, ca, cx - cx * ca + cy * sa, cy - cx * sa - cy * ca };
            SetGraphicsMode(hdc, GM_ADVANCED);
            SetWorldTransform(hdc, &xf);
            draw_image(hdc, ci, self->x, self->y, w, h);
            XFORM id = { 1, 0, 0, 1, 0, 0 };
            SetWorldTransform(hdc, &id);
            SetGraphicsMode(hdc, GM_COMPATIBLE);
        }
        else
        {
            draw_image(hdc, ci, self->x, self->y, w, h);
        }
    }
    #else
    (void)window;
    #endif
}

void FOUR_Element_initImage(FOUR_Element* element) { FOUR_Element_init_image(element); }

FOUR_DEFINE_ELEMENT(list, "list")
{
    if (is_hidden(self)) return;
    #ifdef _WIN32
    draw_styled_bg_border(self, window->render_dc);
    #else
    GC gc = XCreateGC(window->display, window->handle, 0, NULL);
    XDrawRectangle(window->display, window->handle, gc, self->x, self->y, (unsigned)self->width, (unsigned)self->height);
    XFreeGC(window->display, gc);
    #endif

    int border_offset = 0;
    FCS_ComputedStyle effective = get_effective_style(self);
    if (effective.flags & FCS_F_BORDER)
    {
        border_offset = effective.border.width > 0 ? effective.border.width : 1;
    }

    int cy = self->y + border_offset;
    int child_x = self->x + border_offset;
    int child_w = self->width - 2 * border_offset;
    if (child_w < 0) child_w = 0;

    for (int i = 0; i < self->child_count; i++)
    {
        FOUR_Element* child = self->children[i];
        if (!child) continue;
        child->x = child_x;
        child->y = cy;
        child->width = child_w;
        cy += child->height;
    }
}

void FOUR_Element_initList(FOUR_Element* element) { FOUR_Element_init_list(element); }

FOUR_DEFINE_ELEMENT(heading1, "h1") { draw_text_scaled(self, window, 32, FW_BOLD); }
FOUR_DEFINE_ELEMENT(heading2, "h2") { draw_text_scaled(self, window, 24, FW_BOLD); }
FOUR_DEFINE_ELEMENT(heading3, "h3") { draw_text_scaled(self, window, 18, FW_BOLD); }
FOUR_DEFINE_ELEMENT(paragraph, "p") { draw_text_scaled(self, window, 12, FW_NORMAL); }

void FOUR_Element_initHeading1(FOUR_Element* element) { FOUR_Element_init_heading1(element); }
void FOUR_Element_initHeading2(FOUR_Element* element) { FOUR_Element_init_heading2(element); }
void FOUR_Element_initHeading3(FOUR_Element* element) { FOUR_Element_init_heading3(element); }
void FOUR_Element_initParagraph(FOUR_Element* element) { FOUR_Element_init_paragraph(element); }

FOUR_DEFINE_ELEMENT(frame, "frame")
{
    if (is_hidden(self)) return;
    #ifdef _WIN32
    draw_styled_bg_border(self, window->render_dc);
    if (self->frame_document) FOUR_renderDocumentAt(self->frame_document, window, self->x, self->y, self->width, self->height);
    #else
    (void)window;
    #endif
}

void FOUR_Element_initFrame(FOUR_Element* element) { FOUR_Element_init_frame(element); }

FOUR_DEFINE_ELEMENT(input, "input")
{
    if (is_hidden(self)) return;
    #ifdef _WIN32
    HDC hdc = window->render_dc;
    FCS_ComputedStyle _eff = get_effective_style(self);
    FCS_ComputedStyle* s = &_eff;

    if (s->flags & FCS_F_BG_COLOR) fill_bg(hdc, self->x, self->y, self->width, self->height, (FCS_BorderRadius){0,0,0,0}, s->bg_color);
    else
    {
        HBRUSH br = CreateSolidBrush(RGB(28, 28, 30));
        RECT r = { self->x, self->y, self->x + self->width, self->y + self->height };
        FillRect(hdc, &r, br);
        DeleteObject(br);
    }

    COLORREF border_col = self->focused ? RGB(80, 150, 255) : RGB(70, 70, 75);
    {
        HPEN pen = CreatePen(PS_SOLID, 1, border_col);
        HPEN oldp = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldb = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, self->x, self->y, self->x + self->width, self->y + self->height);
        SelectObject(hdc, oldp);
        SelectObject(hdc, oldb);
        DeleteObject(pen);
    }

    const char* text = self->content ? self->content : "";
    {
        const char* family = (s->flags & FCS_F_FONT_FAMILY && s->font_family) ? s->font_family : "Arial";
        int weight = (s->flags & FCS_F_FONT_WEIGHT) ? s->font_weight : FW_NORMAL;
        int size = (s->flags & FCS_F_FONT_SIZE) ? s->font_size : 11;
        HFONT font = CreateFontA(-MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, family);
        HFONT old_font = (HFONT)SelectObject(hdc, font);
        COLORREF text_col = (s->flags & FCS_F_COLOR) ? fcs_colorref(s->color) : RGB(220, 220, 220);
        SetTextColor(hdc, text_col);
        SetBkMode(hdc, TRANSPARENT);
        RECT tr = { self->x + 5, self->y, self->x + self->width - 5, self->y + self->height };
        DrawTextA(hdc, text, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        if (self->focused)
        {
            SIZE sz = {0, 0};
            GetTextExtentPoint32A(hdc, text, (int)strlen(text), &sz);
            int cx = self->x + 5 + sz.cx;
            if (cx < self->x + self->width - 3)
            {
                HPEN cp = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
                HPEN op = (HPEN)SelectObject(hdc, cp);
                MoveToEx(hdc, cx, self->y + 3, NULL);
                LineTo(hdc, cx, self->y + self->height - 3);
                SelectObject(hdc, op);
                DeleteObject(cp);
            }
        }
        SelectObject(hdc, old_font);
        DeleteObject(font);
    }
    #else
    (void)window;
    #endif
}

void FOUR_Element_initInput(FOUR_Element* element)
{
    FOUR_Element_init_input(element);
    element->input_value[0] = '\0';
    element->content = element->input_value;
}

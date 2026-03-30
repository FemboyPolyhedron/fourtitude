#include "fourtitudec17.h"
#include "FourtitudeStyle.h"
#include <stdlib.h>

static void flex_layout(FOUR_Element* elem)
{
    if (!elem || elem->child_count == 0) return;

    FCS_ComputedStyle* s = elem->computed_style;
    if (!s || !(s->flags & FCS_F_DISPLAY) || s->display != FCS_DISPLAY_FLEX) return;

    int pad_left   = (s->flags & FCS_F_PADDING) ? s->padding.left   : 0;
    int pad_top    = (s->flags & FCS_F_PADDING) ? s->padding.top    : 0;
    int pad_right  = (s->flags & FCS_F_PADDING) ? s->padding.right  : 0;
    int pad_bottom = (s->flags & FCS_F_PADDING) ? s->padding.bottom : 0;

    int inner_w = elem->width  - pad_left - pad_right;
    int inner_h = elem->height - pad_top  - pad_bottom;

    int is_row = s->flex_direction == FCS_FLEX_ROW || s->flex_direction == FCS_FLEX_ROW_REVERSE;
    int reverse = s->flex_direction == FCS_FLEX_ROW_REVERSE || s->flex_direction == FCS_FLEX_COLUMN_REVERSE;

    int count = elem->child_count;

    int total_fixed = 0;
    float total_grow = 0.0f;
    for (int i = 0; i < count; i++)
    {
        FOUR_Element* child = elem->children[i];
        if (!child) continue;
        FCS_ComputedStyle* cs = child->computed_style;
        float grow = (cs && (cs->flags & FCS_F_FLEX_GROW)) ? cs->flex_grow : 0.0f;
        if (grow > 0.0f) total_grow += grow;
        else total_fixed += is_row ? child->width : child->height;
    }

    int available = (is_row ? inner_w : inner_h) - total_fixed;
    int cursor = is_row ? (elem->x + pad_left) : (elem->y + pad_top);
    int gap = 0;
    int offset_pre = 0;

    if (total_grow == 0.0f && count > 0)
    {
        int content = total_fixed;
        int space   = (is_row ? inner_w : inner_h) - content;
        switch (s->justify_content)
        {
            case FCS_JUSTIFY_CENTER: offset_pre = space / 2; break;
            case FCS_JUSTIFY_END: offset_pre = space; break;
            case FCS_JUSTIFY_SPACE_BETWEEN: gap = count > 1 ? space / (count - 1) : 0; break;
            case FCS_JUSTIFY_SPACE_AROUND:  gap = space / count; offset_pre = gap / 2; break;
            case FCS_JUSTIFY_SPACE_EVENLY:  gap = space / (count + 1); offset_pre = gap; break;
            default: break;
        }
        cursor += offset_pre;
    }

    int start = reverse ? count - 1 : 0;
    int end   = reverse ? -1        : count;
    int step  = reverse ? -1        : 1;

    for (int i = start; i != end; i += step)
    {
        FOUR_Element* child = elem->children[i];
        if (!child) continue;
        FCS_ComputedStyle* cs = child->computed_style;

        float grow = (cs && (cs->flags & FCS_F_FLEX_GROW)) ? cs->flex_grow : 0.0f;
        int size = 0;
        if (grow > 0.0f && total_grow > 0.0f) size = (int)(available * grow / total_grow);
        else size = is_row ? child->width : child->height;

        if (is_row)
        {
            child->x = cursor;
            if (grow > 0.0f) child->width = size;
            int cross = (s->flags & FCS_F_ALIGN_ITEMS) ? s->align_items : FCS_ALIGN_START;
            switch (cross)
            {
                case FCS_ALIGN_CENTER: child->y = elem->y + pad_top + (inner_h - child->height) / 2; break;
                case FCS_ALIGN_END: child->y = elem->y + pad_top +  inner_h - child->height; break;
                case FCS_ALIGN_STRETCH: child->y = elem->y + pad_top; child->height = inner_h; break;
                default: child->y = elem->y + pad_top; break;
            }
            cursor += size + gap;
        }
        else
        {
            child->y = cursor;
            if (grow > 0.0f) child->height = size;
            int cross = (s->flags & FCS_F_ALIGN_ITEMS) ? s->align_items : FCS_ALIGN_START;
            switch (cross)
            {
                case FCS_ALIGN_CENTER: child->x = elem->x + pad_left + (inner_w - child->width) / 2; break;
                case FCS_ALIGN_END: child->x = elem->x + pad_left +  inner_w - child->width; break;
                case FCS_ALIGN_STRETCH: child->x = elem->x + pad_left; child->width = inner_w; break;
                default: child->x = elem->x + pad_left; break;
            }
            cursor += size + gap;
        }

        flex_layout(child);
    }
}

static int get_z_index(const FOUR_Element* elem)
{
    if (!elem || !elem->computed_style) return 0;
    if (elem->computed_style->flags & FCS_F_Z_INDEX) return elem->computed_style->z_index;
    return 0;
}

static void sort_elements_by_z_index(FOUR_Element** elements, int count)
{
    if (!elements || count <= 1) return;

    for (int i = 1; i < count; i++)
    {
        FOUR_Element* key = elements[i];
        int key_z = get_z_index(key);
        int j = i - 1;

        while (j >= 0 && get_z_index(elements[j]) > key_z)
        {
            elements[j + 1] = elements[j];
            j--;
        }
        elements[j + 1] = key;
    }
}

static void render_element(FOUR_Element* elem, FOUR_Window* window)
{
    if (!elem->enabled) return;
    FCS_ComputedStyle* cs = elem->computed_style;
    if (cs && (cs->flags & FCS_F_DISPLAY) && cs->display == FCS_DISPLAY_NONE) return;

    if (elem->handleRender) elem->handleRender(elem, window);
 
    if (elem->child_count <= 0) return;

    FOUR_Element** sorted = malloc(sizeof(FOUR_Element*) * elem->child_count);
    if (!sorted) return;

    for (int i = 0; i < elem->child_count; i++) sorted[i] = elem->children[i];

    sort_elements_by_z_index(sorted, elem->child_count);

    for (int i = 0; i < elem->child_count; i++) render_element(sorted[i], window);

    free(sorted);
}

void FOUR_renderDocuments(FOUR_Document document[], FOUR_Window window)
{
    if (!document) return;

    #ifdef _WIN32
    if (window.render_dc)
    {
        RECT rect;
        GetClientRect(window.handle, &rect);
        FillRect(window.render_dc, &rect, GetStockObject(WHITE_BRUSH));
    }
    #endif

    for (int d = 0; document[d].elements != NULL; d++)
    {
        FOUR_Document* doc = &document[d];
        for (int i = 0; i < doc->element_count; i++)
        {
            FOUR_Element* elem = doc->elements[i];
            if (elem && !elem->parent) flex_layout(elem);
        }

        if (doc->element_count > 0)
        {
            FOUR_Element** sorted_top = malloc(sizeof(FOUR_Element*) * doc->element_count);
            if (sorted_top)
            {
                int root_count = 0;
                for (int i = 0; i < doc->element_count; i++)
                {
                    FOUR_Element* elem = doc->elements[i];
                    if (elem && !elem->parent) sorted_top[root_count++] = elem;
                }

                sort_elements_by_z_index(sorted_top, root_count);

                for (int i = 0; i < root_count; i++) render_element(sorted_top[i], &window);

                free(sorted_top);
            }
        }
        else
        {
            for (int i = 0; i < doc->element_count; i++)
            {
                FOUR_Element* elem = doc->elements[i];
                if (elem && !elem->parent) render_element(elem, &window);
            }
        }
    }

    #ifdef _WIN32
    InvalidateRect(window.handle, NULL, FALSE);
    UpdateWindow(window.handle);
    #endif
}

void FOUR_renderDocumentAt(FOUR_Document* doc, FOUR_Window* window, int x, int y, int w, int h)
{
    if (!doc || !window) return;
    #ifdef _WIN32
    HDC hdc = window->render_dc;
    if (!hdc) return;
    HRGN clip = CreateRectRgn(x, y, x + w, y + h);
    SelectClipRgn(hdc, clip);
    DeleteObject(clip);
    SetViewportOrgEx(hdc, x, y, NULL);
    #endif

    for (int i = 0; i < doc->element_count; i++)
    {
        FOUR_Element* elem = doc->elements[i];
        if (elem && !elem->parent) { flex_layout(elem); render_element(elem, window); }
    }

    #ifdef _WIN32
    SetViewportOrgEx(hdc, 0, 0, NULL);
    SelectClipRgn(hdc, NULL);
    #endif
}

void FOUR_renderDocuments_noflush(FOUR_Document document[], FOUR_Window window)
{
    if (!document) return;

    #ifdef _WIN32
    if (window.render_dc)
    {
        RECT rect;
        GetClientRect(window.handle, &rect);
        FillRect(window.render_dc, &rect, GetStockObject(WHITE_BRUSH));
    }
    #endif

    for (int d = 0; document[d].elements != NULL; d++)
    {
        FOUR_Document* doc = &document[d];
        for (int i = 0; i < doc->element_count; i++)
        {
            FOUR_Element* elem = doc->elements[i];
            if (elem && !elem->parent) flex_layout(elem);
        }

        if (doc->element_count > 0)
        {
            FOUR_Element** sorted_top = malloc(sizeof(FOUR_Element*) * doc->element_count);
            if (sorted_top)
            {
                int root_count = 0;
                for (int i = 0; i < doc->element_count; i++)
                {
                    FOUR_Element* elem = doc->elements[i];
                    if (elem && !elem->parent) sorted_top[root_count++] = elem;
                }
                sort_elements_by_z_index(sorted_top, root_count);
                for (int i = 0; i < root_count; i++) render_element(sorted_top[i], &window);
                free(sorted_top);
            }
        }
        else
        {
            for (int i = 0; i < doc->element_count; i++)
            {
                FOUR_Element* elem = doc->elements[i];
                if (elem && !elem->parent) render_element(elem, &window);
            }
        }
    }
}

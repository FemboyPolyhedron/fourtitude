#include <vector>
#include <stdio.h>
#include "fourtitudecpp.h"

namespace Fourtitude
{
    void FourtitudeForm::attach(FourtitudeDocument* doc)
    {
        this->document.push_back(doc);
    }

    void FourtitudeForm::render(FourtitudeWindow* window)
    {
        _cdocs.clear();
        _cdocs.reserve(this->document.size() + 1);
        for (auto* doc : this->document) _cdocs.push_back(*doc->raw());
        _cdocs.push_back({ nullptr, 0 });

        _cform.documents    = _cdocs.data();
        _cform.window       = window->raw();
        window->raw()->form = &_cform;

        FOUR_renderDocuments_noflush(_cdocs.data(), *window->raw());

        if (_post_render) _post_render(window->raw());

        #ifdef _WIN32
        InvalidateRect(window->raw()->handle, NULL, FALSE);
        UpdateWindow(window->raw()->handle);
        #endif
    }

    bool FourtitudeForm::dispatchEvents(FourtitudeWindow* window)
    {
        FOUR_Window* w = window->raw();
        int mx = w->mouse_x;
        int my = w->mouse_y;
        bool dirty = false;
        bool clicked_input = false;

        // Handle char input for focused input element
        if (_focused && w->last_char)
        {
            char c = (char)w->last_char;
            int len = (int)strlen(_focused->input_value);
            if (c == 8) { if (len > 0) { _focused->input_value[--len] = '\0'; dirty = true; } }
            else if (c >= 32 && len < 255) { _focused->input_value[len] = c; _focused->input_value[len + 1] = '\0'; dirty = true; }
            _focused->content = _focused->input_value;
        }

        for (auto* doc : this->document)
        {
            for (auto* elem : doc->elems())
            {
                if (!elem) continue;
                FOUR_Element* raw = elem->raw();

                // dispatch into frame sub-document with translated coordinates
                if (raw->frame_document && raw->frame_document->elements)
                {
                    int fmx = mx - raw->x, fmy = my - raw->y;
                    for (int fi = 0; fi < raw->frame_document->element_count; fi++)
                    {
                        FOUR_Element* fe = raw->frame_document->elements[fi];
                        if (!fe || !fe->enabled) continue;
                        if (fe->computed_style && (fe->computed_style->flags & FCS_F_DISPLAY) && fe->computed_style->display == FCS_DISPLAY_NONE) continue;
                        bool fhit = fmx >= fe->x && fmx < fe->x + fe->width && fmy >= fe->y && fmy < fe->y + fe->height;
                        bool was_fhovered = fe->hovered != 0;
                        fe->hovered = fhit ? 1 : 0;
                        if (fhit != was_fhovered && (fe->hover_style || fe->active_style)) dirty = true;
                        if (fhit && w->mouse_clicked)
                        {
                            FOUR_EventData ed = {};
                            ed.event = UIEVENTS_onClick; ed.x = fmx; ed.y = fmy;
                            FOUR_Element_triggerEvent(fe, UIEVENTS_onClick, ed);
                            dirty = true;
                        }
                        if (fhit && w->mouse_down)
                        {
                            bool is_factive = true;
                            if (fe->active != (is_factive ? 1 : 0)) { fe->active = is_factive ? 1 : 0; if (fe->active_style) dirty = true; }
                        }
                        else if (fe->active)
                        {
                            fe->active = 0;
                            if (fe->active_style) dirty = true;
                        }
                    }
                }
                if (!raw->enabled) continue;
                if (raw->computed_style && (raw->computed_style->flags & FCS_F_DISPLAY) && raw->computed_style->display == FCS_DISPLAY_NONE) continue;

                bool hit = mx >= raw->x && mx < raw->x + raw->width && my >= raw->y && my < raw->y + raw->height;

                bool was_hovered = elem->hovered();
                bool was_active  = elem->mouseWasDown();
                elem->setHovered(hit);

                if (hit && !was_hovered)
                {
                    Event e; e.event = UIEVENTS_onMouseEnter; e.x = mx; e.y = my;
                    elem->trigger(e);
                    if (raw->hover_style) dirty = true;
                }
                if (!hit && was_hovered)
                {
                    Event e; e.event = UIEVENTS_onMouseLeave; e.x = mx; e.y = my;
                    elem->trigger(e);
                    if (raw->hover_style) dirty = true;
                }
                if (hit)
                {
                    Event e; e.event = UIEVENTS_onMouseMove; e.x = mx; e.y = my;
                    elem->trigger(e);
                }
                if (hit && w->mouse_down && !was_active)
                {
                    Event e; e.event = UIEVENTS_onMouseDown; e.x = mx; e.y = my; e.button = 0;
                    elem->trigger(e);
                }
                if (hit && w->mouse_clicked)
                {
                    if (raw->type && strcmp(raw->type, "input") == 0)
                    {
                        if (_focused && _focused != raw) { _focused->focused = 0; }
                        _focused = raw;
                        raw->focused = 1;
                        clicked_input = true;
                        dirty = true;
                    }
                    Event e; e.event = UIEVENTS_onMouseUp; e.x = mx; e.y = my; e.button = 0;
                    elem->trigger(e);
                    Event ec; ec.event = UIEVENTS_onClick; ec.x = mx; ec.y = my; ec.button = 0;
                    elem->trigger(ec);
                }

                bool is_active = hit && w->mouse_down != 0;
                if (raw->hovered != (hit ? 1 : 0))  { raw->hovered = hit ? 1 : 0; dirty = true; }
                if (raw->active  != (is_active ? 1 : 0)) { raw->active = is_active ? 1 : 0; if (raw->active_style) dirty = true; }
                elem->setMouseWasDown(hit && w->mouse_down);

                if (w->last_key)
                {
                    Event e; e.key = w->last_key;
                    e.event = w->key_down ? UIEVENTS_onKeyDown : UIEVENTS_onKeyUp;
                    elem->trigger(e);
                }
            }
        }
        if (w->mouse_clicked && !clicked_input && _focused)
        {
            _focused->focused = 0;
            _focused = nullptr;
            dirty = true;
        }

        return dirty;
    }
}

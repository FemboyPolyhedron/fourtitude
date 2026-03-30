#include "fourtitudec17.h"
#include <stdlib.h>
#include <string.h>

static FOUR_EventListeners* find_listeners(FOUR_Element* element, Events ev)
{
    for (int i = 0; i < element->listener_count; i++) if (element->listeners[i].event == ev) return &element->listeners[i];
    return NULL;
}

void FOUR_Element_addEvent(FOUR_Document document, FOUR_Element* element, Events ev, FOUR_EventFn function)
{
    (void)document;

    FOUR_EventListeners* slot = find_listeners(element, ev);
    if (!slot)
    {
        element->listeners = realloc(element->listeners, sizeof(FOUR_EventListeners) * (element->listener_count + 1));
        slot = &element->listeners[element->listener_count++];
        slot->event = ev;
        slot->fns   = NULL;
        slot->count = 0;
    }

    slot->fns = realloc(slot->fns, sizeof(FOUR_EventFn) * (slot->count + 1));
    slot->fns[slot->count++] = function;
}

void FOUR_Element_triggerEvent(FOUR_Element* element, Events ev, FOUR_EventData data)
{
    FOUR_EventListeners* slot = find_listeners(element, ev);
    if (!slot) return;
    for (int i = 0; i < slot->count; i++) if (slot->fns[i]) slot->fns[i](data);
}

void FOUR_triggerEvent(FOUR_Document document, Events ev, void* value)
{
    (void)value;
    FOUR_EventData data = {0};
    data.event = ev;
    for (int i = 0; i < document.element_count; i++) if (document.elements[i]) FOUR_Element_triggerEvent(document.elements[i], ev, data);
}

static void dispatch_element(FOUR_Element* elem, FOUR_Window* w, int* prev_hover, int elem_idx)
{
    if (!elem->enabled) return;

    int hit = w->mouse_x >= elem->x && w->mouse_x < elem->x + elem->width && w->mouse_y >= elem->y && w->mouse_y < elem->y + elem->height;

    int was_hovered = (prev_hover[elem_idx / 32] >> (elem_idx % 32)) & 1;

    FOUR_EventData d = {0};
    d.x = w->mouse_x;
    d.y = w->mouse_y;

    if (hit && !was_hovered) { d.event = UIEVENTS_onMouseEnter; FOUR_Element_triggerEvent(elem, UIEVENTS_onMouseEnter, d); }
    if (!hit && was_hovered) { d.event = UIEVENTS_onMouseLeave; FOUR_Element_triggerEvent(elem, UIEVENTS_onMouseLeave, d); }
    if (hit) { d.event = UIEVENTS_onMouseMove;  FOUR_Element_triggerEvent(elem, UIEVENTS_onMouseMove,  d); }

    if (hit && w->mouse_down) { d.event = UIEVENTS_onMouseDown; FOUR_Element_triggerEvent(elem, UIEVENTS_onMouseDown, d); }
    if (hit && w->mouse_clicked)
    {
        d.event = UIEVENTS_onMouseUp; FOUR_Element_triggerEvent(elem, UIEVENTS_onMouseUp, d);
        d.event = UIEVENTS_onClick; FOUR_Element_triggerEvent(elem, UIEVENTS_onClick,   d);
    }

    if (w->last_key)
    {
        d.key = w->last_key;
        d.event = w->key_down ? UIEVENTS_onKeyDown : UIEVENTS_onKeyUp;
        FOUR_Element_triggerEvent(elem, d.event, d);
    }

    if (hit) prev_hover[elem_idx / 32] |=  (1 << (elem_idx % 32));
    else prev_hover[elem_idx / 32] &= ~(1 << (elem_idx % 32));
}

void FOUR_dispatchEvents(FOUR_Document document[], FOUR_Window* window)
{
    if (!document || !window) return;

    static int prev_hover[64] = {0};
    int elem_idx = 0;

    for (int d = 0; document[d].elements != NULL; d++)
    {
        FOUR_Document* doc = &document[d];
        for (int i = 0; i < doc->element_count && elem_idx < 64 * 32; i++, elem_idx++)
        {
            FOUR_Element* elem = doc->elements[i];
            if (elem) dispatch_element(elem, window, prev_hover, elem_idx);
        }
    }
}

#include "fourtitudec17.h"

void FOUR_Form_render(FOUR_Form* self, FOUR_Window* window)
{
    self->window  = window;
    window->form  = self;
    FOUR_renderDocuments(self->documents, *window);
}

void FOUR_Form_dispatchEvents(FOUR_Form* self)
{
    if (!self->window) return;
    FOUR_dispatchEvents(self->documents, self->window);
}

#include "fourtitudec17.h"
#include <stdlib.h>

void FOUR_Element_appendChild(FOUR_Element* parent, FOUR_Element* child)
{
    if (!parent || !child) return;

    child->parent = parent;
    parent->children = realloc(parent->children, sizeof(FOUR_Element*) * (parent->child_count + 1));
    parent->children[parent->child_count++] = child;
}

void FOUR_Element_removeChild(FOUR_Element* parent, FOUR_Element* child)
{
    if (!parent || !child) return;

    for (int i = 0; i < parent->child_count; i++)
    {
        if (parent->children[i] != child) continue;

        child->parent = NULL;
        parent->children[i] = parent->children[--parent->child_count];
        parent->children = realloc(parent->children, sizeof(FOUR_Element*) * parent->child_count);
        return;
    }
}

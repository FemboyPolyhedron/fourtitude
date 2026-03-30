#include "fourtitudecpp.h"
#include <cstring>

namespace Fourtitude
{
    void FourtitudeDocument::append(FourtitudeElement* elem)
    {
        if (!elem) return;

        _elems.push_back(elem);

        document.elements = static_cast<FOUR_Element**>( realloc(document.elements, sizeof(FOUR_Element*) * (document.element_count + 1)));
        document.elements[document.element_count++] = elem->raw();
    }

    FourtitudeElement* FourtitudeDocument::getElementById(std::string id)
    {
        for (auto* e : _elems)
        {
            const char* eid = e->raw()->id;
            if (eid && id == eid) return e;
        }
        return nullptr;
    }

    std::vector<FourtitudeElement*> FourtitudeDocument::getElementsByClass(std::string clazz)
    {
        std::vector<FourtitudeElement*> result;
        for (auto* e : _elems)
        {
            const char* ec = e->raw()->clazz;
            if (ec && clazz == ec) result.push_back(e);
        }
        return result;
    }

    std::vector<FourtitudeElement*> FourtitudeDocument::getElementsByType(std::string type)
    {
        std::vector<FourtitudeElement*> result;
        for (auto* e : _elems)
        {
            const char* et = e->raw()->type;
            if (et && type == et) result.push_back(e);
        }
        return result;
    }
}

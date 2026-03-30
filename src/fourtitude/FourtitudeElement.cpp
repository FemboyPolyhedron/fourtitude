#include "fourtitudecpp.h"

namespace Fourtitude
{
    using Listener = void(*)(Event);

    void FourtitudeElement::addEventListener(Events ev, Listener function)
    {
        listeners[ev].push_back(function);
    }

    void FourtitudeElement::remove()
    {
        if (parent) parent->removeChild(this);
    }

    void FourtitudeElement::trigger(Event ev)
    {
        auto it = listeners.find(ev.event);
        if (it != listeners.end())
        {
            for (auto fn : it->second)
            {
                if (fn) fn(ev);
            }
        }

        /// @deprecated
        switch (ev.event)
        {
            case UIEVENTS_onEnable:  this->onEnable();  break;
            case UIEVENTS_onDisable: this->onDisable(); break;
        }
    }

    void FourtitudeElement::appendChild(FourtitudeElement* child)
    {
        if (!child || child == this) return;

        if (child->parent)
            child->parent->removeChild(child);

        child->parent = this;
        children.push_back(child);
        FOUR_Element_appendChild(raw(), child->raw());
    }

    void FourtitudeElement::removeChild(FourtitudeElement* child)
    {
        if (!child) return;

        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if (*it != child) continue;
            child->parent = nullptr;
            children.erase(it);
            FOUR_Element_removeChild(raw(), child->raw());
            return;
        }
    }

    FourtitudeElement* FourtitudeElement::getParent() const
    {
        return parent;
    }

    const std::vector<FourtitudeElement*>& FourtitudeElement::getChildren() const
    {
        return children;
    }
}

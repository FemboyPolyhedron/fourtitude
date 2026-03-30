#pragma once

#ifndef __cplusplus
#error must use cpp
#endif

#include "fourtitudec17.h"
#include <functional>

// FOURTITUDE for C++17
//
// Authors:
//  - fempolyhedron :3
//    * github  : https://github.com/FemboyPolyhedron
//    * website : https://fempolyhedron.dev
//    * bsky    : poly.fempolyhedron.dev
//                (poly.bskypds.fempolyhedron.dev)
//    * discord : @elephant_lover
//    * twitter : @FemPolyhedron
//
// (c) fempolyhedron 2026
//
// four :3

#include <vector>
#include <unordered_map>
#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif

/// @brief Fourtitude
/// @author fempolyhedron
/// @version 0.0.1
namespace Fourtitude
{
    /// @file ./FourtitudeWindow.cpp
    class FourtitudeWindow
    {
        public: FourtitudeWindow();
        public: ~FourtitudeWindow();

        public: void create(int width, int height, const char* title, int flags = FOUR_WINDOW_DEFAULT);
        public: void destroy();
        public: void setPos(int x, int y) { FOUR_Window_setPos(&win, x, y); }
        public: void center()             { FOUR_Window_center(&win); }

        public: void pollEvents();
        public: bool shouldClose() const;

        public: FOUR_Window*       raw()       { return &win; }
        public: const FOUR_Window* raw() const { return &win; }

        private: FOUR_Window win = {};
        private: bool running = false;
    };

    struct Event
    {
        Events      event;
        int         x = 0, y = 0;
        int         key = 0;
        int         button = 0;
    };

    static inline Events event_from_string(const char* name)
    {
        if (!name) return UIEVENTS_onEnable;
        if (strcmp(name, "onClick")      == 0) return UIEVENTS_onClick;
        if (strcmp(name, "onMouseEnter") == 0) return UIEVENTS_onMouseEnter;
        if (strcmp(name, "onMouseLeave") == 0) return UIEVENTS_onMouseLeave;
        if (strcmp(name, "onMouseMove")  == 0) return UIEVENTS_onMouseMove;
        if (strcmp(name, "onMouseDown")  == 0) return UIEVENTS_onMouseDown;
        if (strcmp(name, "onMouseUp")    == 0) return UIEVENTS_onMouseUp;
        if (strcmp(name, "onKeyDown")    == 0) return UIEVENTS_onKeyDown;
        if (strcmp(name, "onKeyUp")      == 0) return UIEVENTS_onKeyUp;
        if (strcmp(name, "onFocus")      == 0) return UIEVENTS_onFocus;
        if (strcmp(name, "onBlur")       == 0) return UIEVENTS_onBlur;
        if (strcmp(name, "onChange")     == 0) return UIEVENTS_onChange;
        if (strcmp(name, "onEnable")     == 0) return UIEVENTS_onEnable;
        if (strcmp(name, "onDisable")    == 0) return UIEVENTS_onDisable;
        return UIEVENTS_onEnable;
    }

    /// @brief a fourtitude UI element
    /// @author fempolyhedron
    /// @file ./FourtitudeElement.cpp
    /// @version 0.0.1
    class FourtitudeElement
    {
        private: FOUR_Element elem = {};

        using Listener = void(*)(Event);
        private: std::unordered_map<Events, std::vector<Listener>> listeners;

        private: FourtitudeElement* parent = nullptr;
        private: std::vector<FourtitudeElement*> children;
        private: bool _hovered      = false;
        private: bool _mouseWasDown = false;

        public: FourtitudeElement() = default;
        public: virtual ~FourtitudeElement() = default;

        public: FOUR_Element*       raw()       { return &elem; }
        public: const FOUR_Element* raw() const { return &elem; }

        public: void setPosition(int x, int y)              { elem.x = x; elem.y = y; }
        public: void setSize(int width, int height)         { elem.width = width; elem.height = height; }
        public: void setBounds(int x, int y, int w, int h)  { elem.x = x; elem.y = y; elem.width = w; elem.height = h; }
        public: void setEnabled(bool enabled)               { elem.enabled = enabled ? 1 : 0; }
        public: bool isEnabled() const                      { return elem.enabled != 0; }
        public: void setRenderFn(FOUR_RenderFn fn)          { elem.handleRender = fn; }

        /// @brief Add a listener
        public: void addEventListener(Events ev, void (*function)(Event));
        /// @brief Add a listener by event name
        public: void addEventListener(const char* ev, void (*function)(Event)) { addEventListener(event_from_string(ev), function); }
        /// @brief Remove element
        public: void remove();
        /// @brief Trigger an event
        public: void trigger(Event ev);

        public: void appendChild(FourtitudeElement* child);
        public: void removeChild(FourtitudeElement* child);
        public: FourtitudeElement* getParent() const;
        public: const std::vector<FourtitudeElement*>& getChildren() const;

        public: void handleRender() { if (elem.handleRender) elem.handleRender(&elem, nullptr); }

        public: bool hovered()      const { return _hovered; }
        public: bool mouseWasDown() const { return _mouseWasDown; }
        public: void setHovered(bool v)      { _hovered = v; }
        public: void setMouseWasDown(bool v) { _mouseWasDown = v; }

        #pragma region events

        /// @brief Runs every UI tick
        public: virtual void update() { }

        /// @brief Runs on enable
        /// @deprecated use event "onEnable"
        public: virtual void onEnable() { }

        /// @brief Runs on disable
        /// @deprecated use event "onDisable"
        public: virtual void onDisable() { }

        /// @brief Runs on element initialize
        public: virtual void onInit() { }

        /// @brief Runs on deinitialize/delete
        public: virtual void onDeinit() { }

        #pragma endregion events
    };

    /// @file ./FourtitudeDocument.cpp
    class FourtitudeDocument
    {
        private: FOUR_Document document = {};
        private: std::vector<FourtitudeElement*> _elems;

        public: FOUR_Document*       raw()       { return &document; }
        public: const FOUR_Document* raw() const { return &document; }

        public: void append(FourtitudeElement* elem);
        public: FourtitudeElement* getElementById(std::string id);
        public: std::vector<FourtitudeElement*> getElementsByClass(std::string clazz);
        public: std::vector<FourtitudeElement*> getElementsByType(std::string type);
        public: const std::vector<FourtitudeElement*>& elems() const { return _elems; }
    };

    /// @file ./FourtitudeForm.cpp
    class FourtitudeForm
    {
        private: std::vector<FourtitudeDocument*>        document;
        private: std::vector<FOUR_Document>              _cdocs;
        private: FOUR_Form                               _cform = {};
        private: std::function<void(FOUR_Window*)>       _post_render;
        private: FOUR_Element*                           _focused = nullptr;

        public: void attach(FourtitudeDocument* doc);
        public: void render(FourtitudeWindow* window);
        public: bool dispatchEvents(FourtitudeWindow* window);
        public: void setPostRender(std::function<void(FOUR_Window*)> fn) { _post_render = fn; }

        public: FOUR_Form*       raw()       { return &_cform; }
        public: const FOUR_Form* raw() const { return &_cform; }
    };

    /// @brief Runtime FCS stylesheet loader and applier
    class FourtitudeStylesheet
    {
        private: FCS_Stylesheet* sheet = nullptr;

        public: FourtitudeStylesheet() = default;
        public: ~FourtitudeStylesheet() { FCS_freeStylesheet(sheet); }

        public: bool loadFile(const char* path)  { sheet = FCS_loadFile(path);   return sheet != nullptr; }
        public: bool loadString(const char* src) { sheet = FCS_parseString(src); return sheet != nullptr; }

        public: FCS_Stylesheet*       raw()       { return sheet; }
        public: const FCS_Stylesheet* raw() const { return sheet; }

        public: void        apply(FourtitudeElement* elem) { FCS_applyToElement(sheet, elem->raw()); }
        public: void        apply(FourtitudeDocument* doc) { FCS_applyToDocument(sheet, doc->raw()); }
        public: const char* resolveVar(const char* name)   { return FCS_resolveVar(sheet, name); }
    };
}

/// @file ./FourtitudeElements.cpp
namespace Fourtitude::Elements
{
    /// @brief a layout container
    class Container : public FourtitudeElement
    {
        public: Container() { FOUR_Element_initContainer(raw()); }
    };

    /// @brief a frame — renders an embedded FourtitudeDocument at the frame's position
    class Frame : public FourtitudeElement
    {
        public: Frame() { FOUR_Element_initFrame(raw()); }
        public: void setDocument(FourtitudeDocument* doc)
        {
            raw()->frame_document = doc ? doc->raw() : nullptr;
        }
    };

    /// @brief a text label
    class Text : public FourtitudeElement
    {
        public: Text() { FOUR_Element_initText(raw()); }

        public: void        setText(const char* text) { raw()->content = text; }
        public: const char* getText() const           { return raw()->content; }
    };

    /// @brief a clickable button
    class Button : public FourtitudeElement
    {
        public: Button() { FOUR_Element_initButton(raw()); }

        public: void        setLabel(const char* label) { raw()->content = label; }
        public: const char* getLabel() const            { return raw()->content; }
        public: void        onClick(void (*fn)(Event))  { addEventListener(UIEVENTS_onEnable, fn); }
    };

    /// @brief a large heading (h1)
    class Heading1 : public FourtitudeElement
    {
        public: Heading1() { FOUR_Element_initHeading1(raw()); }

        public: void        setText(const char* text) { raw()->content = text; }
        public: const char* getText() const           { return raw()->content; }
    };

    /// @brief a medium heading (h2)
    class Heading2 : public FourtitudeElement
    {
        public: Heading2() { FOUR_Element_initHeading2(raw()); }

        public: void        setText(const char* text) { raw()->content = text; }
        public: const char* getText() const           { return raw()->content; }
    };

    /// @brief a small heading (h3)
    class Heading3 : public FourtitudeElement
    {
        public: Heading3() { FOUR_Element_initHeading3(raw()); }

        public: void        setText(const char* text) { raw()->content = text; }
        public: const char* getText() const           { return raw()->content; }
    };

    /// @brief a paragraph of body text
    class Paragraph : public FourtitudeElement
    {
        public: Paragraph() { FOUR_Element_initParagraph(raw()); }

        public: void        setText(const char* text) { raw()->content = text; }
        public: const char* getText() const           { return raw()->content; }
    };

    /// @brief an image element
    class Image : public FourtitudeElement
    {
        public: Image() { FOUR_Element_initImage(raw()); }

        public: void        setSrc(const char* path) { raw()->src = path; }
        public: const char* getSrc() const           { return raw()->src; }
    };

    /// @brief a vertical list container that stacks children
    class List : public FourtitudeElement
    {
        public: List() { FOUR_Element_initList(raw()); }
    };

    /// @brief a text input element
    class Input : public FourtitudeElement
    {
        public: Input() { FOUR_Element_initInput(raw()); }

        public: void        setValue(const char* v) { if (v) { strncpy(raw()->input_value, v, 255); raw()->input_value[255] = '\0'; raw()->content = raw()->input_value; } }
        public: const char* getValue() const        { return raw()->input_value; }
    };
}

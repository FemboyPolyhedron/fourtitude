#ifdef __cplusplus
extern "C" {
#endif
#ifndef FOURTITUDE_C17_H
#define FOURTITUDE_C17_H

// FOURTITUDE for C17
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

#ifdef _WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif

#include "FourtitudeLog.h"

typedef enum
{
    UIEVENTS_onEnable,
    UIEVENTS_onDisable,
    UIEVENTS_onClick,
    UIEVENTS_onMouseEnter,
    UIEVENTS_onMouseLeave,
    UIEVENTS_onMouseMove,
    UIEVENTS_onMouseDown,
    UIEVENTS_onMouseUp,
    UIEVENTS_onKeyDown,
    UIEVENTS_onKeyUp,
    UIEVENTS_onFocus,
    UIEVENTS_onBlur,
    UIEVENTS_onChange
} Events;

#define FOUR_WINDOW_DEFAULT    0
#define FOUR_WINDOW_BORDERLESS (1 << 0)
#define FOUR_WINDOW_FULLSCREEN (1 << 1)
#define FOUR_WINDOW_TOPMOST    (1 << 2)
#define FOUR_WINDOW_HIDDEN     (1 << 3)
#define FOUR_WINDOW_TOPLESS    (1 << 4)

struct FOUR_Form;

typedef struct
{
    #ifdef _WIN32
    HWND handle;
    HINSTANCE instance;
    HDC render_dc;
    HBITMAP render_bmp;
    #else
    Display* display;
    Window handle;
    int screen;
    #endif

    int width;
    int height;
    const char* title;
    int flags;
    int should_close;

    int mouse_x;
    int mouse_y;
    int mouse_down;
    int mouse_clicked;
    int last_key;
    int key_down;
    int last_char;

    char dropped_file[10240];

    struct FOUR_Form* form;
} FOUR_Window;

void FOUR_Window_create(FOUR_Window* window, int width, int height, const char* title, int flags);
void FOUR_Window_setPos(FOUR_Window* window, int x, int y);
void FOUR_Window_center(FOUR_Window* window);
void FOUR_Window_destroy(FOUR_Window* window);
void FOUR_Window_pollEvents(FOUR_Window* window);
int  FOUR_Window_shouldClose(FOUR_Window* window);

typedef struct
{
    Events event;
    int x, y;
    int key;
    int button;
} FOUR_EventData;

typedef void (*FOUR_EventFn)(FOUR_EventData);

typedef struct
{
    Events event;
    FOUR_EventFn* fns;
    int count;
} FOUR_EventListeners;

typedef struct FCS_ComputedStyle FCS_ComputedStyle;

typedef struct FOUR_Element FOUR_Element;
typedef void (*FOUR_RenderFn)(FOUR_Element* self, FOUR_Window* window);

struct FOUR_Element
{
    const char* id;
    const char* clazz;
    const char* type;
    const char* content;
    const char* src;

    int x, y, width, height;
    float rotation;
    int enabled;

    FOUR_EventListeners* listeners;
    int listener_count;

    FOUR_Element*  parent;
    FOUR_Element** children;
    int child_count;

    FOUR_RenderFn handleRender;
    FCS_ComputedStyle* computed_style;
    FCS_ComputedStyle* hover_style;
    FCS_ComputedStyle* active_style;
    int hovered;
    int active;
    int focused;
    char input_value[256];
    void* image_cache;

    struct FOUR_Document* frame_document;
};

typedef struct FOUR_Document
{
    FOUR_Element** elements;
    int element_count;
} FOUR_Document;

#include "FourtitudeStyle.h"

void FOUR_Element_addEvent(FOUR_Document document, FOUR_Element* element, Events ev, FOUR_EventFn function);
void FOUR_Element_triggerEvent(FOUR_Element* element, Events ev, FOUR_EventData data);
void FOUR_dispatchEvents(FOUR_Document document[], FOUR_Window* window);

void FOUR_Element_appendChild(FOUR_Element* parent, FOUR_Element* child);
void FOUR_Element_removeChild(FOUR_Element* parent, FOUR_Element* child);

void FOUR_Element_initContainer(FOUR_Element* element);
void FOUR_Element_initText(FOUR_Element* element);
void FOUR_Element_initButton(FOUR_Element* element);
void FOUR_Element_initHeading1(FOUR_Element* element);
void FOUR_Element_initHeading2(FOUR_Element* element);
void FOUR_Element_initHeading3(FOUR_Element* element);
void FOUR_Element_initParagraph(FOUR_Element* element);
void FOUR_Element_initImage(FOUR_Element* element);
void FOUR_Element_initList(FOUR_Element* element);
void FOUR_Element_initAppDragRegion(FOUR_Element* element);

void FOUR_FreeImageCache(void** cache_slot);

void FOUR_renderDocuments(FOUR_Document document[], FOUR_Window window);
void FOUR_renderDocuments_noflush(FOUR_Document document[], FOUR_Window window);
void FOUR_renderDocumentAt(FOUR_Document* doc, FOUR_Window* window, int x, int y, int w, int h);

void FOUR_Element_initFrame(FOUR_Element* element);
void FOUR_Element_initInput(FOUR_Element* element);
void FOUR_triggerEvent(FOUR_Document document, Events ev, void* value);

typedef struct FOUR_Form
{
    FOUR_Document* documents;
    FOUR_Window*   window;
} FOUR_Form;

void FOUR_Form_render(FOUR_Form* self, FOUR_Window* window);
void FOUR_Form_dispatchEvents(FOUR_Form* self);

#define FOUR_DEFINE_ELEMENT(name, type_str)                                 \
    static void name##_render(FOUR_Element* self, FOUR_Window* window);     \
    static inline void FOUR_Element_init_##name(FOUR_Element* el) {         \
        el->type = (type_str);                                              \
        el->enabled = 1;                                                    \
        el->handleRender = name##_render;                                   \
    }                                                                       \
    static void name##_render(FOUR_Element* self, FOUR_Window* window)

#endif
#ifdef __cplusplus
}
#endif

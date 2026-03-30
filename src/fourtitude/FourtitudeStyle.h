#ifdef __cplusplus
extern "C" {
#endif
#ifndef FOURTITUDE_STYLE_H
#define FOURTITUDE_STYLE_H

// FOURTITUDE - FCS (Fourtitude Cascading Styles)
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

#include <stdint.h>

typedef struct { uint8_t r, g, b, a; } FCS_Color;

#define FCS_RGB(r,g,b)        ((FCS_Color){(r),(g),(b),255})
#define FCS_RGBA(r,g,b,a)     ((FCS_Color){(r),(g),(b),(a)})
#define FCS_COLOR_UNSET       ((FCS_Color){0,0,0,0})
#define FCS_COLOR_BLACK       ((FCS_Color){0,0,0,255})
#define FCS_COLOR_WHITE       ((FCS_Color){255,255,255,255})
#define FCS_COLOR_TRANSPARENT ((FCS_Color){0,0,0,0})

typedef struct
{
    FCS_Color color;
    float     position;
} FCS_ColorStop;

typedef struct
{
    int            angle_deg;
    FCS_ColorStop* stops;
    int            stop_count;
} FCS_Gradient;

typedef enum
{
    FCS_DISPLAY_ELEMENT      = 0,
    FCS_DISPLAY_BLOCK        = 1,
    FCS_DISPLAY_INLINE_BLOCK = 2,
    FCS_DISPLAY_INLINE       = 3,
    FCS_DISPLAY_NONE         = 4,
    FCS_DISPLAY_FLEX         = 5,
} FCS_Display;

typedef enum
{
    FCS_TEXTALIGN_LEFT   = 0,
    FCS_TEXTALIGN_RIGHT  = 1,
    FCS_TEXTALIGN_CENTER = 2,
} FCS_TextAlign;

typedef enum
{
    FCS_HALIGN_DEFAULT  = 0,
    FCS_HALIGN_BASELINE = 1,
    FCS_HALIGN_TOP      = 2,
    FCS_HALIGN_BOTTOM   = 3,
    FCS_HALIGN_MIDDLE   = 4,
} FCS_HAlign;

typedef enum
{
    FCS_BORDER_NONE   = 0,
    FCS_BORDER_SOLID  = 1,
    FCS_BORDER_DASHED = 2,
    FCS_BORDER_DOTTED = 3,
} FCS_BorderStyle;

typedef enum
{
    FCS_FLEX_ROW            = 0,
    FCS_FLEX_ROW_REVERSE    = 1,
    FCS_FLEX_COLUMN         = 2,
    FCS_FLEX_COLUMN_REVERSE = 3,
} FCS_FlexDirection;

typedef enum
{
    FCS_FLEX_NOWRAP       = 0,
    FCS_FLEX_WRAP         = 1,
    FCS_FLEX_WRAP_REVERSE = 2,
} FCS_FlexWrap;

typedef enum
{
    FCS_JUSTIFY_START         = 0,
    FCS_JUSTIFY_END           = 1,
    FCS_JUSTIFY_CENTER        = 2,
    FCS_JUSTIFY_SPACE_BETWEEN = 3,
    FCS_JUSTIFY_SPACE_AROUND  = 4,
    FCS_JUSTIFY_SPACE_EVENLY  = 5,
} FCS_JustifyContent;

typedef enum
{
    FCS_ALIGN_STRETCH  = 0,
    FCS_ALIGN_START    = 1,
    FCS_ALIGN_END      = 2,
    FCS_ALIGN_CENTER   = 3,
    FCS_ALIGN_BASELINE = 4,
} FCS_AlignItems;

typedef struct { int top, right, bottom, left; } FCS_Spacing;

typedef struct
{
    int             width;
    FCS_BorderStyle style;
    FCS_Color       color;
} FCS_Border;

typedef struct
{
    int       x, y, blur, spread;
    FCS_Color color;
    int       inset;
} FCS_BoxShadow;

#define FCS_F_COLOR           (1u <<  0)
#define FCS_F_BG_COLOR        (1u <<  1)
#define FCS_F_BG_GRADIENT     (1u <<  2)
#define FCS_F_FONT_FAMILY     (1u <<  3)
#define FCS_F_FONT_WEIGHT     (1u <<  4)
#define FCS_F_DISPLAY         (1u <<  5)
#define FCS_F_TEXT_ALIGN      (1u <<  6)
#define FCS_F_HALIGN          (1u <<  7)
#define FCS_F_MARGIN          (1u <<  8)
#define FCS_F_PADDING         (1u <<  9)
#define FCS_F_BOX_SHADOW      (1u << 10)
#define FCS_F_BORDER          (1u << 11)
#define FCS_F_BORDER_RADIUS   (1u << 12)
#define FCS_F_IMAGE           (1u << 13)
#define FCS_F_FLEX_DIRECTION  (1u << 14)
#define FCS_F_FLEX_WRAP       (1u << 15)
#define FCS_F_JUSTIFY_CONTENT (1u << 16)
#define FCS_F_ALIGN_ITEMS     (1u << 17)
#define FCS_F_ALIGN_CONTENT   (1u << 18)
#define FCS_F_FLEX_GROW       (1u << 19)
#define FCS_F_FLEX_SHRINK     (1u << 20)
#define FCS_F_FLEX_BASIS      (1u << 21)
#define FCS_F_FONT_SIZE       (1u << 22)
#define FCS_F_BG_IMAGE        (1u << 23)
#define FCS_F_CURSOR          (1u << 24)
#define FCS_F_Z_INDEX         (1u << 25)
#define FCS_F_BORDER_TOP      (1u << 26)
#define FCS_F_BORDER_RIGHT    (1u << 27)
#define FCS_F_BORDER_BOTTOM   (1u << 28)
#define FCS_F_BORDER_LEFT     (1u << 29)

typedef enum
{
    FCS_CURSOR_DEFAULT     = 0,
    FCS_CURSOR_POINTER     = 1,
    FCS_CURSOR_TEXT        = 2,
    FCS_CURSOR_MOVE        = 3,
    FCS_CURSOR_CROSSHAIR   = 4,
    FCS_CURSOR_WAIT        = 5,
    FCS_CURSOR_HELP        = 6,
    FCS_CURSOR_NOT_ALLOWED = 7,
    FCS_CURSOR_NONE        = 8,
    FCS_CURSOR_EW_RESIZE   = 9,
    FCS_CURSOR_NS_RESIZE   = 10,
    FCS_CURSOR_GRAB        = 11,
    FCS_CURSOR_CUSTOM      = 12,
} FCS_Cursor;

typedef struct { int top_left, top_right, bottom_left, bottom_right; } FCS_BorderRadius;

typedef struct FCS_ComputedStyle
{
    uint32_t           flags;

    FCS_Color          color;
    FCS_Color          bg_color;
    FCS_Gradient*      bg_gradient;

    char*              font_family;
    int                font_weight;
    int                font_size;

    FCS_Display        display;
    FCS_TextAlign      text_align;
    FCS_HAlign         halign;

    FCS_Spacing        margin;
    FCS_Spacing        padding;

    FCS_BoxShadow      box_shadow;
    FCS_Border         border;
    FCS_BorderRadius   border_radius;

    char*              image;
    char*              bg_image;
    void*              image_cache;
    void*              bg_image_cache;

    FCS_FlexDirection  flex_direction;
    FCS_FlexWrap       flex_wrap;
    FCS_JustifyContent justify_content;
    FCS_AlignItems     align_items;
    FCS_AlignItems     align_content;
    float              flex_grow;
    float              flex_shrink;
    int                flex_basis;

    FCS_Cursor         cursor;
    char*              cursor_image;
    void*              cursor_cache;
    int                z_index;

    FCS_Border         border_top;
    FCS_Border         border_right;
    FCS_Border         border_bottom;
    FCS_Border         border_left;
} FCS_ComputedStyle;

#define FCS_SEL_TAG   0
#define FCS_SEL_CLASS 1
#define FCS_SEL_ID    2
#define FCS_SEL_ROOT  3

#define FCS_PSEUDO_NONE   0
#define FCS_PSEUDO_HOVER  1
#define FCS_PSEUDO_ACTIVE 2

typedef struct
{
    int    kind;
    int    pseudo;
    char*  name;
    char** prop_names;
    char** prop_values;
    int    decl_count;
} FCS_Rule;

typedef struct
{
    FCS_Rule* rules;
    int       rule_count;
    char**    var_names;
    char**    var_values;
    int       var_count;
} FCS_Stylesheet;

FCS_Stylesheet* FCS_loadFile(const char* path);
FCS_Stylesheet* FCS_parseString(const char* src);
void            FCS_freeStylesheet(FCS_Stylesheet* sheet);

void FCS_applyToElement(FCS_Stylesheet* sheet, struct FOUR_Element* elem);
void FCS_applyToDocument(FCS_Stylesheet* sheet, struct FOUR_Document* doc);

void FCS_freeComputedStyle(FCS_ComputedStyle* style);
const char* FCS_resolveVar(FCS_Stylesheet* sheet, const char* name);

#endif
#ifdef __cplusplus
}
#endif

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

#include "fourtitudec17.h"
#include "FourtitudeStyle.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static char* fcs_strdup(const char* s)
{
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

static char* fcs_strndup(const char* s, size_t n)
{
    char* copy = malloc(n + 1);
    if (copy) { memcpy(copy, s, n); copy[n] = '\0'; }
    return copy;
}

static void fcs_str_trim(char* s)
{
    if (!s) return;
    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    char* start = s;
    while (isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
}

typedef struct
{
    const char* src;
    const char* pos;
} FCS_Parser;

static void p_skip_ws(FCS_Parser* p)
{
    for (;;)
    {
        while (*p->pos && isspace((unsigned char)*p->pos)) p->pos++;
        if (p->pos[0] == '/' && p->pos[1] == '*')
        {
            p->pos += 2;
            while (*p->pos && !(p->pos[0] == '*' && p->pos[1] == '/')) p->pos++;
            if (*p->pos) p->pos += 2;
        }
        else break;
    }
}

static char* p_read_selector(FCS_Parser* p)
{
    p_skip_ws(p);
    const char* start = p->pos;
    while (*p->pos && *p->pos != '{') p->pos++;
    char* sel = fcs_strndup(start, (size_t)(p->pos - start));
    fcs_str_trim(sel);
    return sel;
}

static char* p_read_ident(FCS_Parser* p)
{
    p_skip_ws(p);
    const char* start = p->pos;
    while (*p->pos && *p->pos != ':' && *p->pos != ';' && *p->pos != '}' && !isspace((unsigned char)*p->pos))
        p->pos++;
    return fcs_strndup(start, (size_t)(p->pos - start));
}

static char* p_read_value(FCS_Parser* p)
{
    p_skip_ws(p);

    if (*p->pos == '"' || *p->pos == '\'')
    {
        char quote = *p->pos++;
        const char* start = p->pos;
        while (*p->pos && *p->pos != quote) p->pos++;
        char* val = fcs_strndup(start, (size_t)(p->pos - start));
        if (*p->pos == quote) p->pos++;
        p_skip_ws(p);
        if (*p->pos == ';') p->pos++;
        return val;
    }

    const char* start = p->pos;
    int depth = 0;
    while (*p->pos)
    {
        if (*p->pos == '(') { depth++; p->pos++; }
        else if (*p->pos == ')') { depth--; p->pos++; }
        else if (*p->pos == ';' && depth == 0) break;
        else if (*p->pos == '}' && depth == 0) break;
        else p->pos++;
    }
    char* val = fcs_strndup(start, (size_t)(p->pos - start));
    fcs_str_trim(val);
    return val;
}

static void rule_add_decl(FCS_Rule* rule, char* name, char* value)
{
    rule->prop_names  = realloc(rule->prop_names,  sizeof(char*) * (rule->decl_count + 1));
    rule->prop_values = realloc(rule->prop_values, sizeof(char*) * (rule->decl_count + 1));
    rule->prop_names [rule->decl_count] = name;
    rule->prop_values[rule->decl_count] = value;
    rule->decl_count++;
}

static void sheet_add_rule(FCS_Stylesheet* sheet, FCS_Rule rule)
{
    sheet->rules = realloc(sheet->rules, sizeof(FCS_Rule) * (sheet->rule_count + 1));
    sheet->rules[sheet->rule_count++] = rule;
}

static void sheet_add_var(FCS_Stylesheet* sheet, const char* name, const char* value)
{
    for (int i = 0; i < sheet->var_count; i++)
    {
        if (strcmp(sheet->var_names[i], name) == 0)
        {
            free(sheet->var_values[i]);
            sheet->var_values[i] = fcs_strdup(value);
            return;
        }
    }
    sheet->var_names  = realloc(sheet->var_names,  sizeof(char*) * (sheet->var_count + 1));
    sheet->var_values = realloc(sheet->var_values, sizeof(char*) * (sheet->var_count + 1));
    sheet->var_names [sheet->var_count] = fcs_strdup(name);
    sheet->var_values[sheet->var_count] = fcs_strdup(value);
    sheet->var_count++;
}

static FCS_Stylesheet* parse_internal(const char* src)
{
    FCS_Stylesheet* sheet = calloc(1, sizeof(FCS_Stylesheet));
    FCS_Parser p = { src, src };

    while (*p.pos)
    {
        p_skip_ws(&p);
        if (!*p.pos) break;

        char* selector = p_read_selector(&p);
        if (!*p.pos || *p.pos != '{') { free(selector); break; }
        p.pos++;
 
        char* sel_copy = fcs_strdup(selector);
        free(selector);
 
        FCS_Rule* rules = NULL;
        int rule_count = 0;

        char* tok = strtok(sel_copy, ",");
        while (tok)
        {
            fcs_str_trim(tok);

            FCS_Rule rule = {0};

            int pseudo = FCS_PSEUDO_NONE;
            char* colon = strrchr(tok, ':');
            if (colon && colon != tok)
            {
                if (strcmp(colon, ":hover")  == 0) { pseudo = FCS_PSEUDO_HOVER;  *colon = '\0'; }
                else if (strcmp(colon, ":active") == 0) { pseudo = FCS_PSEUDO_ACTIVE; *colon = '\0'; }
            }
            rule.pseudo = pseudo;

            if (tok[0] == '#') { rule.kind = FCS_SEL_ID;    rule.name = fcs_strdup(tok + 1); }
            else if (tok[0] == '.') { rule.kind = FCS_SEL_CLASS; rule.name = fcs_strdup(tok + 1); }
            else if (strcmp(tok, ":root") == 0) { rule.kind = FCS_SEL_ROOT; rule.name = fcs_strdup(""); }
            else { rule.kind = FCS_SEL_TAG;   rule.name = fcs_strdup(tok); }

            rules = realloc(rules, sizeof(FCS_Rule) * (rule_count + 1));
            rules[rule_count++] = rule;

            tok = strtok(NULL, ",");
        }
        free(sel_copy);
 
        char** prop_names = NULL;
        char** prop_values = NULL;
        int decl_count = 0;

        while (*p.pos)
        {
            p_skip_ws(&p);
            if (*p.pos == '}') { p.pos++; break; }

            char* prop = p_read_ident(&p);
            p_skip_ws(&p);
            if (*p.pos != ':') { free(prop); break; }
            p.pos++;

            char* val = p_read_value(&p);
            if (*p.pos == ';') p.pos++;
 
            if (prop[0] == '-' && prop[1] == '-')
            {
                sheet_add_var(sheet, prop, val);
                free(prop); free(val);
                continue;
            }

            prop_names  = realloc(prop_names,  sizeof(char*) * (decl_count + 1));
            prop_values = realloc(prop_values, sizeof(char*) * (decl_count + 1));
            prop_names [decl_count] = prop;
            prop_values[decl_count] = val;
            decl_count++;
        }
 
        for (int i = 0; i < rule_count; i++)
        {
            FCS_Rule rule = rules[i];

            for (int j = 0; j < decl_count; j++)
            {
                rule_add_decl(&rule, fcs_strdup(prop_names[j]), fcs_strdup(prop_values[j]));
            }

            if (rule.kind != FCS_SEL_ROOT || rule.decl_count > 0) sheet_add_rule(sheet, rule);
            else free(rule.name);
        }
 
        for (int i = 0; i < decl_count; i++)
        {
            free(prop_names[i]);
            free(prop_values[i]);
        }
        free(prop_names);
        free(prop_values);
        free(rules);
    }

    return sheet;
}
FCS_Stylesheet* FCS_parseString(const char* src)
{
    return parse_internal(src);
}

FCS_Stylesheet* FCS_loadFile(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc((size_t)size + 1);
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    FCS_Stylesheet* sheet = parse_internal(buf);
    free(buf);
    return sheet;
}

void FCS_freeStylesheet(FCS_Stylesheet* sheet)
{
    if (!sheet) return;
    for (int i = 0; i < sheet->rule_count; i++)
    {
        FCS_Rule* r = &sheet->rules[i];
        free(r->name);
        for (int j = 0; j < r->decl_count; j++) { free(r->prop_names[j]); free(r->prop_values[j]); }
        free(r->prop_names);
        free(r->prop_values);
    }
    free(sheet->rules);
    for (int i = 0; i < sheet->var_count; i++) { free(sheet->var_names[i]); free(sheet->var_values[i]); }
    free(sheet->var_names);
    free(sheet->var_values);
    free(sheet);
}

void FCS_freeComputedStyle(FCS_ComputedStyle* style)
{
    if (!style) return;
    free(style->font_family);
    free(style->image);
    free(style->bg_image);
    free(style->cursor_image);
    FOUR_FreeImageCache(&style->image_cache);
    FOUR_FreeImageCache(&style->bg_image_cache);
    FOUR_FreeImageCache(&style->cursor_cache);
    if (style->bg_gradient)
    {
        free(style->bg_gradient->stops);
        free(style->bg_gradient);
    }
    free(style);
}

const char* FCS_resolveVar(FCS_Stylesheet* sheet, const char* name)
{
    if (!sheet || !name) return NULL;
    for (int i = 0; i < sheet->var_count; i++) if (strcmp(sheet->var_names[i], name) == 0) return sheet->var_values[i];
    return NULL;
}

static char* resolve_vars_in(FCS_Stylesheet* sheet, const char* raw)
{
    char* result = fcs_strdup(raw);
    char* var_start;
    while ((var_start = strstr(result, "var(")) != NULL)
    {
        char* name_start = var_start + 4;
        char* paren_end  = strchr(name_start, ')');
        if (!paren_end) break;
        char* var_name = fcs_strndup(name_start, (size_t)(paren_end - name_start));
        fcs_str_trim(var_name);
        const char* replacement = FCS_resolveVar(sheet, var_name);
        free(var_name);
        if (!replacement) break;
        size_t before = (size_t)(var_start - result);
        size_t after  = strlen(paren_end + 1);
        size_t rlen   = strlen(replacement);
        char* next = malloc(before + rlen + after + 1);
        memcpy(next, result, before);
        memcpy(next + before, replacement, rlen);
        memcpy(next + before + rlen, paren_end + 1, after + 1);
        free(result);
        result = next;
    }
    return result;
}

typedef struct { const char* name; uint8_t r, g, b; } FCS_NamedColor;
static const FCS_NamedColor NAMED_COLORS[] =
{
    { "black",   0,   0,   0   }, { "white",   255, 255, 255 },
    { "red",     255, 0,   0   }, { "green",   0,   128, 0   },
    { "blue",    0,   0,   255 }, { "yellow",  255, 255, 0   },
    { "cyan",    0,   255, 255 }, { "magenta", 255, 0,   255 },
    { "orange",  255, 165, 0   }, { "purple",  128, 0,   128 },
    { "pink",    255, 192, 203 }, { "brown",   165, 42,  42  },
    { "gray",    128, 128, 128 }, { "grey",    128, 128, 128 },
    { "silver",  192, 192, 192 }, { "gold",    255, 215, 0   },
    { "navy",    0,   0,   128 }, { "teal",    0,   128, 128 },
    { "maroon",  128, 0,   0   }, { "lime",    0,   255, 0   },
    { "transparent", 0, 0, 0   },
    { NULL, 0, 0, 0 }
};

static int parse_hex2(const char* s) { int v = 0; for (int i = 0; i < 2; i++) { v <<= 4; char c = s[i]; v += (c >= '0' && c <= '9') ? c-'0' : (c >= 'a' && c <= 'f') ? c-'a'+10 : c-'A'+10; } return v; }
static int parse_hex1(char c) { return (c >= '0' && c <= '9') ? c-'0' : (c >= 'a' && c <= 'f') ? c-'a'+10 : c-'A'+10; }

static int parse_color_str(const char* raw, FCS_Color* out)
{
    if (!raw || !*raw) return 0;
    const char* s = raw;
    while (isspace((unsigned char)*s)) s++;

    if (*s == '#')
    {
        s++;
        size_t len = strlen(s);
        if (len >= 6)
        {
            out->r = (uint8_t)parse_hex2(s);
            out->g = (uint8_t)parse_hex2(s+2);
            out->b = (uint8_t)parse_hex2(s+4);
            out->a = len >= 8 ? (uint8_t)parse_hex2(s+6) : 255;
            return 1;
        }
        if (len >= 3)
        {
            out->r = (uint8_t)(parse_hex1(s[0]) * 17);
            out->g = (uint8_t)(parse_hex1(s[1]) * 17);
            out->b = (uint8_t)(parse_hex1(s[2]) * 17);
            out->a = len >= 4 ? (uint8_t)(parse_hex1(s[3]) * 17) : 255;
            return 1;
        }
        return 0;
    }

    if (strncmp(s, "rgb(", 4) == 0 || strncmp(s, "rgba(", 5) == 0)
    {
        int is_rgba = (strncmp(s, "rgba(", 5) == 0);
        const char* p = s + (is_rgba ? 5 : 4);
        int vals[4] = {0, 0, 0, 255};
        int n = is_rgba ? 4 : 3;
        for (int i = 0; i < n; i++)
        {
            while (isspace((unsigned char)*p) || *p == ',') p++;
            char* end;
            if (i == 3)
            {
                double a = strtod(p, &end);
                vals[3] = (int)((a <= 1.0 ? a * 255.0 : a) + 0.5);
                if (vals[3] > 255) vals[3] = 255;
            }
            else vals[i] = (int)strtol(p, &end, 10);
            p = end;
        }
        out->r = (uint8_t)vals[0]; out->g = (uint8_t)vals[1];
        out->b = (uint8_t)vals[2]; out->a = (uint8_t)vals[3];
        return 1;
    }

    for (int i = 0; NAMED_COLORS[i].name; i++)
    {
        if (strcmp(s, NAMED_COLORS[i].name) == 0)
        {
            out->r = NAMED_COLORS[i].r;
            out->g = NAMED_COLORS[i].g;
            out->b = NAMED_COLORS[i].b;
            out->a = (strcmp(s, "transparent") == 0) ? 0 : 255;
            return 1;
        }
    }
    return 0;
}

static int parse_px(const char* s)
{
    if (!s) return 0;
    return (int)strtol(s, NULL, 10);
}

static void parse_spacing(const char* raw, FCS_Spacing* out)
{
    char buf[256];
    strncpy(buf, raw, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    int vals[4] = {0,0,0,0};
    int count = 0;
    char* tok = strtok(buf, " ");
    while (tok && count < 4) { vals[count++] = parse_px(tok); tok = strtok(NULL, " "); }
    if      (count == 1) { out->top=out->right=out->bottom=out->left=vals[0]; }
    else if (count == 2) { out->top=out->bottom=vals[0]; out->right=out->left=vals[1]; }
    else if (count == 3) { out->top=vals[0]; out->right=out->left=vals[1]; out->bottom=vals[2]; }
    else if (count == 4) { out->top=vals[0]; out->right=vals[1]; out->bottom=vals[2]; out->left=vals[3]; }
}

static FCS_Gradient* parse_linear_gradient(const char* raw, FCS_Stylesheet* sheet)
{
    const char* inner_start = strchr(raw, '(');
    if (!inner_start) return NULL;
    inner_start++;
    const char* inner_end = strrchr(raw, ')');
    if (!inner_end) return NULL;
    char* inner = fcs_strndup(inner_start, (size_t)(inner_end - inner_start));
    char* resolved = resolve_vars_in(sheet, inner);
    free(inner);

    FCS_Gradient* g = calloc(1, sizeof(FCS_Gradient));
    g->angle_deg = 180;

    char* tok = strtok(resolved, ",");
    while (tok)
    {
        while (isspace((unsigned char)*tok)) tok++;
        if (strncmp(tok, "to ", 3) == 0)
        {
            if (strstr(tok, "bottom")) g->angle_deg = 180;
            else if (strstr(tok, "top")) g->angle_deg = 0;
            else if (strstr(tok, "right")) g->angle_deg = 90;
            else if (strstr(tok, "left")) g->angle_deg = 270;
        }
        else if (isdigit((unsigned char)*tok) && strstr(tok, "deg"))
        {
            g->angle_deg = atoi(tok);
        }
        else
        {
            FCS_Color c = {0};
            char color_part[64] = {0};
            float pos = -1.0f;
            char* space = strchr(tok, ' ');
            if (space) { strncpy(color_part, tok, (size_t)(space - tok)); pos = (float)atof(space+1) / 100.0f; }
            else strncpy(color_part, tok, sizeof(color_part)-1);
            if (parse_color_str(color_part, &c))
            {
                g->stops = realloc(g->stops, sizeof(FCS_ColorStop) * (g->stop_count + 1));
                g->stops[g->stop_count].color    = c;
                g->stops[g->stop_count].position = pos;
                g->stop_count++;
            }
        }
        tok = strtok(NULL, ",");
    }
    free(resolved);

    for (int i = 0; i < g->stop_count; i++)
        if (g->stops[i].position < 0) g->stops[i].position = (g->stop_count == 1) ? 0.0f : (float)i / (float)(g->stop_count - 1);

    return g;
}

static void parse_border_shorthand(const char* raw, FCS_Stylesheet* sheet, FCS_Border* out)
{
    out->width = 1;
    out->style = FCS_BORDER_SOLID;

    char part[64];
    const char* p = raw;
    
    while (*p)
    {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        int len = 0;
        while (p[len] && !isspace((unsigned char)p[len])) len++;
        if (len >= 64) len = 63;
        strncpy(part, p, len);
        part[len] = '\0';
        p += len;
        if (isdigit((unsigned char)part[0]))
        {
            out->width = parse_px(part);
        }
        else if (strcmp(part, "solid") == 0)
        {
            out->style = FCS_BORDER_SOLID;
        } 
        else if (strcmp(part, "dashed") == 0)
        {
            out->style = FCS_BORDER_DASHED;
        } 
        else if (strcmp(part, "dotted") == 0)
        {
            out->style = FCS_BORDER_DOTTED;
        } 
        else if (strcmp(part, "none") == 0)
        {
            out->style = FCS_BORDER_NONE;
        }
        else
        {
            FCS_Color c = {0};
            if (parse_color_str(part, &c))
            {
                out->color = c;
            }
        }
    }
    (void)sheet;
}

static void apply_decl(FCS_ComputedStyle* cs, const char* prop, const char* raw_val, FCS_Stylesheet* sheet)
{
    char* val = resolve_vars_in(sheet, raw_val);

    if (strcmp(prop, "color") == 0)
    {
        if (parse_color_str(val, &cs->color)) cs->flags |= FCS_F_COLOR;
    }
    else if (strcmp(prop, "background-color") == 0 || strcmp(prop, "bg-color") == 0)
    {
        if (parse_color_str(val, &cs->bg_color)) cs->flags |= FCS_F_BG_COLOR;
    }
    else if (strcmp(prop, "background") == 0 || strcmp(prop, "bg") == 0)
    {
        if (strstr(val, "linear-gradient"))
        {
            FCS_Gradient* g = parse_linear_gradient(val, sheet);
            if (g)
            {
                if (cs->bg_gradient) { free(cs->bg_gradient->stops); free(cs->bg_gradient); }
                cs->bg_gradient = g;
                cs->flags |= FCS_F_BG_GRADIENT;
            }
        }
        else if (parse_color_str(val, &cs->bg_color)) cs->flags |= FCS_F_BG_COLOR;
    }
    else if (strcmp(prop, "font-family") == 0)
    {
        free(cs->font_family);
        cs->font_family = fcs_strdup(val);
        cs->flags |= FCS_F_FONT_FAMILY;
    }
    else if (strcmp(prop, "font-weight") == 0)
    {
        if (strcmp(val,"bold")==0) cs->font_weight = 700;
        else if (strcmp(val,"normal")==0) cs->font_weight = 400;
        else if (strcmp(val,"bolder")==0) cs->font_weight = 700;
        else if (strcmp(val,"lighter")==0) cs->font_weight = 300;
        else cs->font_weight = atoi(val);
        cs->flags |= FCS_F_FONT_WEIGHT;
    }
    else if (strcmp(prop, "display") == 0)
    {
        if (strcmp(val,"none")==0) cs->display = FCS_DISPLAY_NONE;
        else if (strcmp(val,"block")==0) cs->display = FCS_DISPLAY_BLOCK;
        else if (strcmp(val,"inline-block")==0) cs->display = FCS_DISPLAY_INLINE_BLOCK;
        else if (strcmp(val,"inline")==0)        cs->display = FCS_DISPLAY_INLINE;
        else if (strcmp(val,"flex")==0 || strcmp(val,"flexbox")==0) cs->display = FCS_DISPLAY_FLEX;
        else cs->display = FCS_DISPLAY_ELEMENT;
        cs->flags |= FCS_F_DISPLAY;
    }
    else if (strcmp(prop, "align-text") == 0 || strcmp(prop, "text-align") == 0)
    {
        if (strcmp(val,"right")==0)  cs->text_align = FCS_TEXTALIGN_RIGHT;
        else if (strcmp(val,"center")==0) cs->text_align = FCS_TEXTALIGN_CENTER;
        else cs->text_align = FCS_TEXTALIGN_LEFT;
        cs->flags |= FCS_F_TEXT_ALIGN;
    }
    else if (strcmp(prop, "halign-text") == 0)
    {
        if (strcmp(val,"baseline")==0) cs->halign = FCS_HALIGN_BASELINE;
        else if (strcmp(val,"top")==0)      cs->halign = FCS_HALIGN_TOP;
        else if (strcmp(val,"bottom")==0)   cs->halign = FCS_HALIGN_BOTTOM;
        else if (strcmp(val,"middle")==0)   cs->halign = FCS_HALIGN_MIDDLE;
        else cs->halign = FCS_HALIGN_DEFAULT;
        cs->flags |= FCS_F_HALIGN;
    }
    else if (strcmp(prop, "margin") == 0) { parse_spacing(val, &cs->margin);  cs->flags |= FCS_F_MARGIN; }
    else if (strcmp(prop, "margin-top") == 0) { cs->margin.top    = parse_px(val); cs->flags |= FCS_F_MARGIN; }
    else if (strcmp(prop, "margin-right") == 0) { cs->margin.right  = parse_px(val); cs->flags |= FCS_F_MARGIN; }
    else if (strcmp(prop, "margin-bottom") == 0) { cs->margin.bottom = parse_px(val); cs->flags |= FCS_F_MARGIN; }
    else if (strcmp(prop, "margin-left") == 0) { cs->margin.left   = parse_px(val); cs->flags |= FCS_F_MARGIN; }
    else if (strcmp(prop, "padding") == 0) { parse_spacing(val, &cs->padding); cs->flags |= FCS_F_PADDING; }
    else if (strcmp(prop, "padding-top") == 0) { cs->padding.top    = parse_px(val); cs->flags |= FCS_F_PADDING; }
    else if (strcmp(prop, "padding-right") == 0) { cs->padding.right  = parse_px(val); cs->flags |= FCS_F_PADDING; }
    else if (strcmp(prop, "padding-bottom") == 0) { cs->padding.bottom = parse_px(val); cs->flags |= FCS_F_PADDING; }
    else if (strcmp(prop, "padding-left") == 0) { cs->padding.left   = parse_px(val); cs->flags |= FCS_F_PADDING; }
    else if (strcmp(prop, "border") == 0) { parse_border_shorthand(val, sheet, &cs->border); cs->flags |= FCS_F_BORDER; }
    else if (strcmp(prop, "border-top") == 0) { parse_border_shorthand(val, sheet, &cs->border_top);    cs->flags |= FCS_F_BORDER_TOP; }
    else if (strcmp(prop, "border-right") == 0) { parse_border_shorthand(val, sheet, &cs->border_right);  cs->flags |= FCS_F_BORDER_RIGHT; }
    else if (strcmp(prop, "border-bottom") == 0) { parse_border_shorthand(val, sheet, &cs->border_bottom); cs->flags |= FCS_F_BORDER_BOTTOM; }
    else if (strcmp(prop, "border-left") == 0) { parse_border_shorthand(val, sheet, &cs->border_left);   cs->flags |= FCS_F_BORDER_LEFT; }
    else if (strcmp(prop, "border-width") == 0) { cs->border.width = parse_px(val); cs->flags |= FCS_F_BORDER; }
    else if (strcmp(prop, "border-style") == 0)
    {
        if (strcmp(val,"solid")==0)  cs->border.style = FCS_BORDER_SOLID;
        else if (strcmp(val,"dashed")==0) cs->border.style = FCS_BORDER_DASHED;
        else if (strcmp(val,"dotted")==0) cs->border.style = FCS_BORDER_DOTTED;
        else cs->border.style = FCS_BORDER_NONE;
        cs->flags |= FCS_F_BORDER;
    }
    else if (strcmp(prop, "border-color") == 0)
    {
        parse_color_str(val, &cs->border.color);
        cs->flags |= FCS_F_BORDER;
    }
    else if (strcmp(prop, "border-radius") == 0)
    {
        char* tmp = fcs_strdup(val);
        char* tok = strtok(tmp, " ");
        int v[4] = {0, 0, 0, 0}; int n = 0;
        while (tok && n < 4) { v[n++] = parse_px(tok); tok = strtok(NULL, " "); }
        if (n == 1) { cs->border_radius.top_left = cs->border_radius.top_right = cs->border_radius.bottom_left = cs->border_radius.bottom_right = v[0]; }
        else if (n == 2) { cs->border_radius.top_left = cs->border_radius.bottom_right = v[0]; cs->border_radius.top_right = cs->border_radius.bottom_left = v[1]; }
        else if (n == 4) { cs->border_radius.top_left = v[0]; cs->border_radius.top_right = v[1]; cs->border_radius.bottom_right = v[2]; cs->border_radius.bottom_left = v[3]; }
        cs->flags |= FCS_F_BORDER_RADIUS;
        free(tmp);
    }
    else if (strcmp(prop, "border-top-left-radius")     == 0) { cs->border_radius.top_left     = parse_px(val); cs->flags |= FCS_F_BORDER_RADIUS; }
    else if (strcmp(prop, "border-top-right-radius")    == 0) { cs->border_radius.top_right    = parse_px(val); cs->flags |= FCS_F_BORDER_RADIUS; }
    else if (strcmp(prop, "border-bottom-left-radius")  == 0) { cs->border_radius.bottom_left  = parse_px(val); cs->flags |= FCS_F_BORDER_RADIUS; }
    else if (strcmp(prop, "border-bottom-right-radius") == 0) { cs->border_radius.bottom_right = parse_px(val); cs->flags |= FCS_F_BORDER_RADIUS; }
    else if (strcmp(prop, "box-shadow") == 0)
    {
        int x=0,y=0,blur=0,spread=0;
        FCS_Color c = FCS_COLOR_BLACK;
        char buf[256]; strncpy(buf, val, sizeof(buf)-1);
        char* tok = strtok(buf, " ");
        int idx = 0;
        while (tok)
        {
            FCS_Color tc = {0};
            if (strcmp(tok,"inset")==0) cs->box_shadow.inset = 1;
            else if (parse_color_str(tok, &tc)) c = tc;
            else { int v = parse_px(tok); if (idx==0) x=v; else if (idx==1) y=v; else if (idx==2) blur=v; else spread=v; idx++; }
            tok = strtok(NULL, " ");
        }
        cs->box_shadow.x=x; cs->box_shadow.y=y; cs->box_shadow.blur=blur;
        cs->box_shadow.spread=spread; cs->box_shadow.color=c;
        cs->flags |= FCS_F_BOX_SHADOW;
    }
    else if (strcmp(prop, "image") == 0)
    {
        free(cs->image);
        cs->image = fcs_strdup(val);
        FOUR_FreeImageCache(&cs->image_cache);
        cs->flags |= FCS_F_IMAGE;
    }
    else if (strcmp(prop, "background-image") == 0)
    {
        const char* path = val;
        if (strncmp(val, "url(", 4) == 0)
        {
            char* inner = fcs_strdup(val + 4);
            char* end   = strrchr(inner, ')');
            if (end) *end = '\0';
            if ((*inner == '"' || *inner == '\'') && inner[strlen(inner)-1] == inner[0])
            {
                inner[strlen(inner)-1] = '\0';
                memmove(inner, inner + 1, strlen(inner));
            }
            free(cs->bg_image);
            cs->bg_image = inner;
        }
        else
        {
            free(cs->bg_image);
            cs->bg_image = fcs_strdup(path);
        }
        FOUR_FreeImageCache(&cs->bg_image_cache);
        cs->flags |= FCS_F_BG_IMAGE;
    }
    else if (strcmp(prop, "flex-direction") == 0)
    {
        if      (strcmp(val,"row")==0) cs->flex_direction = FCS_FLEX_ROW;
        else if (strcmp(val,"row-reverse")==0) cs->flex_direction = FCS_FLEX_ROW_REVERSE;
        else if (strcmp(val,"column")==0) cs->flex_direction = FCS_FLEX_COLUMN;
        else if (strcmp(val,"column-reverse")==0) cs->flex_direction = FCS_FLEX_COLUMN_REVERSE;
        cs->flags |= FCS_F_FLEX_DIRECTION;
    }
    else if (strcmp(prop, "flex-wrap") == 0)
    {
        if (strcmp(val,"wrap")==0) cs->flex_wrap = FCS_FLEX_WRAP;
        else if (strcmp(val,"wrap-reverse")==0) cs->flex_wrap = FCS_FLEX_WRAP_REVERSE;
        else cs->flex_wrap = FCS_FLEX_NOWRAP;
        cs->flags |= FCS_F_FLEX_WRAP;
    }
    else if (strcmp(prop, "justify-content") == 0)
    {
        if      (strcmp(val,"flex-end")==0||strcmp(val,"end")==0) cs->justify_content = FCS_JUSTIFY_END;
        else if (strcmp(val,"center")==0) cs->justify_content = FCS_JUSTIFY_CENTER;
        else if (strcmp(val,"space-between")==0) cs->justify_content = FCS_JUSTIFY_SPACE_BETWEEN;
        else if (strcmp(val,"space-around")==0) cs->justify_content = FCS_JUSTIFY_SPACE_AROUND;
        else if (strcmp(val,"space-evenly")==0) cs->justify_content = FCS_JUSTIFY_SPACE_EVENLY;
        else cs->justify_content = FCS_JUSTIFY_START;
        cs->flags |= FCS_F_JUSTIFY_CONTENT;
    }
    else if (strcmp(prop, "align-items") == 0)
    {
        if      (strcmp(val,"flex-end")==0||strcmp(val,"end")==0) cs->align_items = FCS_ALIGN_END;
        else if (strcmp(val,"center")==0) cs->align_items = FCS_ALIGN_CENTER;
        else if (strcmp(val,"baseline")==0) cs->align_items = FCS_ALIGN_BASELINE;
        else if (strcmp(val,"stretch")==0) cs->align_items = FCS_ALIGN_STRETCH;
        else cs->align_items = FCS_ALIGN_START;
        cs->flags |= FCS_F_ALIGN_ITEMS;
    }
    else if (strcmp(prop, "align-content") == 0)
    {
        if      (strcmp(val,"flex-end")==0||strcmp(val,"end")==0) cs->align_content = FCS_ALIGN_END;
        else if (strcmp(val,"center")==0) cs->align_content = FCS_ALIGN_CENTER;
        else if (strcmp(val,"baseline")==0) cs->align_content = FCS_ALIGN_BASELINE;
        else if (strcmp(val,"stretch")==0) cs->align_content = FCS_ALIGN_STRETCH;
        else cs->align_content = FCS_ALIGN_START;
        cs->flags |= FCS_F_ALIGN_CONTENT;
    }
    else if (strcmp(prop, "flex-grow") == 0)   { cs->flex_grow   = (float)atof(val); cs->flags |= FCS_F_FLEX_GROW; }
    else if (strcmp(prop, "flex-shrink") == 0) { cs->flex_shrink = (float)atof(val); cs->flags |= FCS_F_FLEX_SHRINK; }
    else if (strcmp(prop, "flex-basis") == 0)  { cs->flex_basis  = parse_px(val);    cs->flags |= FCS_F_FLEX_BASIS; }
    else if (strcmp(prop, "font-size") == 0 || strcmp(prop, "text-size") == 0) { cs->font_size = parse_px(val); cs->flags |= FCS_F_FONT_SIZE; }
    else if (strcmp(prop, "z-index") == 0)
    {
        if (strcmp(val, "auto") == 0) cs->z_index = 0;
        else cs->z_index = atoi(val);
        cs->flags |= FCS_F_Z_INDEX;
    }
    else if (strcmp(prop, "cursor") == 0)
    {
        if      (strcmp(val, "pointer") == 0) { cs->cursor = FCS_CURSOR_POINTER; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "text") == 0) { cs->cursor = FCS_CURSOR_TEXT; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "move") == 0) { cs->cursor = FCS_CURSOR_MOVE; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "crosshair") == 0) { cs->cursor = FCS_CURSOR_CROSSHAIR; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "wait") == 0) { cs->cursor = FCS_CURSOR_WAIT; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "help") == 0) { cs->cursor = FCS_CURSOR_HELP; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "not-allowed") == 0) { cs->cursor = FCS_CURSOR_NOT_ALLOWED; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "none") == 0) { cs->cursor = FCS_CURSOR_NONE; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "ew-resize") == 0) { cs->cursor = FCS_CURSOR_EW_RESIZE; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "ns-resize") == 0) { cs->cursor = FCS_CURSOR_NS_RESIZE; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "grab") == 0) { cs->cursor = FCS_CURSOR_GRAB; cs->flags |= FCS_F_CURSOR; }
        else if (strcmp(val, "default") == 0) { cs->cursor = FCS_CURSOR_DEFAULT; cs->flags |= FCS_F_CURSOR; }
        else if (strncmp(val, "url(", 4) == 0)
        {
            const char* start = val + 4;
            const char* end   = strchr(start, ')');
            if (end)
            {
                free(cs->cursor_image);
                cs->cursor_image = fcs_strndup(start, (size_t)(end - start));
                cs->cursor = FCS_CURSOR_CUSTOM;
                cs->flags |= FCS_F_CURSOR;
            }
        }
    }

    free(val);
}

static void apply_rule(FCS_ComputedStyle* cs, const FCS_Rule* rule, FCS_Stylesheet* sheet)
{
    for (int i = 0; i < rule->decl_count; i++)
        apply_decl(cs, rule->prop_names[i], rule->prop_values[i], sheet);
}

static int elem_matches(const FCS_Rule* r, FOUR_Element* elem)
{
    if (r->kind == FCS_SEL_TAG   && elem->type  && strcmp(r->name, elem->type)  == 0) return 1;
    if (r->kind == FCS_SEL_CLASS && elem->clazz)
    {
        size_t nlen = strlen(r->name);
        const char* c = elem->clazz;
        while (*c)
        {
            while (*c == ' ') c++;
            const char* end = c;
            while (*end && *end != ' ') end++;
            if ((size_t)(end - c) == nlen && strncmp(c, r->name, nlen) == 0) return 1;
            c = end;
        }
    }
    if (r->kind == FCS_SEL_ID    && elem->id    && strcmp(r->name, elem->id)    == 0) return 1;
    return 0;
}

void FCS_applyToElement(FCS_Stylesheet* sheet, FOUR_Element* elem)
{
    if (!sheet || !elem) return;

    if (!elem->computed_style) elem->computed_style = calloc(1, sizeof(FCS_ComputedStyle));

    for (int pass = FCS_SEL_TAG; pass <= FCS_SEL_ID; pass++)
    {
        for (int i = 0; i < sheet->rule_count; i++)
        {
            const FCS_Rule* r = &sheet->rules[i];
            if (r->kind != pass || r->pseudo != FCS_PSEUDO_NONE) continue;
            if (elem_matches(r, elem)) apply_rule(elem->computed_style, r, sheet);
        }
    }

    for (int i = 0; i < sheet->rule_count; i++)
    {
        const FCS_Rule* r = &sheet->rules[i];
        if (r->pseudo == FCS_PSEUDO_NONE || r->kind == FCS_SEL_ROOT) continue;
        if (!elem_matches(r, elem)) continue;
        if (r->pseudo == FCS_PSEUDO_HOVER)
        {
            if (!elem->hover_style) elem->hover_style = calloc(1, sizeof(FCS_ComputedStyle));
            apply_rule(elem->hover_style, r, sheet);
        }
        else if (r->pseudo == FCS_PSEUDO_ACTIVE)
        {
            if (!elem->active_style) elem->active_style = calloc(1, sizeof(FCS_ComputedStyle));
            apply_rule(elem->active_style, r, sheet);
        }
    }
}

#define FCS_INHERITABLE_FLAGS (FCS_F_COLOR | FCS_F_FONT_FAMILY | FCS_F_FONT_WEIGHT | FCS_F_FONT_SIZE | FCS_F_TEXT_ALIGN | FCS_F_CURSOR)

static void inherit_from_cs(const FCS_ComputedStyle* pcs, FOUR_Element* elem)
{
    if (!pcs) return;
    if (!elem->computed_style) elem->computed_style = calloc(1, sizeof(FCS_ComputedStyle));
    FCS_ComputedStyle* cs = elem->computed_style;

    uint32_t inh = FCS_INHERITABLE_FLAGS & pcs->flags & ~cs->flags;
    if (!inh) return;

    if (inh & FCS_F_COLOR) { cs->color = pcs->color; cs->flags |= FCS_F_COLOR; }
    if (inh & FCS_F_FONT_FAMILY) { free(cs->font_family); cs->font_family = fcs_strdup(pcs->font_family); cs->flags |= FCS_F_FONT_FAMILY; }
    if (inh & FCS_F_FONT_WEIGHT) { cs->font_weight = pcs->font_weight; cs->flags |= FCS_F_FONT_WEIGHT; }
    if (inh & FCS_F_FONT_SIZE) { cs->font_size   = pcs->font_size; cs->flags |= FCS_F_FONT_SIZE; }
    if (inh & FCS_F_TEXT_ALIGN) { cs->text_align  = pcs->text_align; cs->flags |= FCS_F_TEXT_ALIGN; }
    if (inh & FCS_F_CURSOR) { cs->cursor = pcs->cursor; free(cs->cursor_image); cs->cursor_image = fcs_strdup(pcs->cursor_image); cs->flags |= FCS_F_CURSOR; }
}

void FCS_applyToDocument(FCS_Stylesheet* sheet, FOUR_Document* doc)
{
    if (!sheet || !doc) return;

    FCS_ComputedStyle root_cs = {0};
    for (int i = 0; i < sheet->rule_count; i++) if (sheet->rules[i].kind == FCS_SEL_ROOT) apply_rule(&root_cs, &sheet->rules[i], sheet);

    for (int i = 0; i < doc->element_count; i++) if (doc->elements[i]) FCS_applyToElement(sheet, doc->elements[i]);

    for (int i = 0; i < doc->element_count; i++)
    {
        FOUR_Element* elem = doc->elements[i];
        if (!elem) continue;
        if (!elem->parent) inherit_from_cs(&root_cs, elem);
        else if (elem->parent->computed_style) inherit_from_cs(elem->parent->computed_style, elem);
    }

    free(root_cs.font_family);
    if (root_cs.bg_gradient) { free(root_cs.bg_gradient->stops); free(root_cs.bg_gradient); }
}

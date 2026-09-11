#ifndef UI_CORE_H
#define UI_CORE_H



// ==============================================================================
// UI Core: Backend-Agnostic GUI Abstraction Layer
// ==============================================================================
// This header defines the application-facing UI interface.
// Third-party GUI headers (Raylib, Raygui, ImGui, etc.) MUST NEVER be included here.
// To switch GUI backends, ONLY ui_core.cpp needs to be replaced.
// ==============================================================================

//
//~ Helper Functions
//

// Construct a rectangle from (x, y, width, height)
static inline Rng2F32
ui_rect(f32 x, f32 y, f32 width, f32 height)
{
    Rng2F32 r;
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + width;
    r.y1 = y + height;
    return r;
}

// Construct a color vector (r, g, b, a in [0.0, 1.0])
static inline Vec4F32
ui_color(f32 r, f32 g, f32 b, f32 a)
{
    Vec4F32 c;
    c.x = r;
    c.y = g;
    c.z = b;
    c.w = a;
    return c;
}

// Predefined colors
#define UI_COLOR_WHITE        ui_color(1.0f, 1.0f, 1.0f, 1.0f)
#define UI_COLOR_BLACK        ui_color(0.0f, 0.0f, 0.0f, 1.0f)
#define UI_COLOR_BG           ui_color(0.12f, 0.12f, 0.14f, 1.0f)
#define UI_COLOR_PANEL_BG     ui_color(0.18f, 0.18f, 0.22f, 1.0f)
#define UI_COLOR_BAR_BG       ui_color(0.14f, 0.15f, 0.18f, 1.0f)
#define UI_COLOR_BORDER       ui_color(0.25f, 0.27f, 0.32f, 1.0f)
#define UI_COLOR_CARD_BG      ui_color(0.16f, 0.17f, 0.20f, 1.0f)
#define UI_COLOR_ACCENT       ui_color(0.20f, 0.55f, 0.90f, 1.0f)
#define UI_COLOR_SUCCESS      ui_color(0.20f, 0.75f, 0.35f, 1.0f)
#define UI_COLOR_DANGER       ui_color(0.85f, 0.25f, 0.20f, 1.0f)
#define UI_COLOR_WARNING      ui_color(0.95f, 0.70f, 0.15f, 1.0f)
#define UI_COLOR_TEXT_MUTED   ui_color(0.60f, 0.63f, 0.68f, 1.0f)
#define UI_COLOR_ACTIVE_ITEM  ui_color(0.22f, 0.35f, 0.55f, 1.0f)
#define UI_COLOR_LED_OFF      ui_color(0.25f, 0.27f, 0.30f, 1.0f)
#define UI_COLOR_LED_ON       ui_color(0.20f, 0.85f, 0.40f, 1.0f)
#define UI_COLOR_LED_ALERT    ui_color(0.90f, 0.25f, 0.20f, 1.0f)

//
//~ Window & Application Lifecycle
//
void ui_init(int width, int height, const char *title);
b32  ui_window_should_close(void);
void ui_close(void);
void ui_set_target_fps(int fps);
f32  ui_get_frame_time(void);
Vec2F32 ui_get_window_size(void);

//
//~ Frame Lifecycle
//
void ui_begin_frame(void);
void ui_end_frame(void);
void ui_clear_background(Vec4F32 color);

//
//~ Immediate-Mode GUI Controls (Backend-Agnostic)
//
b32  ui_button(Rng2F32 rect, const char *text);
void ui_label(Rng2F32 rect, const char *text);
b32  ui_check_box(Rng2F32 rect, const char *text, b32 *checked);
b32  ui_slider(Rng2F32 rect, const char *text_left, const char *text_right, f32 *value, f32 min_val, f32 max_val);
b32  ui_window_box(Rng2F32 rect, const char *title);
void ui_group_box(Rng2F32 rect, const char *text);
void ui_panel(Rng2F32 rect, const char *text);
void ui_status_bar(Rng2F32 rect, const char *text);
void ui_progress_bar(Rng2F32 rect, const char *text_left, const char *text_right, f32 *value, f32 min_val, f32 max_val);
b32  ui_spinner(Rng2F32 rect, const char *text, int *value, int min_val, int max_val, b32 edit_mode);

//
//~ Basic 2D Drawing Primitives
//
void ui_draw_rect(Rng2F32 rect, Vec4F32 color);
void ui_draw_rect_outline(Rng2F32 rect, f32 thickness, Vec4F32 color);
void ui_draw_circle(f32 center_x, f32 center_y, f32 radius, Vec4F32 color);
void ui_draw_text(const char *text, f32 x, f32 y, f32 font_size, Vec4F32 color);
void ui_draw_fps(int x, int y);

//
//~ String8 Overloads (Codebase style helpers)
//
static inline b32  ui_button_s8(Rng2F32 rect, String8 s)   { return ui_button(rect, (char *)s.str); }
static inline void ui_label_s8(Rng2F32 rect, String8 s)    { ui_label(rect, (char *)s.str); }
static inline b32  ui_window_box_s8(Rng2F32 rect, String8 s){ return ui_window_box(rect, (char *)s.str); }

#endif // UI_CORE_H



// ==============================================================================
// UI Core Backend: Raylib & Raylib-GUI Implementation
// ==============================================================================
// This file is the ONLY place in the entire codebase where Raylib and Raygui
// are included. All Raylib types (Rectangle, Color, etc.) are converted to
// and from our project's native types (Rng2F32, Vec4F32, etc.).
//
// If you ever want to replace Raylib with ImGui, Nuklear, or a custom renderer,
// you ONLY rewrite this file.

// ==============================================================================
// Inherits Raylib and Raygui definitions from machine_maker_main.cpp


//
//~ Internal Type Converters
//


static inline Rectangle
ui__to_raylib_rec(Rng2F32 r)
{
    Rectangle out;
    out.x = r.x0;
    out.y = r.y0;
    out.width = r.x1 - r.x0;
    out.height = r.y1 - r.y0;
    return out;
}



static inline Color
ui__to_raylib_color(Vec4F32 c)
{
    Color out;
    out.r = (unsigned char)(c.x * 255.0f);
    out.g = (unsigned char)(c.y * 255.0f);
    out.b = (unsigned char)(c.z * 255.0f);
    out.a = (unsigned char)(c.w * 255.0f);
    return out;
}



//
//~ Window & Application Lifecycle
//

void ui_init(int width, int height, const char *title)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(60);
}


b32 ui_window_should_close(void)
{
    return (b32)WindowShouldClose();
}

void ui_close(void)
{
    CloseWindow();
}

void ui_set_target_fps(int fps)
{
    SetTargetFPS(fps);
}


f32 ui_get_frame_time(void)
{
    return GetFrameTime();
}


Vec2F32 ui_get_window_size(void)
{
    Vec2F32 sz;
    sz.x = (f32)GetScreenWidth();
    sz.y = (f32)GetScreenHeight();
    return sz;
}

//
//~ Frame Lifecycle
//

void ui_begin_frame(void)
{
    BeginDrawing();
}

void ui_end_frame(void)
{
    EndDrawing();
}

void ui_clear_background(Vec4F32 color)
{
    ClearBackground(ui__to_raylib_color(color));
}

//
//~ Immediate-Mode Controls
//

b32 ui_button(Rng2F32 rect, const char *text)
{
    return (b32)(GuiButton(ui__to_raylib_rec(rect), text) != 0);
}

void ui_label(Rng2F32 rect, const char *text)
{
    GuiLabel(ui__to_raylib_rec(rect), text);
}

b32 ui_check_box(Rng2F32 rect, const char *text, b32 *checked)
{
    bool b = checked ? (*checked != 0) : false;
    int res = GuiCheckBox(ui__to_raylib_rec(rect), text, &b);
    if (checked)
    {
        *checked = (b32)b;
    }
    return (b32)(res != 0);
}

b32 ui_slider(Rng2F32 rect, const char *text_left, const char *text_right, f32 *value, f32 min_val, f32 max_val)
{
    return (b32)(GuiSlider(ui__to_raylib_rec(rect), text_left, text_right, value, min_val, max_val) != 0);
}

b32 ui_window_box(Rng2F32 rect, const char *title)
{
    return (b32)(GuiWindowBox(ui__to_raylib_rec(rect), title) != 0);
}

void ui_group_box(Rng2F32 rect, const char *text)
{
    GuiGroupBox(ui__to_raylib_rec(rect), text);
}

void ui_panel(Rng2F32 rect, const char *text)
{
    GuiPanel(ui__to_raylib_rec(rect), text);
}

void ui_status_bar(Rng2F32 rect, const char *text)
{
    GuiStatusBar(ui__to_raylib_rec(rect), text);
}

void ui_progress_bar(Rng2F32 rect, const char *text_left, const char *text_right, f32 *value, f32 min_val, f32 max_val)
{
    GuiProgressBar(ui__to_raylib_rec(rect), text_left, text_right, value, min_val, max_val);
}

b32 ui_spinner(Rng2F32 rect, const char *text, int *value, int min_val, int max_val, b32 edit_mode)
{
    return (b32)(GuiSpinner(ui__to_raylib_rec(rect), text, value, min_val, max_val, edit_mode != 0) != 0);
}

//
//~ Basic 2D Drawing Primitives
//

void ui_draw_rect(Rng2F32 rect, Vec4F32 color)
{
    DrawRectangleRec(ui__to_raylib_rec(rect), ui__to_raylib_color(color));
}

void ui_draw_rect_outline(Rng2F32 rect, f32 thickness, Vec4F32 color)
{
    DrawRectangleLinesEx(ui__to_raylib_rec(rect), thickness, ui__to_raylib_color(color));
}

void ui_draw_circle(f32 center_x, f32 center_y, f32 radius, Vec4F32 color)
{
    DrawCircle((int)center_x, (int)center_y, radius, ui__to_raylib_color(color));
}

void ui_draw_text(const char *text, f32 x, f32 y, f32 font_size, Vec4F32 color)
{
    DrawText(text, (int)x, (int)y, (int)font_size, ui__to_raylib_color(color));
}

void ui_draw_fps(int x, int y)
{
    DrawFPS(x, y);
}

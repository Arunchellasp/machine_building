

// ==============================================================================
// ui_core.cpp - UI Core Backend: GLFW & Nuklear (OpenGL 2) Implementation
// ==============================================================================
// This file implements the backend-agnostic UI abstractions defined in ui_core.h
// using GLFW 3.4 for windowing and Nuklear with OpenGL 2 for immediate-mode GUI.
//
// Layering:
//   - Application Level (mm_ui.cpp): interacts ONLY with ui_core.h.
//   - Core UI (this file): translates ui_* calls to GLFW and Nuklear.
//   - If the backend ever changes again, ONLY this file needs to be modified.
// ==============================================================================

//
//~ Backend State
//
static GLFWwindow          *g_ui_window     = NULL;
static struct nk_context   *g_nk_ctx        = NULL;
static struct nk_font_atlas *g_nk_atlas     = NULL;
static double               g_ui_last_time  = 0.0;
static f32                  g_ui_frame_time = 0.0166f;
static int                  g_ui_win_w      = 1280;
static int                  g_ui_win_h      = 720;
static int                  g_ui_fb_w       = 1280;
static int                  g_ui_fb_h       = 720;

//
//~ Internal Type Converters
//

static inline struct nk_color
ui__to_nk_color(Vec4F32 c)
{
    struct nk_color out;
    out.r = (nk_byte)(c.x * 255.0f);
    out.g = (nk_byte)(c.y * 255.0f);
    out.b = (nk_byte)(c.z * 255.0f);
    out.a = (nk_byte)(c.w * 255.0f);
    return out;
}

static inline struct nk_rect
ui__to_nk_rect(Rng2F32 r)
{
    return nk_rect(r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0);
}

//
//~ Sleek Industrial Dark Theme for Nuklear
//
static void
ui__apply_dark_theme(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT]                    = nk_rgba(230, 235, 245, 255);
    table[NK_COLOR_WINDOW]                  = nk_rgba(32, 34, 40, 255);
    table[NK_COLOR_HEADER]                  = nk_rgba(42, 46, 56, 255);
    table[NK_COLOR_BORDER]                  = nk_rgba(65, 70, 82, 255);
    table[NK_COLOR_BUTTON]                  = nk_rgba(45, 50, 62, 255);
    table[NK_COLOR_BUTTON_HOVER]            = nk_rgba(58, 65, 82, 255);
    table[NK_COLOR_BUTTON_ACTIVE]           = nk_rgba(35, 95, 175, 255);
    table[NK_COLOR_TOGGLE]                  = nk_rgba(45, 50, 62, 255);
    table[NK_COLOR_TOGGLE_HOVER]            = nk_rgba(58, 65, 82, 255);
    table[NK_COLOR_TOGGLE_CURSOR]           = nk_rgba(40, 160, 80, 255);
    table[NK_COLOR_SELECT]                  = nk_rgba(35, 95, 175, 255);
    table[NK_COLOR_SELECT_ACTIVE]           = nk_rgba(45, 120, 210, 255);
    table[NK_COLOR_SLIDER]                  = nk_rgba(38, 42, 52, 255);
    table[NK_COLOR_SLIDER_CURSOR]           = nk_rgba(50, 140, 230, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER]     = nk_rgba(70, 160, 250, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE]    = nk_rgba(90, 180, 255, 255);
    table[NK_COLOR_PROPERTY]                = nk_rgba(38, 42, 52, 255);
    table[NK_COLOR_EDIT]                    = nk_rgba(38, 42, 52, 255);
    table[NK_COLOR_EDIT_CURSOR]             = nk_rgba(230, 235, 245, 255);
    table[NK_COLOR_COMBO]                   = nk_rgba(45, 50, 62, 255);
    table[NK_COLOR_CHART]                   = nk_rgba(38, 42, 52, 255);
    table[NK_COLOR_CHART_COLOR]             = nk_rgba(50, 140, 230, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT]   = nk_rgba(235, 75, 60, 255);
    table[NK_COLOR_SCROLLBAR]               = nk_rgba(30, 32, 38, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR]        = nk_rgba(55, 60, 72, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]  = nk_rgba(70, 78, 92, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(90, 100, 118, 255);
    table[NK_COLOR_TAB_HEADER]              = nk_rgba(42, 46, 56, 255);
    nk_style_from_table(ctx, table);
}

//
//~ Window & Application Lifecycle
//

void ui_init(int width, int height, const char *title)
{
    if (!glfwInit())
    {
        return;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    g_ui_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!g_ui_window)
    {
        glfwTerminate();
        return;
    }
    
    glfwMakeContextCurrent(g_ui_window);
    glfwSwapInterval(1); // Enable VSync for smooth 60 FPS
    
    // Initialize Nuklear with GLFW3 callbacks
    g_nk_ctx = nk_glfw3_init(g_ui_window, NK_GLFW3_INSTALL_CALLBACKS);
    
    // Initialize font atlas with default font
    nk_glfw3_font_stash_begin(&g_nk_atlas);
    nk_glfw3_font_stash_end();
    
    // Apply industrial dark theme styling
    ui__apply_dark_theme(g_nk_ctx);
    
    g_ui_last_time = glfwGetTime();
}

b32 ui_window_should_close(void)
{
    return g_ui_window ? (b32)glfwWindowShouldClose(g_ui_window) : 1;
}

void ui_close(void)
{
    nk_glfw3_shutdown();
    if (g_ui_window)
    {
        glfwDestroyWindow(g_ui_window);
        g_ui_window = NULL;
    }
    glfwTerminate();
}

void ui_set_target_fps(int fps)
{
    // VSync handles refresh rate; interval 1 = standard monitor rate
    (void)fps;
}

f32 ui_get_frame_time(void)
{
    return g_ui_frame_time;
}

Vec2F32 ui_get_window_size(void)
{
    Vec2F32 sz = {0};
    if (g_ui_window)
    {
        int w = 0, h = 0;
        glfwGetWindowSize(g_ui_window, &w, &h);
        sz.x = (f32)w;
        sz.y = (f32)h;
    }
    return sz;
}

//
//~ Frame Lifecycle
//

void ui_begin_frame(void)
{
    double now = glfwGetTime();
    g_ui_frame_time = (f32)(now - g_ui_last_time);
    if (g_ui_frame_time <= 0.0001f || g_ui_frame_time > 0.5f) g_ui_frame_time = 0.0166f;
    g_ui_last_time = now;
    
    glfwPollEvents();
    nk_glfw3_new_frame();
    
    if (g_ui_window)
    {
        glfwGetWindowSize(g_ui_window, &g_ui_win_w, &g_ui_win_h);
        glfwGetFramebufferSize(g_ui_window, &g_ui_fb_w, &g_ui_fb_h);
    }
    
    // Configure zero-margin master canvas covering entire window
    g_nk_ctx->style.window.padding       = nk_vec2(0.0f, 0.0f);
    g_nk_ctx->style.window.group_padding = nk_vec2(0.0f, 0.0f);
    g_nk_ctx->style.window.spacing       = nk_vec2(0.0f, 0.0f);
    g_nk_ctx->style.window.border        = 0.0f;
    g_nk_ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    
    nk_flags flags = NK_WINDOW_NO_SCROLLBAR;
    if (nk_begin(g_nk_ctx, "MasterCanvas", nk_rect(0, 0, (float)g_ui_win_w, (float)g_ui_win_h), flags))
    {
        nk_layout_space_begin(g_nk_ctx, NK_STATIC, (float)g_ui_win_h, 8192);
    }
}

void ui_end_frame(void)
{
    nk_layout_space_end(g_nk_ctx);
    nk_end(g_nk_ctx);
    
    // Viewport & Nuklear Render
    glViewport(0, 0, g_ui_fb_w, g_ui_fb_h);
    nk_glfw3_render(NK_ANTI_ALIASING_ON);
    
    if (g_ui_window)
    {
        glfwSwapBuffers(g_ui_window);
    }
}

void ui_clear_background(Vec4F32 color)
{
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//
//~ Immediate-Mode Controls
//

b32 ui_button(Rng2F32 rect, const char *text)
{
    if (!g_nk_ctx || !text) return 0;
    nk_layout_space_push(g_nk_ctx, ui__to_nk_rect(rect));
    return (b32)(nk_button_label(g_nk_ctx, text) != 0);
}

void ui_label(Rng2F32 rect, const char *text)
{
    if (!g_nk_ctx || !text) return;
    nk_layout_space_push(g_nk_ctx, ui__to_nk_rect(rect));
    nk_label(g_nk_ctx, text, NK_TEXT_LEFT);
}

b32 ui_check_box(Rng2F32 rect, const char *text, b32 *checked)
{
    if (!g_nk_ctx || !text) return 0;
    nk_layout_space_push(g_nk_ctx, ui__to_nk_rect(rect));
    int val = checked ? (*checked != 0) : 0;
    int res = nk_checkbox_label(g_nk_ctx, text, &val);
    if (checked)
    {
        *checked = (b32)val;
    }
    return (b32)(res != 0);
}

b32 ui_slider(Rng2F32 rect, const char *text_left, const char *text_right, f32 *value, f32 min_val, f32 max_val)
{
    (void)text_left;
    (void)text_right;
    if (!g_nk_ctx || !value) return 0;
    nk_layout_space_push(g_nk_ctx, ui__to_nk_rect(rect));
    float step = (max_val - min_val) * 0.01f;
    if (step < 0.01f) step = 0.01f;
    return (b32)(nk_slider_float(g_nk_ctx, min_val, value, max_val, step) != 0);
}

b32 ui_window_box(Rng2F32 rect, const char *title)
{
    if (!g_nk_ctx) return 0;
    struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
    if (canvas)
    {
        struct nk_rect r = ui__to_nk_rect(rect);
        // Window background
        nk_fill_rect(canvas, r, 4.0f, nk_rgb(32, 34, 42));
        nk_stroke_rect(canvas, r, 4.0f, 1.0f, nk_rgb(60, 66, 80));
        
        // Title bar
        struct nk_rect hr = nk_rect(r.x, r.y, r.w, 26.0f);
        nk_fill_rect(canvas, hr, 4.0f, nk_rgb(42, 48, 62));
        if (title && g_nk_ctx->style.font)
        {
            struct nk_rect tr = nk_rect(r.x + 8.0f, r.y + 6.0f, r.w - 16.0f, 16.0f);
            nk_draw_text(canvas, tr, title, (int)strlen(title), g_nk_ctx->style.font,
                         nk_rgba(0,0,0,0), nk_rgb(220, 225, 235));
        }
    }
    return 0;
}

void ui_group_box(Rng2F32 rect, const char *text)
{
    if (!g_nk_ctx) return;
    struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
    if (canvas)
    {
        struct nk_rect r = ui__to_nk_rect(rect);
        nk_stroke_rect(canvas, r, 2.0f, 1.0f, nk_rgb(55, 62, 75));
        if (text && text[0] && g_nk_ctx->style.font)
        {
            f32 tw = (f32)(strlen(text) * 7.5f + 12.0f);
            struct nk_rect tr = nk_rect(r.x + 10.0f, r.y - 6.0f, tw, 13.0f);
            nk_fill_rect(canvas, tr, 0.0f, nk_rgb(30, 32, 38));
            nk_draw_text(canvas, tr, text, (int)strlen(text), g_nk_ctx->style.font,
                         nk_rgba(0,0,0,0), nk_rgb(160, 175, 195));
        }
    }
}

void ui_panel(Rng2F32 rect, const char *text)
{
    ui_draw_rect(rect, UI_COLOR_PANEL_BG);
    ui_draw_rect_outline(rect, 1.0f, UI_COLOR_BORDER);
    if (text && text[0])
    {
        ui_draw_text(text, rect.x0 + 8.0f, rect.y0 + 6.0f, 12.0f, UI_COLOR_TEXT_MUTED);
    }
}

void ui_status_bar(Rng2F32 rect, const char *text)
{
    ui_draw_rect(rect, UI_COLOR_BAR_BG);
    ui_draw_rect_outline(rect, 1.0f, UI_COLOR_BORDER);
    if (text && text[0])
    {
        ui_draw_text(text, rect.x0 + 8.0f, rect.y0 + 6.0f, 11.0f, UI_COLOR_WHITE);
    }
}

void ui_progress_bar(Rng2F32 rect, const char *text_left, const char *text_right, f32 *value, f32 min_val, f32 max_val)
{
    (void)text_left;
    (void)text_right;
    if (!g_nk_ctx || !value) return;
    nk_layout_space_push(g_nk_ctx, ui__to_nk_rect(rect));
    nk_size cur = (nk_size)(*value - min_val);
    nk_size max_n = (nk_size)(max_val - min_val);
    if (max_n == 0) max_n = 1;
    nk_progress(g_nk_ctx, &cur, max_n, nk_false);
}

b32 ui_spinner(Rng2F32 rect, const char *text, int *value, int min_val, int max_val, b32 edit_mode)
{
    (void)text;
    (void)edit_mode;
    if (!g_nk_ctx || !value) return 0;
    nk_layout_space_push(g_nk_ctx, ui__to_nk_rect(rect));
    int old_v = *value;
    nk_property_int(g_nk_ctx, "Value:", min_val, value, max_val, 1, 1.0f);
    return (b32)(*value != old_v);
}

//
//~ Basic 2D Drawing Primitives
//

void ui_draw_rect(Rng2F32 rect, Vec4F32 color)
{
    if (!g_nk_ctx) return;
    struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
    if (canvas)
    {
        nk_fill_rect(canvas, ui__to_nk_rect(rect), 0.0f, ui__to_nk_color(color));
    }
}

void ui_draw_rect_outline(Rng2F32 rect, f32 thickness, Vec4F32 color)
{
    if (!g_nk_ctx) return;
    struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
    if (canvas)
    {
        nk_stroke_rect(canvas, ui__to_nk_rect(rect), 0.0f, thickness, ui__to_nk_color(color));
    }
}

void ui_draw_circle(f32 center_x, f32 center_y, f32 radius, Vec4F32 color)
{
    if (!g_nk_ctx) return;
    struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
    if (canvas)
    {
        struct nk_rect r = nk_rect(center_x - radius, center_y - radius, radius * 2.0f, radius * 2.0f);
        nk_fill_circle(canvas, r, ui__to_nk_color(color));
    }
}

void ui_draw_text(const char *text, f32 x, f32 y, f32 font_size, Vec4F32 color)
{
    (void)font_size;
    if (!g_nk_ctx || !text || !text[0]) return;
    struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);
    if (canvas && g_nk_ctx->style.font)
    {
        struct nk_rect r = nk_rect(x, y, 1200.0f, 22.0f);
        nk_draw_text(canvas, r, text, (int)strlen(text), g_nk_ctx->style.font,
                     nk_rgba(0,0,0,0), ui__to_nk_color(color));
    }
}

void ui_draw_fps(int x, int y)
{
    char buf[32];
    int fps = (g_ui_frame_time > 0.0001f) ? (int)(1.0f / g_ui_frame_time + 0.5f) : 60;
    stbsp_snprintf(buf, sizeof(buf), "%d FPS", fps);
    ui_draw_text(buf, (f32)x, (f32)y, 12.0f, UI_COLOR_SUCCESS);
}

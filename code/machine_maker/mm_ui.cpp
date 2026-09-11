
// ==============================================================================
// mm_ui.cpp - Machine Maker UI: Modern Adaptable HMI & Windows Application Layout
// ==============================================================================
// Architecture Layering:
//   - Application Level (this file): orchestrates layout, state, and interaction.
//   - Core UI (ui_core.h): backend-agnostic GUI abstractions.
//   - Motion HAL (machine_core.h): pure C wrapper for GTS motion controllers.
//
// Layout Sections:
//   1. Top Bar: Project badge, Play/Stop/Resume controls, Action icons (Home, Config, Login)
//   2. Left Sidebar (File Bar / Tree): Categorized navigation (Overview, Motors, IO, Jog, Config, Diagnostics)
//   3. Center Main Layout: Dynamic view rendering based on active selection
//   4. Bottom Status Bar: Hardware connection, safety loop, operator, telemetry
//   5. Modals / Overlays: Centered dialogs for Login and Configuration
// ==============================================================================

// Global Application UI State
MM_UI_State mm_ui_state = {0};

//
//~ Helper: Event Logging
//
void mm_ui_add_log(const char *msg)
{
    if (!msg) return;
    
    // Shift existing messages down if full
    if (mm_ui_state.log_count < 8)
    {
        stbsp_snprintf(mm_ui_state.log_messages[mm_ui_state.log_count], 
                       sizeof(mm_ui_state.log_messages[0]), "%s", msg);
        mm_ui_state.log_count++;
    }
    else
    {
        for (int i = 0; i < 7; ++i)
        {
            stbsp_snprintf(mm_ui_state.log_messages[i], sizeof(mm_ui_state.log_messages[0]), 
                           "%s", mm_ui_state.log_messages[i + 1]);
        }
        stbsp_snprintf(mm_ui_state.log_messages[7], sizeof(mm_ui_state.log_messages[0]), "%s", msg);
    }
    
    // Also update current status message
    stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), "%s", msg);
}

//
//~ Lifecycle: Initialization
//
void mm_ui_init(void)
{
    mm_ui_state.initialized = 1;
    
    // Project & User Session
    stbsp_snprintf(mm_ui_state.project_name, sizeof(mm_ui_state.project_name), "CNC_MILL_PROTOTYPE_01");
    stbsp_snprintf(mm_ui_state.user_name, sizeof(mm_ui_state.user_name), "Admin (Operator)");
    mm_ui_state.logged_in = 1;
    
    // Navigation & Execution State
    mm_ui_state.active_tab = MM_NavTab_Overview;
    mm_ui_state.exec_state = MM_ExecState_Idle;
    
    // Hardware State (DO NOT initialize hardware at startup - safe offline mode!)
    mm_ui_state.connected = 0;
    mm_ui_state.servo_enabled = 0;
    mm_ui_state.emergency_stopped = 0;
    
    // Coordinates (Axes 1..4: X, Y, Z, A)
    mm_ui_state.pos_x = 0.0f;
    mm_ui_state.pos_y = 0.0f;
    mm_ui_state.pos_z = 0.0f;
    mm_ui_state.pos_a = 0.0f;
    
    mm_ui_state.target_x = 0.0f;
    mm_ui_state.target_y = 0.0f;
    mm_ui_state.target_z = 0.0f;
    mm_ui_state.target_a = 0.0f;
    
    mm_ui_state.vel_x = 0.0f;
    mm_ui_state.vel_y = 0.0f;
    mm_ui_state.vel_z = 0.0f;
    
    // Motion Parameters
    mm_ui_state.feed_rate = 800.0f;
    mm_ui_state.feed_override = 100.0f;
    mm_ui_state.spindle_rpm = 12000.0f;
    mm_ui_state.spindle_override = 100.0f;
    mm_ui_state.jog_step = 1.0f;
    mm_ui_state.jog_continuous = 0;
    
    // Digital I/O Defaults (Sensors nominal)
    mm_ui_state.gpi_bits = 0x000E; // Air pressure OK, door closed, coolant OK
    mm_ui_state.gpo_bits = 0x0000;
    
    // Modals
    mm_ui_state.show_login_modal = 0;
    mm_ui_state.show_config_modal = 0;
    
    // Event Logs
    mm_ui_state.log_count = 0;
    mm_ui_add_log("System initialized. Controller offline (simulation ready).");
}

//
//~ Simulation & Telemetry Tick
//
static void mm_ui__tick_simulation(f32 dt)
{
    // If connected to real hardware, query live positions and IO
    if (mm_ui_state.connected && mc_card_is_open())
    {
        mm_ui_state.pos_x = (f32)mc_axis_get_actual_pos(1);
        mm_ui_state.pos_y = (f32)mc_axis_get_actual_pos(2);
        mm_ui_state.pos_z = (f32)mc_axis_get_actual_pos(3);
        mm_ui_state.pos_a = (f32)mc_axis_get_actual_pos(4);
        
        mm_ui_state.vel_x = (f32)mc_axis_get_actual_vel(1);
        mm_ui_state.vel_y = (f32)mc_axis_get_actual_vel(2);
        mm_ui_state.vel_z = (f32)mc_axis_get_actual_vel(3);
        
        unsigned long gpi = 0;
        if (mc_io_get_gpi_all(&gpi) == MC_OK)
        {
            mm_ui_state.gpi_bits = (u16)gpi;
        }
        return;
    }
    
    // Offline Simulation: smoothly interpolate positions towards targets
    f32 speed = mm_ui_state.feed_rate * (mm_ui_state.feed_override / 100.0f) * (dt / 60.0f);
    if (speed < 0.001f) speed = 0.001f;
    
    // Interpolate X
    f32 dx = mm_ui_state.target_x - mm_ui_state.pos_x;
    if (fabsf(dx) > 0.001f)
    {
        f32 step = (dx > 0.0f ? 1.0f : -1.0f) * speed;
        if (fabsf(step) >= fabsf(dx)) mm_ui_state.pos_x = mm_ui_state.target_x;
        else mm_ui_state.pos_x += step;
        mm_ui_state.vel_x = (dx > 0.0f ? speed : -speed) / dt;
    }
    else
    {
        mm_ui_state.pos_x = mm_ui_state.target_x;
        mm_ui_state.vel_x = 0.0f;
    }
    
    // Interpolate Y
    f32 dy = mm_ui_state.target_y - mm_ui_state.pos_y;
    if (fabsf(dy) > 0.001f)
    {
        f32 step = (dy > 0.0f ? 1.0f : -1.0f) * speed;
        if (fabsf(step) >= fabsf(dy)) mm_ui_state.pos_y = mm_ui_state.target_y;
        else mm_ui_state.pos_y += step;
        mm_ui_state.vel_y = (dy > 0.0f ? speed : -speed) / dt;
    }
    else
    {
        mm_ui_state.pos_y = mm_ui_state.target_y;
        mm_ui_state.vel_y = 0.0f;
    }
    
    // Interpolate Z
    f32 dz = mm_ui_state.target_z - mm_ui_state.pos_z;
    if (fabsf(dz) > 0.001f)
    {
        f32 step = (dz > 0.0f ? 1.0f : -1.0f) * speed;
        if (fabsf(step) >= fabsf(dz)) mm_ui_state.pos_z = mm_ui_state.target_z;
        else mm_ui_state.pos_z += step;
        mm_ui_state.vel_z = (dz > 0.0f ? speed : -speed) / dt;
    }
    else
    {
        mm_ui_state.pos_z = mm_ui_state.target_z;
        mm_ui_state.vel_z = 0.0f;
    }
    
    // Interpolate A
    f32 da = mm_ui_state.target_a - mm_ui_state.pos_a;
    if (fabsf(da) > 0.001f)
    {
        f32 step = (da > 0.0f ? 1.0f : -1.0f) * speed;
        if (fabsf(step) >= fabsf(da)) mm_ui_state.pos_a = mm_ui_state.target_a;
        else mm_ui_state.pos_a += step;
    }
    else
    {
        mm_ui_state.pos_a = mm_ui_state.target_a;
    }
    
    // Simulation toolpath loop if running
    if (mm_ui_state.exec_state == MM_ExecState_Running && mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
    {
        if (fabsf(dx) <= 0.005f && fabsf(dy) <= 0.005f)
        {
            static int sim_waypoint = 0;
            sim_waypoint = (sim_waypoint + 1) % 4;
            switch (sim_waypoint)
            {
                case 0: mm_ui_state.target_x = 50.0f;  mm_ui_state.target_y = 0.0f;  break;
                case 1: mm_ui_state.target_x = 50.0f;  mm_ui_state.target_y = 50.0f; break;
                case 2: mm_ui_state.target_x = 0.0f;   mm_ui_state.target_y = 50.0f; break;
                case 3: mm_ui_state.target_x = 0.0f;   mm_ui_state.target_y = 0.0f;  break;
            }
        }
    }
}

//
//~ 1. Top Bar: Project Name, Play/Stop/Resume, Action Icons
//
static void mm_ui__render_top_bar(Vec2F32 win_sz, f32 top_bar_h)
{
    Rng2F32 bar_rect = ui_rect(0.0f, 0.0f, win_sz.x, top_bar_h);
    ui_draw_rect(bar_rect, UI_COLOR_BAR_BG);
    ui_draw_rect(ui_rect(0.0f, top_bar_h - 1.0f, win_sz.x, 1.0f), UI_COLOR_BORDER);
    
    //
    // Project Name Badge (Highlighted container)
    //
    Rng2F32 proj_badge = ui_rect(10.0f, 8.0f, 210.0f, 36.0f);
    ui_draw_rect(proj_badge, UI_COLOR_CARD_BG);
    ui_draw_rect_outline(proj_badge, 1.0f, UI_COLOR_ACCENT);
    
    char proj_txt[64];
    stbsp_snprintf(proj_txt, sizeof(proj_txt), "PROJECT: %s", mm_ui_state.project_name);
    ui_draw_text(proj_txt, 20.0f, 18.0f, 13.0f, UI_COLOR_WHITE);
    
    //
    // Execution Controls: [ ▶ PLAY ] [ ⏸ PAUSE / RESUME ] [ ⏹ STOP ]
    //
    f32 exec_x = 230.0f;
    f32 btn_h = 36.0f;
    
    // Play Button (Highlight green)
    Rng2F32 play_rect = ui_rect(exec_x, 8.0f, 95.0f, btn_h);
    if (mm_ui_state.exec_state == MM_ExecState_Running)
    {
        ui_draw_rect(play_rect, UI_COLOR_SUCCESS);
    }
    if (ui_button(play_rect, "> PLAY"))
    {
        if (mm_ui_state.emergency_stopped)
        {
            mm_ui_add_log("[WARN] Cannot run: Emergency Stop is active!");
        }
        else
        {
            mm_ui_state.exec_state = MM_ExecState_Running;
            if (!mm_ui_state.servo_enabled)
            {
                mm_ui_state.servo_enabled = 1;
                if (mm_ui_state.connected) { mc_axis_enable(1); mc_axis_enable(2); mc_axis_enable(3); }
            }
            mm_ui_add_log("Execution Started [PLAY].");
        }
    }
    
    // Pause / Resume Button
    exec_x += 102.0f;
    Rng2F32 pause_rect = ui_rect(exec_x, 8.0f, 105.0f, btn_h);
    const char *pause_label = (mm_ui_state.exec_state == MM_ExecState_Paused) ? "|| RESUME" : "|| PAUSE";
    if (mm_ui_state.exec_state == MM_ExecState_Paused)
    {
        ui_draw_rect(pause_rect, UI_COLOR_WARNING);
    }
    if (ui_button(pause_rect, pause_label))
    {
        if (mm_ui_state.exec_state == MM_ExecState_Running)
        {
            mm_ui_state.exec_state = MM_ExecState_Paused;
            mm_ui_add_log("Execution Paused [PAUSE].");
        }
        else if (mm_ui_state.exec_state == MM_ExecState_Paused)
        {
            mm_ui_state.exec_state = MM_ExecState_Running;
            mm_ui_add_log("Execution Resumed [RESUME].");
        }
    }
    
    // Stop Button (Highlight Red)
    exec_x += 112.0f;
    Rng2F32 stop_rect = ui_rect(exec_x, 8.0f, 95.0f, btn_h);
    if (mm_ui_state.exec_state == MM_ExecState_Stopped)
    {
        ui_draw_rect(stop_rect, UI_COLOR_DANGER);
    }
    if (ui_button(stop_rect, "[] STOP"))
    {
        mm_ui_state.exec_state = MM_ExecState_Stopped;
        mm_ui_state.target_x = mm_ui_state.pos_x;
        mm_ui_state.target_y = mm_ui_state.pos_y;
        mm_ui_state.target_z = mm_ui_state.pos_z;
        if (mm_ui_state.connected) mc_axes_stop_all(0);
        mm_ui_add_log("Execution Stopped [STOP].");
    }
    
    // State Pill Badge
    exec_x += 105.0f;
    Rng2F32 pill_rect = ui_rect(exec_x, 12.0f, 90.0f, 28.0f);
    Vec4F32 pill_color = UI_COLOR_CARD_BG;
    const char *pill_txt = "IDLE";
    switch (mm_ui_state.exec_state)
    {
        case MM_ExecState_Running: pill_color = UI_COLOR_SUCCESS; pill_txt = "RUNNING"; break;
        case MM_ExecState_Paused:  pill_color = UI_COLOR_WARNING; pill_txt = "PAUSED"; break;
        case MM_ExecState_Stopped: pill_color = UI_COLOR_DANGER;  pill_txt = "STOPPED"; break;
        case MM_ExecState_EStop:   pill_color = UI_COLOR_DANGER;  pill_txt = "E-STOP"; break;
        default:                   pill_color = UI_COLOR_BORDER;  pill_txt = "IDLE"; break;
    }
    ui_draw_rect(pill_rect, pill_color);
    ui_draw_rect_outline(pill_rect, 1.0f, UI_COLOR_WHITE);
    ui_draw_text(pill_txt, exec_x + 16.0f, 18.0f, 12.0f, UI_COLOR_WHITE);
    
    //
    // Right Action Icons (Extensible action buttons)
    //
    f32 right_x = win_sz.x - 10.0f;
    
    // 1. Operator / Login
    right_x -= 120.0f;
    Rng2F32 login_btn = ui_rect(right_x, 8.0f, 115.0f, btn_h);
    if (ui_button(login_btn, "[USER] Login"))
    {
        mm_ui_state.show_login_modal = !mm_ui_state.show_login_modal;
    }
    
    // 2. Machine Configuration Action Icon
    right_x -= 115.0f;
    Rng2F32 cfg_btn = ui_rect(right_x, 8.0f, 105.0f, btn_h);
    if (ui_button(cfg_btn, "[CFG] Config"))
    {
        mm_ui_state.active_tab = MM_NavTab_Config;
    }
    
    // 3. Home All Action Icon
    right_x -= 115.0f;
    Rng2F32 home_btn = ui_rect(right_x, 8.0f, 105.0f, btn_h);
    if (ui_button(home_btn, "[HOME] Origin"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_x = 0.0f;
            mm_ui_state.target_y = 0.0f;
            mm_ui_state.target_z = 0.0f;
            mm_ui_state.target_a = 0.0f;
            if (mm_ui_state.connected)
            {
                for (short a = 1; a <= 4; ++a) mc_axis_zero_position(a);
            }
            mm_ui_add_log("Homing sequence executed: All coordinates set to Origin.");
        }
        else
        {
            mm_ui_add_log("[WARN] Cannot home: Servos are disabled.");
        }
    }
    
    // 4. Connect GTS Hardware Action Icon
    right_x -= 135.0f;
    Rng2F32 conn_btn = ui_rect(right_x, 8.0f, 125.0f, btn_h);
    const char *conn_txt = mm_ui_state.connected ? "Disconnect GTS" : "Connect GTS";
    if (ui_button(conn_btn, conn_txt))
    {
        if (!mm_ui_state.connected)
        {
            // Only try opening GTS when button clicked explicitly
            MC_Result res = mc_init(0, "GTS800.cfg");
            if (res == MC_OK)
            {
                mm_ui_state.connected = 1;
                MC_CardInfo info;
                mc_card_get_info(&info);
                char buf[96];
                stbsp_snprintf(buf, sizeof(buf), "GTS800 Card %d Connected (DLL: %s).", info.card_no, info.dll_version);
                mm_ui_add_log(buf);
            }
            else
            {
                mm_ui_state.connected = 0;
                char buf[96];
                stbsp_snprintf(buf, sizeof(buf), "[OFFLINE] No GTS card detected (Code: %d). In Simulation.", (int)res);
                mm_ui_add_log(buf);
            }
        }
        else
        {
            mc_shutdown();
            mm_ui_state.connected = 0;
            mm_ui_state.servo_enabled = 0;
            mm_ui_add_log("GTS Motion Controller Disconnected.");
        }
    }
}

//
//~ 2. Left Sidebar: Subsystem Navigation Tree / File Bar
//
static void mm_ui__render_sidebar(Vec2F32 win_sz, f32 top_bar_h, f32 sidebar_w, f32 bottom_bar_h)
{
    f32 sidebar_h = win_sz.y - top_bar_h - bottom_bar_h;
    Rng2F32 side_rect = ui_rect(0.0f, top_bar_h, sidebar_w, sidebar_h);
    ui_draw_rect(side_rect, UI_COLOR_PANEL_BG);
    ui_draw_rect(ui_rect(sidebar_w - 1.0f, top_bar_h, 1.0f, sidebar_h), UI_COLOR_BORDER);
    
    f32 cur_y = top_bar_h + 12.0f;
    f32 item_h = 34.0f;
    f32 item_pad_x = 8.0f;
    f32 item_w = sidebar_w - (item_pad_x * 2.0f);
    
    // Group 1: MACHINE STATUS
    ui_draw_text("MACHINE STATUS", item_pad_x + 6.0f, cur_y + 4.0f, 11.0f, UI_COLOR_TEXT_MUTED);
    cur_y += 24.0f;
    
    struct NavItem {
        MM_NavTab tab;
        const char *name;
    };
    
    struct NavItem items_grp1[] = {
        { MM_NavTab_Overview, "  Overview / DRO" },
        { MM_NavTab_Motors,   "  Motor Status" },
        { MM_NavTab_IO,       "  Digital / Analog IO" },
    };
    
    for (int i = 0; i < 3; ++i)
    {
        Rng2F32 r = ui_rect(item_pad_x, cur_y, item_w, item_h);
        b32 is_active = (mm_ui_state.active_tab == items_grp1[i].tab);
        
        if (is_active)
        {
            ui_draw_rect(r, UI_COLOR_ACTIVE_ITEM);
            ui_draw_rect(ui_rect(0.0f, cur_y, 4.0f, item_h), UI_COLOR_ACCENT);
        }
        
        if (ui_button(r, items_grp1[i].name))
        {
            mm_ui_state.active_tab = items_grp1[i].tab;
        }
        cur_y += item_h + 4.0f;
    }
    
    // Group 2: MOTION CONTROL
    cur_y += 12.0f;
    ui_draw_text("MOTION CONTROL", item_pad_x + 6.0f, cur_y + 4.0f, 11.0f, UI_COLOR_TEXT_MUTED);
    cur_y += 24.0f;
    
    struct NavItem items_grp2[] = {
        { MM_NavTab_Jog, "  Manual Jog & Coord" },
    };
    
    for (int i = 0; i < 1; ++i)
    {
        Rng2F32 r = ui_rect(item_pad_x, cur_y, item_w, item_h);
        b32 is_active = (mm_ui_state.active_tab == items_grp2[i].tab);
        if (is_active)
        {
            ui_draw_rect(r, UI_COLOR_ACTIVE_ITEM);
            ui_draw_rect(ui_rect(0.0f, cur_y, 4.0f, item_h), UI_COLOR_ACCENT);
        }
        if (ui_button(r, items_grp2[i].name))
        {
            mm_ui_state.active_tab = items_grp2[i].tab;
        }
        cur_y += item_h + 4.0f;
    }
    
    // Group 3: SYSTEM SETUP
    cur_y += 12.0f;
    ui_draw_text("SYSTEM SETUP", item_pad_x + 6.0f, cur_y + 4.0f, 11.0f, UI_COLOR_TEXT_MUTED);
    cur_y += 24.0f;
    
    struct NavItem items_grp3[] = {
        { MM_NavTab_Config,      "  Machine Config" },
        { MM_NavTab_Diagnostics, "  Diagnostics & Logs" },
    };
    
    for (int i = 0; i < 2; ++i)
    {
        Rng2F32 r = ui_rect(item_pad_x, cur_y, item_w, item_h);
        b32 is_active = (mm_ui_state.active_tab == items_grp3[i].tab);
        if (is_active)
        {
            ui_draw_rect(r, UI_COLOR_ACTIVE_ITEM);
            ui_draw_rect(ui_rect(0.0f, cur_y, 4.0f, item_h), UI_COLOR_ACCENT);
        }
        if (ui_button(r, items_grp3[i].name))
        {
            mm_ui_state.active_tab = items_grp3[i].tab;
        }
        cur_y += item_h + 4.0f;
    }
}

//
//~ 3. Central Views: Sub-Layouts
//

// View A: Overview / Digital Readout (DRO)
static void mm_ui__render_tab_overview(Rng2F32 content_rect)
{
    f32 pad = 16.0f;
    f32 cur_x = content_rect.x0 + pad;
    f32 cur_y = content_rect.y0 + pad;
    f32 total_w = (content_rect.x1 - content_rect.x0) - (pad * 2.0f);
    
    // Header
    ui_draw_text("LIVE MACHINE DASHBOARD & DIGITAL READOUT (DRO)", cur_x, cur_y, 16.0f, UI_COLOR_WHITE);
    cur_y += 30.0f;
    
    // 4 Coordinate Display Cards (X, Y, Z, A)
    f32 dro_w = (total_w - 45.0f) / 4.0f;
    f32 dro_h = 130.0f;
    
    const char *axis_names[4] = { "AXIS X (Table)", "AXIS Y (Saddle)", "AXIS Z (Head)", "AXIS A (Rotary)" };
    f32 axis_pos[4] = { mm_ui_state.pos_x, mm_ui_state.pos_y, mm_ui_state.pos_z, mm_ui_state.pos_a };
    f32 axis_vel[4] = { mm_ui_state.vel_x, mm_ui_state.vel_y, mm_ui_state.vel_z, 0.0f };
    const char *units[4] = { "mm", "mm", "mm", "deg" };
    
    for (int i = 0; i < 4; ++i)
    {
        f32 card_x = cur_x + i * (dro_w + 15.0f);
        Rng2F32 card_rect = ui_rect(card_x, cur_y, dro_w, dro_h);
        ui_draw_rect(card_rect, UI_COLOR_CARD_BG);
        ui_draw_rect_outline(card_rect, 1.0f, UI_COLOR_BORDER);
        
        // Header
        ui_draw_text(axis_names[i], card_x + 12.0f, cur_y + 12.0f, 13.0f, UI_COLOR_ACCENT);
        
        // Position readout (Large display)
        char pos_str[32];
        stbsp_snprintf(pos_str, sizeof(pos_str), "%+08.3f %s", axis_pos[i], units[i]);
        ui_draw_text(pos_str, card_x + 12.0f, cur_y + 40.0f, 22.0f, UI_COLOR_WHITE);
        
        // Velocity
        char vel_str[32];
        stbsp_snprintf(vel_str, sizeof(vel_str), "Vel: %0.1f mm/s", axis_vel[i]);
        ui_draw_text(vel_str, card_x + 12.0f, cur_y + 75.0f, 12.0f, UI_COLOR_TEXT_MUTED);
        
        // Zero Axis button
        Rng2F32 zero_btn = ui_rect(card_x + 12.0f, cur_y + 95.0f, dro_w - 24.0f, 24.0f);
        char btn_label[32];
        stbsp_snprintf(btn_label, sizeof(btn_label), "Zero %c", "XYZA"[i]);
        if (ui_button(zero_btn, btn_label))
        {
            switch (i)
            {
                case 0: mm_ui_state.pos_x = mm_ui_state.target_x = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(1); break;
                case 1: mm_ui_state.pos_y = mm_ui_state.target_y = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(2); break;
                case 2: mm_ui_state.pos_z = mm_ui_state.target_z = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(3); break;
                case 3: mm_ui_state.pos_a = mm_ui_state.target_a = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(4); break;
            }
        }
    }
    cur_y += dro_h + 20.0f;
    
    // Middle Section: Motion Control Group & Machine Status Panel
    f32 mid_w = (total_w - 15.0f) * 0.5f;
    f32 mid_h = 190.0f;
    
    // Left Box: Motion Dynamics Sliders
    Rng2F32 mot_box = ui_rect(cur_x, cur_y, mid_w, mid_h);
    ui_group_box(mot_box, "Motion Parameters & Feed Rate Override");
    
    char feed_label[64];
    stbsp_snprintf(feed_label, sizeof(feed_label), "Feed Rate: %0.0f mm/min (Override: %0.0f%%)", 
                   mm_ui_state.feed_rate, mm_ui_state.feed_override);
    ui_label(ui_rect(cur_x + 15.0f, cur_y + 25.0f, mid_w - 30.0f, 20.0f), feed_label);
    ui_slider(ui_rect(cur_x + 15.0f, cur_y + 50.0f, mid_w - 30.0f, 22.0f), "0%", "200%", &mm_ui_state.feed_override, 0.0f, 200.0f);
    
    char spd_label[64];
    stbsp_snprintf(spd_label, sizeof(spd_label), "Spindle: %0.0f RPM (Override: %0.0f%%)", 
                   mm_ui_state.spindle_rpm, mm_ui_state.spindle_override);
    ui_label(ui_rect(cur_x + 15.0f, cur_y + 90.0f, mid_w - 30.0f, 20.0f), spd_label);
    ui_slider(ui_rect(cur_x + 15.0f, cur_y + 115.0f, mid_w - 30.0f, 22.0f), "0%", "200%", &mm_ui_state.spindle_override, 0.0f, 200.0f);
    
    // Right Box: Safety & State Overview
    Rng2F32 st_box = ui_rect(cur_x + mid_w + 15.0f, cur_y, mid_w, mid_h);
    ui_group_box(st_box, "Safety Loop & Drive Power");
    
    // Emergency Stop Button
    Rng2F32 estop_rect = ui_rect(cur_x + mid_w + 30.0f, cur_y + 30.0f, mid_w - 60.0f, 40.0f);
    if (mm_ui_state.emergency_stopped)
    {
        ui_draw_rect(estop_rect, UI_COLOR_DANGER);
    }
    const char *estop_txt = mm_ui_state.emergency_stopped ? "CLEAR EMERGENCY STOP" : "! EMERGENCY STOP !";
    if (ui_button(estop_rect, estop_txt))
    {
        mm_ui_state.emergency_stopped = !mm_ui_state.emergency_stopped;
        if (mm_ui_state.emergency_stopped)
        {
            mm_ui_state.exec_state = MM_ExecState_EStop;
            mm_ui_state.servo_enabled = 0;
            if (mm_ui_state.connected) mc_axes_stop_all(1);
            mm_ui_add_log("EMERGENCY STOP ENGAGED! All axes halted.");
        }
        else
        {
            mm_ui_state.exec_state = MM_ExecState_Idle;
            mm_ui_add_log("Emergency Stop Cleared. Ready to enable drives.");
        }
    }
    
    // Servo Energize Toggle
    Rng2F32 srv_rect = ui_rect(cur_x + mid_w + 30.0f, cur_y + 85.0f, mid_w - 60.0f, 35.0f);
    const char *srv_txt = mm_ui_state.servo_enabled ? "Disable Drives (De-energize)" : "Enable Drives (Energize Servos)";
    if (ui_button(srv_rect, srv_txt))
    {
        if (!mm_ui_state.emergency_stopped)
        {
            mm_ui_state.servo_enabled = !mm_ui_state.servo_enabled;
            if (mm_ui_state.connected)
            {
                for (short a = 1; a <= 4; ++a)
                {
                    if (mm_ui_state.servo_enabled) mc_axis_enable(a);
                    else mc_axis_disable(a);
                }
            }
            mm_ui_add_log(mm_ui_state.servo_enabled ? "Servo Drives Energized." : "Servo Drives Disabled.");
        }
        else
        {
            mm_ui_add_log("[WARN] Cannot enable servos while E-Stop is active!");
        }
    }
    
    // Bottom Quick Command Row
    cur_y += mid_h + 15.0f;
    Rng2F32 quick_box = ui_rect(cur_x, cur_y, total_w, 75.0f);
    ui_group_box(quick_box, "Quick Actions & Macro Controls");
    
    f32 q_btn_w = (total_w - 45.0f) / 4.0f;
    if (ui_button(ui_rect(cur_x + 15.0f, cur_y + 25.0f, q_btn_w, 35.0f), "Zero All Coordinates"))
    {
        mm_ui_state.pos_x = mm_ui_state.target_x = 0.0f;
        mm_ui_state.pos_y = mm_ui_state.target_y = 0.0f;
        mm_ui_state.pos_z = mm_ui_state.target_z = 0.0f;
        mm_ui_state.pos_a = mm_ui_state.target_a = 0.0f;
        if (mm_ui_state.connected) for (short a = 1; a <= 4; ++a) mc_axis_zero_position(a);
        mm_ui_add_log("All axes zeroed.");
    }
    if (ui_button(ui_rect(cur_x + 15.0f + q_btn_w + 10.0f, cur_y + 25.0f, q_btn_w, 35.0f), "Rapid To Origin (0,0,0)"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_x = 0.0f;
            mm_ui_state.target_y = 0.0f;
            mm_ui_state.target_z = 0.0f;
            mm_ui_add_log("Rapid motion to origin initiated.");
        }
    }
    if (ui_button(ui_rect(cur_x + 15.0f + (q_btn_w + 10.0f) * 2.0f, cur_y + 25.0f, q_btn_w, 35.0f), "Spindle CW On/Off"))
    {
        mm_ui_state.gpo_bits ^= (1 << 0);
        if (mm_ui_state.connected) mc_io_set_gpo_bit(0, (mm_ui_state.gpo_bits & 1) ? 1 : 0);
        mm_ui_add_log((mm_ui_state.gpo_bits & 1) ? "Spindle Motor Started." : "Spindle Motor Stopped.");
    }
    if (ui_button(ui_rect(cur_x + 15.0f + (q_btn_w + 10.0f) * 3.0f, cur_y + 25.0f, q_btn_w, 35.0f), "Coolant Flood On/Off"))
    {
        mm_ui_state.gpo_bits ^= (1 << 2);
        if (mm_ui_state.connected) mc_io_set_gpo_bit(2, (mm_ui_state.gpo_bits & (1 << 2)) ? 1 : 0);
        mm_ui_add_log((mm_ui_state.gpo_bits & (1 << 2)) ? "Coolant Flood Enabled." : "Coolant Flood Disabled.");
    }
}

// View B: Detailed Motor & Axis Status Table
static void mm_ui__render_tab_motors(Rng2F32 content_rect)
{
    f32 pad = 16.0f;
    f32 cur_x = content_rect.x0 + pad;
    f32 cur_y = content_rect.y0 + pad;
    f32 total_w = (content_rect.x1 - content_rect.x0) - (pad * 2.0f);
    
    ui_draw_text("DETAILED MOTOR & AXIS SERVO STATUS", cur_x, cur_y, 16.0f, UI_COLOR_WHITE);
    cur_y += 30.0f;
    
    // Table Box
    Rng2F32 tbl_rect = ui_rect(cur_x, cur_y, total_w, 320.0f);
    ui_group_box(tbl_rect, "Axis Feedback & Drive Health");
    
    // Header row
    f32 row_y = cur_y + 25.0f;
    f32 col_axis   = cur_x + 15.0f;
    f32 col_act    = cur_x + 110.0f;
    f32 col_tgt    = cur_x + 220.0f;
    f32 col_err    = cur_x + 330.0f;
    f32 col_vel    = cur_x + 430.0f;
    f32 col_srv    = cur_x + 530.0f;
    f32 col_lim_p  = cur_x + 630.0f;
    f32 col_lim_n  = cur_x + 710.0f;
    f32 col_home   = cur_x + 790.0f;
    f32 col_alm    = cur_x + 860.0f;
    f32 col_act_b  = cur_x + 940.0f;
    
    ui_draw_text("AXIS",        col_axis,  row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("ACTUAL POS",  col_act,   row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("TARGET POS",  col_tgt,   row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("DIFF (ERR)",  col_err,   row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("VELOCITY",    col_vel,   row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("SERVO",       col_srv,   row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("LIMIT +",     col_lim_p, row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("LIMIT -",     col_lim_n, row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("HOME",        col_home,  row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("ALARM",       col_alm,   row_y, 12.0f, UI_COLOR_ACCENT);
    ui_draw_text("COMMANDS",    col_act_b, row_y, 12.0f, UI_COLOR_ACCENT);
    
    row_y += 18.0f;
    ui_draw_rect(ui_rect(cur_x + 10.0f, row_y, total_w - 20.0f, 1.0f), UI_COLOR_BORDER);
    row_y += 10.0f;
    
    const char *names[4] = { "Axis 1 (X)", "Axis 2 (Y)", "Axis 3 (Z)", "Axis 4 (A)" };
    f32 acts[4] = { mm_ui_state.pos_x, mm_ui_state.pos_y, mm_ui_state.pos_z, mm_ui_state.pos_a };
    f32 tgts[4] = { mm_ui_state.target_x, mm_ui_state.target_y, mm_ui_state.target_z, mm_ui_state.target_a };
    f32 vels[4] = { mm_ui_state.vel_x, mm_ui_state.vel_y, mm_ui_state.vel_z, 0.0f };
    
    for (int i = 0; i < 4; ++i)
    {
        char buf[32];
        
        // Axis label
        ui_draw_text(names[i], col_axis, row_y + 6.0f, 12.0f, UI_COLOR_WHITE);
        
        // Actual Pos
        stbsp_snprintf(buf, sizeof(buf), "%+8.3f", acts[i]);
        ui_draw_text(buf, col_act, row_y + 6.0f, 12.0f, UI_COLOR_WHITE);
        
        // Target Pos
        stbsp_snprintf(buf, sizeof(buf), "%+8.3f", tgts[i]);
        ui_draw_text(buf, col_tgt, row_y + 6.0f, 12.0f, UI_COLOR_TEXT_MUTED);
        
        // Error
        stbsp_snprintf(buf, sizeof(buf), "%+6.3f", acts[i] - tgts[i]);
        ui_draw_text(buf, col_err, row_y + 6.0f, 12.0f, UI_COLOR_TEXT_MUTED);
        
        // Velocity
        stbsp_snprintf(buf, sizeof(buf), "%5.1f", vels[i]);
        ui_draw_text(buf, col_vel, row_y + 6.0f, 12.0f, UI_COLOR_WHITE);
        
        // Servo pill
        Vec4F32 srv_col = mm_ui_state.servo_enabled ? UI_COLOR_SUCCESS : UI_COLOR_BORDER;
        ui_draw_rect(ui_rect(col_srv, row_y + 3.0f, 50.0f, 20.0f), srv_col);
        ui_draw_text(mm_ui_state.servo_enabled ? "ON" : "OFF", col_srv + 14.0f, row_y + 6.0f, 11.0f, UI_COLOR_WHITE);
        
        // Sensor LEDs
        ui_draw_circle(col_lim_p + 15.0f, row_y + 12.0f, 6.0f, mm_ui_state.limit_pos[i] ? UI_COLOR_LED_ALERT : UI_COLOR_LED_OFF);
        ui_draw_circle(col_lim_n + 15.0f, row_y + 12.0f, 6.0f, mm_ui_state.limit_neg[i] ? UI_COLOR_LED_ALERT : UI_COLOR_LED_OFF);
        ui_draw_circle(col_home  + 12.0f, row_y + 12.0f, 6.0f, mm_ui_state.home_sensor[i] ? UI_COLOR_WARNING : UI_COLOR_LED_OFF);
        ui_draw_circle(col_alm   + 15.0f, row_y + 12.0f, 6.0f, mm_ui_state.drive_alarm[i] ? UI_COLOR_LED_ALERT : UI_COLOR_LED_OFF);
        
        // Zero & Clear Buttons
        if (ui_button(ui_rect(col_act_b, row_y + 2.0f, 55.0f, 22.0f), "Zero"))
        {
            switch (i)
            {
                case 0: mm_ui_state.pos_x = mm_ui_state.target_x = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(1); break;
                case 1: mm_ui_state.pos_y = mm_ui_state.target_y = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(2); break;
                case 2: mm_ui_state.pos_z = mm_ui_state.target_z = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(3); break;
                case 3: mm_ui_state.pos_a = mm_ui_state.target_a = 0.0f; if (mm_ui_state.connected) mc_axis_zero_position(4); break;
            }
        }
        if (ui_button(ui_rect(col_act_b + 60.0f, row_y + 2.0f, 55.0f, 22.0f), "Clear"))
        {
            mm_ui_state.drive_alarm[i] = 0;
            if (mm_ui_state.connected) mc_axis_clear_status((short)(i + 1));
        }
        
        row_y += 32.0f;
        ui_draw_rect(ui_rect(cur_x + 10.0f, row_y, total_w - 20.0f, 1.0f), UI_COLOR_CARD_BG);
        row_y += 8.0f;
    }
    
    // Batch controls at bottom
    cur_y += 335.0f;
    if (ui_button(ui_rect(cur_x, cur_y, 160.0f, 35.0f), "Enable All Drives"))
    {
        mm_ui_state.servo_enabled = 1;
        if (mm_ui_state.connected) for (short a = 1; a <= 4; ++a) mc_axis_enable(a);
        mm_ui_add_log("Command: All servo drives enabled.");
    }
    if (ui_button(ui_rect(cur_x + 175.0f, cur_y, 160.0f, 35.0f), "Disable All Drives"))
    {
        mm_ui_state.servo_enabled = 0;
        if (mm_ui_state.connected) for (short a = 1; a <= 4; ++a) mc_axis_disable(a);
        mm_ui_add_log("Command: All servo drives disabled.");
    }
    if (ui_button(ui_rect(cur_x + 350.0f, cur_y, 160.0f, 35.0f), "Reset Drive Alarms"))
    {
        for (int i = 0; i < 4; ++i) mm_ui_state.drive_alarm[i] = 0;
        if (mm_ui_state.connected) for (short a = 1; a <= 4; ++a) mc_axis_clear_status(a);
        mm_ui_add_log("All axis alarms cleared.");
    }
}

// View C: Digital & Analog I/O Matrix
static void mm_ui__render_tab_io(Rng2F32 content_rect)
{
    f32 pad = 16.0f;
    f32 cur_x = content_rect.x0 + pad;
    f32 cur_y = content_rect.y0 + pad;
    f32 total_w = (content_rect.x1 - content_rect.x0) - (pad * 2.0f);
    
    ui_draw_text("DIGITAL & ANALOG I/O SIGNAL STATUS", cur_x, cur_y, 16.0f, UI_COLOR_WHITE);
    cur_y += 30.0f;
    
    f32 col_w = (total_w - 20.0f) * 0.5f;
    f32 box_h = 440.0f;
    
    // Left Box: Digital Inputs (GPI 0 - 15)
    Rng2F32 in_box = ui_rect(cur_x, cur_y, col_w, box_h);
    ui_group_box(in_box, "Digital Inputs (GPI 0..15) & Sensors");
    
    const char *input_names[16] = {
        "E-Stop Button (NC)",    "Safety Door Interlock", "Air Pressure OK",       "Coolant Flow Sensor",
        "Limit Switch X+",       "Limit Switch X-",       "Limit Switch Y+",       "Limit Switch Y-",
        "Limit Switch Z+",       "Limit Switch Z-",       "Home Sensor X",         "Home Sensor Y",
        "Home Sensor Z",         "Spindle Drive Ready",   "Touch Tool Probe",      "Auxiliary Spare In"
    };
    
    f32 in_y = cur_y + 25.0f;
    for (int i = 0; i < 16; ++i)
    {
        b32 bit_val = (mm_ui_state.gpi_bits >> i) & 1;
        
        // LED indicator
        ui_draw_circle(cur_x + 22.0f, in_y + 9.0f, 6.0f, bit_val ? UI_COLOR_LED_ON : UI_COLOR_LED_OFF);
        
        // Text
        char bit_txt[48];
        stbsp_snprintf(bit_txt, sizeof(bit_txt), "GPI #%02d: %s", i, input_names[i]);
        ui_draw_text(bit_txt, cur_x + 38.0f, in_y + 3.0f, 11.0f, bit_val ? UI_COLOR_WHITE : UI_COLOR_TEXT_MUTED);
        
        // Simulation Toggle Button
        Rng2F32 sim_btn = ui_rect(cur_x + col_w - 65.0f, in_y, 50.0f, 18.0f);
        if (ui_button(sim_btn, bit_val ? "HIGH" : "LOW"))
        {
            mm_ui_state.gpi_bits ^= (1 << i);
        }
        
        in_y += 24.0f;
    }
    
    // Right Box: Digital Outputs (GPO 0 - 15)
    f32 out_x = cur_x + col_w + 20.0f;
    Rng2F32 out_box = ui_rect(out_x, cur_y, col_w, box_h);
    ui_group_box(out_box, "Digital Outputs (GPO 0..15) & Actuators");
    
    const char *output_names[16] = {
        "Spindle Motor Run",     "Spindle Direction CW",  "Flood Coolant Pump",    "Mist Air Coolant",
        "Chamber Work Light",    "Chip Conveyor Drive",   "Dust Extraction Vac",   "Pneumatic Tool Clamp",
        "Pneumatic Fixture Lock","Vacuum Work Holding",   "Tower Light: Red",      "Tower Light: Yellow",
        "Tower Light: Green",    "Audible Warning Siren", "Auxiliary Valve 1",     "Auxiliary Valve 2"
    };
    
    f32 out_y = cur_y + 25.0f;
    for (int i = 0; i < 16; ++i)
    {
        b32 bit_val = (mm_ui_state.gpo_bits >> i) & 1;
        
        // LED indicator
        ui_draw_circle(out_x + 22.0f, out_y + 9.0f, 6.0f, bit_val ? UI_COLOR_ACCENT : UI_COLOR_LED_OFF);
        
        // Text
        char bit_txt[48];
        stbsp_snprintf(bit_txt, sizeof(bit_txt), "GPO #%02d: %s", i, output_names[i]);
        ui_draw_text(bit_txt, out_x + 38.0f, out_y + 3.0f, 11.0f, bit_val ? UI_COLOR_WHITE : UI_COLOR_TEXT_MUTED);
        
        // Interactive Toggle Button
        Rng2F32 act_btn = ui_rect(out_x + col_w - 75.0f, out_y, 60.0f, 18.0f);
        if (ui_button(act_btn, bit_val ? "[ON]" : "[OFF]"))
        {
            mm_ui_state.gpo_bits ^= (1 << i);
            if (mm_ui_state.connected)
            {
                mc_io_set_gpo_bit((short)i, (mm_ui_state.gpo_bits >> i) & 1);
            }
        }
        
        out_y += 24.0f;
    }
}

// View D: Manual Jogging & Coordinate Control
static void mm_ui__render_tab_jog(Rng2F32 content_rect)
{
    f32 pad = 16.0f;
    f32 cur_x = content_rect.x0 + pad;
    f32 cur_y = content_rect.y0 + pad;
    f32 total_w = (content_rect.x1 - content_rect.x0) - (pad * 2.0f);
    
    ui_draw_text("MANUAL JOGGING & VIRTUAL CNC PENDANT", cur_x, cur_y, 16.0f, UI_COLOR_WHITE);
    cur_y += 30.0f;
    
    // Step Size Selection Row
    Rng2F32 step_box = ui_rect(cur_x, cur_y, total_w, 65.0f);
    ui_group_box(step_box, "Jog Increment Step Size");
    
    f32 s_btn_w = 95.0f;
    f32 s_x = cur_x + 15.0f;
    f32 s_y = cur_y + 22.0f;
    
    if (ui_button(ui_rect(s_x, s_y, s_btn_w, 30.0f), "0.01 mm")) { mm_ui_state.jog_step = 0.01f; mm_ui_state.jog_continuous = 0; }
    s_x += s_btn_w + 10.0f;
    if (ui_button(ui_rect(s_x, s_y, s_btn_w, 30.0f), "0.10 mm")) { mm_ui_state.jog_step = 0.10f; mm_ui_state.jog_continuous = 0; }
    s_x += s_btn_w + 10.0f;
    if (ui_button(ui_rect(s_x, s_y, s_btn_w, 30.0f), "1.00 mm")) { mm_ui_state.jog_step = 1.00f; mm_ui_state.jog_continuous = 0; }
    s_x += s_btn_w + 10.0f;
    if (ui_button(ui_rect(s_x, s_y, s_btn_w, 30.0f), "10.0 mm")) { mm_ui_state.jog_step = 10.00f; mm_ui_state.jog_continuous = 0; }
    s_x += s_btn_w + 10.0f;
    if (ui_button(ui_rect(s_x, s_y, 120.0f, 30.0f), mm_ui_state.jog_continuous ? "[Continuous]" : "Continuous"))
    {
        mm_ui_state.jog_continuous = !mm_ui_state.jog_continuous;
    }
    
    char step_desc[64];
    stbsp_snprintf(step_desc, sizeof(step_desc), "Current Step: %0.2f mm  |  Mode: %s", 
                   mm_ui_state.jog_step, mm_ui_state.jog_continuous ? "Continuous" : "Step Incremental");
    ui_draw_text(step_desc, s_x + 140.0f, s_y + 8.0f, 13.0f, UI_COLOR_ACCENT);
    
    cur_y += 80.0f;
    
    // D-Pad Direction Controls
    f32 pad_w = (total_w - 20.0f) * 0.6f;
    f32 pad_h = 320.0f;
    Rng2F32 dpad_box = ui_rect(cur_x, cur_y, pad_w, pad_h);
    ui_group_box(dpad_box, "X / Y Plane Directional Jog");
    
    f32 center_x = cur_x + (pad_w * 0.5f);
    f32 center_y = cur_y + (pad_h * 0.5f);
    f32 j_btn_sz = 65.0f;
    f32 step = mm_ui_state.jog_step;
    
    // Y+ (Up)
    if (ui_button(ui_rect(center_x - (j_btn_sz * 0.5f), center_y - j_btn_sz - 15.0f, j_btn_sz, j_btn_sz), "Y +"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_y += step;
            if (mm_ui_state.connected) mc_axis_move_rel(2, step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    // Y- (Down)
    if (ui_button(ui_rect(center_x - (j_btn_sz * 0.5f), center_y + 15.0f, j_btn_sz, j_btn_sz), "Y -"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_y -= step;
            if (mm_ui_state.connected) mc_axis_move_rel(2, -step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    // X- (Left)
    if (ui_button(ui_rect(center_x - (j_btn_sz * 1.5f) - 15.0f, center_y - (j_btn_sz * 0.5f), j_btn_sz, j_btn_sz), "X -"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_x -= step;
            if (mm_ui_state.connected) mc_axis_move_rel(1, -step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    // X+ (Right)
    if (ui_button(ui_rect(center_x + (j_btn_sz * 0.5f) + 15.0f, center_y - (j_btn_sz * 0.5f), j_btn_sz, j_btn_sz), "X +"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_x += step;
            if (mm_ui_state.connected) mc_axis_move_rel(1, step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    // Center: Zero XY
    if (ui_button(ui_rect(center_x - (j_btn_sz * 0.5f), center_y - (j_btn_sz * 0.5f), j_btn_sz, j_btn_sz), "XY (0)"))
    {
        mm_ui_state.pos_x = mm_ui_state.target_x = 0.0f;
        mm_ui_state.pos_y = mm_ui_state.target_y = 0.0f;
        if (mm_ui_state.connected) { mc_axis_zero_position(1); mc_axis_zero_position(2); }
    }
    
    // Z & A Axes Column
    f32 vert_x = cur_x + pad_w + 20.0f;
    f32 vert_w = total_w - pad_w - 20.0f;
    Rng2F32 za_box = ui_rect(vert_x, cur_y, vert_w, pad_h);
    ui_group_box(za_box, "Z Axis & Rotary A Axis Jog");
    
    f32 za_btn_w = (vert_w - 40.0f) * 0.5f;
    
    // Z Buttons
    ui_draw_text("Z AXIS (Vertical)", vert_x + 15.0f, cur_y + 35.0f, 12.0f, UI_COLOR_ACCENT);
    if (ui_button(ui_rect(vert_x + 15.0f, cur_y + 60.0f, za_btn_w, 50.0f), "Z + (Up)"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_z += step;
            if (mm_ui_state.connected) mc_axis_move_rel(3, step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(vert_x + 15.0f, cur_y + 120.0f, za_btn_w, 50.0f), "Z - (Down)"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_z -= step;
            if (mm_ui_state.connected) mc_axis_move_rel(3, -step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(vert_x + 15.0f, cur_y + 180.0f, za_btn_w, 35.0f), "Zero Z"))
    {
        mm_ui_state.pos_z = mm_ui_state.target_z = 0.0f;
        if (mm_ui_state.connected) mc_axis_zero_position(3);
    }
    
    // A Buttons (Rotary)
    f32 a_col_x = vert_x + 25.0f + za_btn_w;
    ui_draw_text("A AXIS (Rotary)", a_col_x, cur_y + 35.0f, 12.0f, UI_COLOR_ACCENT);
    if (ui_button(ui_rect(a_col_x, cur_y + 60.0f, za_btn_w, 50.0f), "A + (CW)"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_a += step;
            if (mm_ui_state.connected) mc_axis_move_rel(4, step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(a_col_x, cur_y + 120.0f, za_btn_w, 50.0f), "A - (CCW)"))
    {
        if (mm_ui_state.servo_enabled && !mm_ui_state.emergency_stopped)
        {
            mm_ui_state.target_a -= step;
            if (mm_ui_state.connected) mc_axis_move_rel(4, -step, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(a_col_x, cur_y + 180.0f, za_btn_w, 35.0f), "Zero A"))
    {
        mm_ui_state.pos_a = mm_ui_state.target_a = 0.0f;
        if (mm_ui_state.connected) mc_axis_zero_position(4);
    }
}

// View E: Machine & Motion Controller Configuration
static void mm_ui__render_tab_config(Rng2F32 content_rect)
{
    f32 pad = 16.0f;
    f32 cur_x = content_rect.x0 + pad;
    f32 cur_y = content_rect.y0 + pad;
    f32 total_w = (content_rect.x1 - content_rect.x0) - (pad * 2.0f);
    
    ui_draw_text("MACHINE CONTROLLER & AXIS CONFIGURATION", cur_x, cur_y, 16.0f, UI_COLOR_WHITE);
    cur_y += 30.0f;
    
    f32 col_w = (total_w - 20.0f) * 0.5f;
    
    // Card Settings Box
    Rng2F32 card_box = ui_rect(cur_x, cur_y, col_w, 230.0f);
    ui_group_box(card_box, "GoogolTech GTS Controller Parameters");
    
    ui_draw_text("Card Index: 0 (GTS800-PCI-8Axis)", cur_x + 15.0f, cur_y + 30.0f, 12.0f, UI_COLOR_WHITE);
    ui_draw_text("Config File: GTS800.cfg", cur_x + 15.0f, cur_y + 55.0f, 12.0f, UI_COLOR_WHITE);
    ui_draw_text("Driver Library: gts.dll (V1.2.0.8)", cur_x + 15.0f, cur_y + 80.0f, 12.0f, UI_COLOR_WHITE);
    ui_draw_text("ExtMdl Library: ExtMdl.dll (Digital/Analog)", cur_x + 15.0f, cur_y + 105.0f, 12.0f, UI_COLOR_WHITE);
    
    char status_hw[64];
    stbsp_snprintf(status_hw, sizeof(status_hw), "Current State: %s", 
                   mm_ui_state.connected ? "HARDWARE ONLINE (GTS800 Active)" : "OFFLINE SIMULATION (No Card)");
    ui_draw_text(status_hw, cur_x + 15.0f, cur_y + 135.0f, 13.0f, mm_ui_state.connected ? UI_COLOR_SUCCESS : UI_COLOR_WARNING);
    
    if (ui_button(ui_rect(cur_x + 15.0f, cur_y + 170.0f, 180.0f, 35.0f), "Reload GTS800.cfg"))
    {
        if (mm_ui_state.connected)
        {
            mc_card_load_config("GTS800.cfg");
            mm_ui_add_log("Reloaded configuration file GTS800.cfg into GTS card.");
        }
        else
        {
            mm_ui_add_log("[SIM] Configuration reloaded in simulation memory.");
        }
    }
    
    // Axis Limits & Soft Travel Box
    f32 lim_x = cur_x + col_w + 20.0f;
    Rng2F32 lim_box = ui_rect(lim_x, cur_y, col_w, 230.0f);
    ui_group_box(lim_box, "Software Travel Limits (mm)");
    
    ui_draw_text("Axis 1 (X Travel):  -500.00 mm  to  +500.00 mm", lim_x + 15.0f, cur_y + 30.0f, 12.0f, UI_COLOR_WHITE);
    ui_draw_text("Axis 2 (Y Travel):  -350.00 mm  to  +350.00 mm", lim_x + 15.0f, cur_y + 60.0f, 12.0f, UI_COLOR_WHITE);
    ui_draw_text("Axis 3 (Z Travel):  -250.00 mm  to  +50.00 mm",  lim_x + 15.0f, cur_y + 90.0f, 12.0f, UI_COLOR_WHITE);
    ui_draw_text("Axis 4 (A Rotary):  -360.00 deg to  +360.00 deg", lim_x + 15.0f, cur_y + 120.0f, 12.0f, UI_COLOR_WHITE);
    
    if (ui_button(ui_rect(lim_x + 15.0f, cur_y + 170.0f, 180.0f, 35.0f), "Save Machine Limits"))
    {
        mm_ui_add_log("Machine travel limits saved to registry.");
    }
}

// View F: Diagnostics & Live Event Log
static void mm_ui__render_tab_diagnostics(Rng2F32 content_rect)
{
    f32 pad = 16.0f;
    f32 cur_x = content_rect.x0 + pad;
    f32 cur_y = content_rect.y0 + pad;
    f32 total_w = (content_rect.x1 - content_rect.x0) - (pad * 2.0f);
    
    ui_draw_text("SYSTEM TELEMETRY & EVENT AUDIT LOG", cur_x, cur_y, 16.0f, UI_COLOR_WHITE);
    cur_y += 30.0f;
    
    // Subsystem Health Card
    Rng2F32 health_rect = ui_rect(cur_x, cur_y, total_w, 140.0f);
    ui_group_box(health_rect, "Subsystem Health Status");
    
    ui_draw_text(mm_ui_state.connected ? "[OK] GTS Motion Card: Hardware active" : "[STANDBY] GTS Motion Card: Offline simulation mode",
                 cur_x + 15.0f, cur_y + 25.0f, 12.0f, mm_ui_state.connected ? UI_COLOR_SUCCESS : UI_COLOR_TEXT_MUTED);
    
    ui_draw_text(mm_ui_state.servo_enabled ? "[OK] Drive Power Stage: Energized" : "[STANDBY] Drive Power Stage: Standby (Unpowered)",
                 cur_x + 15.0f, cur_y + 50.0f, 12.0f, mm_ui_state.servo_enabled ? UI_COLOR_SUCCESS : UI_COLOR_TEXT_MUTED);
                 
    ui_draw_text(mm_ui_state.emergency_stopped ? "[ALERT] Safety Circuit: E-STOP TRIGGERED" : "[OK] Safety Circuit: Safety loop closed",
                 cur_x + 15.0f, cur_y + 75.0f, 12.0f, mm_ui_state.emergency_stopped ? UI_COLOR_DANGER : UI_COLOR_SUCCESS);
                 
    ui_draw_text("[OK] Backend Architecture: Core UI (Raylib) + Machine HAL (GoogolTech)",
                 cur_x + 15.0f, cur_y + 100.0f, 12.0f, UI_COLOR_SUCCESS);
                 
    cur_y += 155.0f;
    
    // Live Event Log Box
    Rng2F32 log_box = ui_rect(cur_x, cur_y, total_w, 240.0f);
    ui_group_box(log_box, "Live Audit Logs & Alerts");
    
    f32 log_line_y = cur_y + 25.0f;
    for (int i = 0; i < mm_ui_state.log_count; ++i)
    {
        char line_buf[128];
        stbsp_snprintf(line_buf, sizeof(line_buf), "[%02d] %s", i + 1, mm_ui_state.log_messages[i]);
        ui_draw_text(line_buf, cur_x + 15.0f, log_line_y, 12.0f, UI_COLOR_WHITE);
        log_line_y += 24.0f;
    }
    
    cur_y += 250.0f;
    if (ui_button(ui_rect(cur_x, cur_y, 140.0f, 32.0f), "Clear Event Log"))
    {
        mm_ui_state.log_count = 0;
        mm_ui_add_log("Audit log cleared.");
    }
    if (ui_button(ui_rect(cur_x + 155.0f, cur_y, 160.0f, 32.0f), "Test Log Message"))
    {
        mm_ui_add_log("Diagnostic self-test completed: all systems nominal.");
    }
}

// Dynamic Dispatcher for Central Content Area
static void mm_ui__render_main_content(Vec2F32 win_sz, f32 top_bar_h, f32 sidebar_w, f32 bottom_bar_h)
{
    f32 content_w = win_sz.x - sidebar_w;
    f32 content_h = win_sz.y - top_bar_h - bottom_bar_h;
    Rng2F32 content_rect = ui_rect(sidebar_w, top_bar_h, content_w, content_h);
    
    // Clear main content background
    ui_draw_rect(content_rect, UI_COLOR_BG);
    
    switch (mm_ui_state.active_tab)
    {
        case MM_NavTab_Overview:     mm_ui__render_tab_overview(content_rect); break;
        case MM_NavTab_Motors:       mm_ui__render_tab_motors(content_rect); break;
        case MM_NavTab_IO:           mm_ui__render_tab_io(content_rect); break;
        case MM_NavTab_Jog:          mm_ui__render_tab_jog(content_rect); break;
        case MM_NavTab_Config:       mm_ui__render_tab_config(content_rect); break;
        case MM_NavTab_Diagnostics:  mm_ui__render_tab_diagnostics(content_rect); break;
        default:                     mm_ui__render_tab_overview(content_rect); break;
    }
}

//
//~ 4. Bottom Status Bar: Connection, Safety, Operator, Telemetry
//
static void mm_ui__render_bottom_bar(Vec2F32 win_sz, f32 bottom_bar_h)
{
    f32 bar_y = win_sz.y - bottom_bar_h;
    Rng2F32 bar_rect = ui_rect(0.0f, bar_y, win_sz.x, bottom_bar_h);
    ui_draw_rect(bar_rect, UI_COLOR_BAR_BG);
    ui_draw_rect(ui_rect(0.0f, bar_y, win_sz.x, 1.0f), UI_COLOR_BORDER);
    
    f32 text_y = bar_y + 7.0f;
    f32 cur_x = 12.0f;
    
    // Segment 1: Controller Connection
    if (mm_ui_state.connected)
    {
        ui_draw_circle(cur_x + 6.0f, bar_y + 14.0f, 5.0f, UI_COLOR_SUCCESS);
        ui_draw_text("GTS800: ONLINE", cur_x + 18.0f, text_y, 11.0f, UI_COLOR_WHITE);
    }
    else
    {
        ui_draw_circle(cur_x + 6.0f, bar_y + 14.0f, 5.0f, UI_COLOR_LED_OFF);
        ui_draw_text("GTS800: OFFLINE (SIM)", cur_x + 18.0f, text_y, 11.0f, UI_COLOR_TEXT_MUTED);
    }
    cur_x += 160.0f;
    ui_draw_rect(ui_rect(cur_x, bar_y + 4.0f, 1.0f, bottom_bar_h - 8.0f), UI_COLOR_BORDER);
    cur_x += 12.0f;
    
    // Segment 2: Drives
    if (mm_ui_state.servo_enabled)
    {
        ui_draw_circle(cur_x + 6.0f, bar_y + 14.0f, 5.0f, UI_COLOR_SUCCESS);
        ui_draw_text("DRIVES: ENERGIZED", cur_x + 18.0f, text_y, 11.0f, UI_COLOR_WHITE);
    }
    else
    {
        ui_draw_circle(cur_x + 6.0f, bar_y + 14.0f, 5.0f, UI_COLOR_LED_OFF);
        ui_draw_text("DRIVES: STANDBY", cur_x + 18.0f, text_y, 11.0f, UI_COLOR_TEXT_MUTED);
    }
    cur_x += 150.0f;
    ui_draw_rect(ui_rect(cur_x, bar_y + 4.0f, 1.0f, bottom_bar_h - 8.0f), UI_COLOR_BORDER);
    cur_x += 12.0f;
    
    // Segment 3: Safety Loop
    if (mm_ui_state.emergency_stopped)
    {
        ui_draw_circle(cur_x + 6.0f, bar_y + 14.0f, 5.0f, UI_COLOR_DANGER);
        ui_draw_text("! E-STOP ACTIVE !", cur_x + 18.0f, text_y, 11.0f, UI_COLOR_DANGER);
    }
    else
    {
        ui_draw_circle(cur_x + 6.0f, bar_y + 14.0f, 5.0f, UI_COLOR_SUCCESS);
        ui_draw_text("SAFETY LOOP: OK", cur_x + 18.0f, text_y, 11.0f, UI_COLOR_WHITE);
    }
    cur_x += 140.0f;
    ui_draw_rect(ui_rect(cur_x, bar_y + 4.0f, 1.0f, bottom_bar_h - 8.0f), UI_COLOR_BORDER);
    cur_x += 12.0f;
    
    // Segment 4: Active Operator
    char user_txt[48];
    stbsp_snprintf(user_txt, sizeof(user_txt), "USER: %s", mm_ui_state.user_name);
    ui_draw_text(user_txt, cur_x, text_y, 11.0f, UI_COLOR_TEXT_MUTED);
    cur_x += 160.0f;
    ui_draw_rect(ui_rect(cur_x, bar_y + 4.0f, 1.0f, bottom_bar_h - 8.0f), UI_COLOR_BORDER);
    cur_x += 12.0f;
    
    // Segment 5: Status Message
    ui_draw_text(mm_ui_state.status_message, cur_x, text_y, 11.0f, UI_COLOR_WHITE);
    
    // Right Side: FPS Counter
    ui_draw_fps((int)(win_sz.x - 85.0f), (int)(bar_y + 6.0f));
}

//
//~ 5. Modal Dialogs (Login, User Management)
//
static void mm_ui__render_modals(Vec2F32 win_sz)
{
    if (mm_ui_state.show_login_modal)
    {
        // Backdrop dimming
        ui_draw_rect(ui_rect(0.0f, 0.0f, win_sz.x, win_sz.y), ui_color(0.0f, 0.0f, 0.0f, 0.60f));
        
        // Centered Window Box
        f32 modal_w = 380.0f;
        f32 modal_h = 240.0f;
        f32 modal_x = (win_sz.x - modal_w) * 0.5f;
        f32 modal_y = (win_sz.y - modal_h) * 0.5f;
        
        Rng2F32 modal_rect = ui_rect(modal_x, modal_y, modal_w, modal_h);
        if (ui_window_box(modal_rect, "OPERATOR ACCESS CONTROL"))
        {
            mm_ui_state.show_login_modal = 0;
        }
        
        ui_draw_text("Select Operator Level:", modal_x + 25.0f, modal_y + 45.0f, 13.0f, UI_COLOR_WHITE);
        
        if (ui_button(ui_rect(modal_x + 25.0f, modal_y + 75.0f, modal_w - 50.0f, 32.0f), "Operator (Standard)"))
        {
            stbsp_snprintf(mm_ui_state.user_name, sizeof(mm_ui_state.user_name), "Floor Operator");
            mm_ui_state.show_login_modal = 0;
            mm_ui_add_log("Logged in as Floor Operator.");
        }
        
        if (ui_button(ui_rect(modal_x + 25.0f, modal_y + 115.0f, modal_w - 50.0f, 32.0f), "Maintenance Technician"))
        {
            stbsp_snprintf(mm_ui_state.user_name, sizeof(mm_ui_state.user_name), "Technician");
            mm_ui_state.show_login_modal = 0;
            mm_ui_add_log("Logged in as Maintenance Technician.");
        }
        
        if (ui_button(ui_rect(modal_x + 25.0f, modal_y + 155.0f, modal_w - 50.0f, 32.0f), "Administrator (Full Access)"))
        {
            stbsp_snprintf(mm_ui_state.user_name, sizeof(mm_ui_state.user_name), "Admin (Full Access)");
            mm_ui_state.show_login_modal = 0;
            mm_ui_add_log("Logged in as Administrator.");
        }
        
        if (ui_button(ui_rect(modal_x + 25.0f, modal_y + 195.0f, modal_w - 50.0f, 28.0f), "Cancel"))
        {
            mm_ui_state.show_login_modal = 0;
        }
    }
}

//
//~ Main Interface Update & Render Function
//
void mm_ui_update_and_render(void)
{
    f32 dt = ui_get_frame_time();
    Vec2F32 win_sz = ui_get_window_size();
    
    // Layout dimension constants
    f32 top_bar_h = 52.0f;
    f32 sidebar_w = 230.0f;
    f32 bottom_bar_h = 28.0f;
    
    // Update simulation / hardware telemetry
    mm_ui__tick_simulation(dt);
    
    // 1. Render Top Bar (Project, Play/Pause/Stop, Action icons)
    mm_ui__render_top_bar(win_sz, top_bar_h);
    
    // 2. Render Left Sidebar (File / Subsystem Tree)
    mm_ui__render_sidebar(win_sz, top_bar_h, sidebar_w, bottom_bar_h);
    
    // 3. Render Center Main Layout (Dynamic View by Tab)
    mm_ui__render_main_content(win_sz, top_bar_h, sidebar_w, bottom_bar_h);
    
    // 4. Render Bottom Status Bar
    mm_ui__render_bottom_bar(win_sz, bottom_bar_h);
    
    // 5. Render Modals & Dialogs (if open)
    mm_ui__render_modals(win_sz);
}

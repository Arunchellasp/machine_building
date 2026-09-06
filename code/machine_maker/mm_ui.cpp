#include "mm_ui.h"

// ==============================================================================
// Machine Maker UI Implementation
// ==============================================================================
// Uses ONLY ui_core functions (ui_button, ui_slider, ui_window_box, etc.).
// No Raylib or Windows headers are included here!
// ==============================================================================

MM_UI_State mm_ui_state = {0};

void mm_ui_init(void)
{
    mm_ui_state.initialized = 1;
    mm_ui_state.connected = 0;
    mm_ui_state.servo_enabled = 0;
    mm_ui_state.emergency_stopped = 0;
    
    mm_ui_state.pos_x = 0.0f;
    mm_ui_state.pos_y = 0.0f;
    mm_ui_state.pos_z = 0.0f;
    mm_ui_state.feed_rate = 500.0f;
    mm_ui_state.spindle_rpm = 12000.0f;
    
    mm_ui_state.show_diagnostics = 1;
    stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), "System Ready. Controller Offline (Simulation Mode).");
}

void mm_ui_update_and_render(void)
{
    // Draw background FPS counter
    ui_draw_fps(10, 10);
    
    //
    //~ Panel 1: Main Controller Window
    //
    Rng2F32 main_window_rect = ui_rect(40.0f, 40.0f, 480.0f, 620.0f);
    ui_window_box(main_window_rect, "MACHINE MAKER - MOTION CONTROLLER");
    
    // Status indicator
    Rng2F32 status_bar_rect = ui_rect(50.0f, 75.0f, 460.0f, 30.0f);
    ui_status_bar(status_bar_rect, mm_ui_state.status_message);
    
    //
    //~ Connection & System Controls
    //
    Rng2F32 conn_btn_rect = ui_rect(50.0f, 115.0f, 145.0f, 35.0f);
    if (ui_button(conn_btn_rect, mm_ui_state.connected ? "Disconnect" : "Connect GTS"))
    {
        if (!mm_ui_state.connected)
        {
            // Only initialize hardware upon explicit button click
            MC_Result res = mc_init(0, "GTS800.cfg");
            if (res == MC_OK)
            {
                mm_ui_state.connected = 1;
                MC_CardInfo info;
                mc_card_get_info(&info);
                stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), 
                               "GTS800 Card %d Connected (DLL: %s).", info.card_no, info.dll_version);
            }
            else
            {
                mm_ui_state.connected = 0;
                stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), 
                               "[OFFLINE] No GTS card detected (Code: %d). Running in Offline Simulation.", (int)res);
            }
        }
        else
        {
            mc_shutdown();
            mm_ui_state.connected = 0;
            mm_ui_state.servo_enabled = 0;
            stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), "Controller Disconnected.");
        }
    }
    
    Rng2F32 servo_btn_rect = ui_rect(205.0f, 115.0f, 145.0f, 35.0f);
    if (ui_button(servo_btn_rect, mm_ui_state.servo_enabled ? "Disable Servos" : "Enable Servos"))
    {
        if (!mm_ui_state.emergency_stopped)
        {
            mm_ui_state.servo_enabled = !mm_ui_state.servo_enabled;
            if (mm_ui_state.connected)
            {
                if (mm_ui_state.servo_enabled)
                {
                    mc_axis_enable(1);
                    mc_axis_enable(2);
                    mc_axis_enable(3);
                }
                else
                {
                    mc_axis_disable(1);
                    mc_axis_disable(2);
                    mc_axis_disable(3);
                }
            }
            
            stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), 
                           mm_ui_state.servo_enabled ? 
                           (mm_ui_state.connected ? "Servos Energized (Axes 1-3)." : "Servos Enabled (Simulated).") : 
                           "Servos Disabled.");
        }
    }
    
    Rng2F32 estop_btn_rect = ui_rect(360.0f, 115.0f, 150.0f, 35.0f);
    if (ui_button(estop_btn_rect, mm_ui_state.emergency_stopped ? "CLEAR E-STOP" : "! E-STOP !"))
    {
        mm_ui_state.emergency_stopped = !mm_ui_state.emergency_stopped;
        if (mm_ui_state.emergency_stopped)
        {
            if (mm_ui_state.connected)
            {
                mc_axes_stop_all(1);
            }
            mm_ui_state.servo_enabled = 0;
            stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), "EMERGENCY STOP TRIGGERED! Motion halted.");
        }
        else
        {
            stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), "E-Stop Cleared. Ready to enable.");
        }
    }
    
    //
    //~ Axis Digital Readout (DRO)
    //
    Rng2F32 dro_group_rect = ui_rect(50.0f, 165.0f, 460.0f, 210.0f);
    ui_group_box(dro_group_rect, "Axis Coordinates & Manual Jog (mm)");
    
    char buf[64];
    
    // Axis X
    stbsp_snprintf(buf, sizeof(buf), "X: %8.3f mm", mm_ui_state.pos_x);
    ui_label(ui_rect(65.0f, 195.0f, 170.0f, 30.0f), buf);
    if (ui_button(ui_rect(250.0f, 195.0f, 60.0f, 30.0f), "X -"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_x -= 1.0f;
            mc_axis_move_rel(1, -1.0, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(320.0f, 195.0f, 60.0f, 30.0f), "X +"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_x += 1.0f;
            mc_axis_move_rel(1, 1.0, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(400.0f, 195.0f, 90.0f, 30.0f), "Zero X"))
    {
        mm_ui_state.pos_x = 0.0f;
        mc_axis_zero_position(1);
    }
    
    // Axis Y
    stbsp_snprintf(buf, sizeof(buf), "Y: %8.3f mm", mm_ui_state.pos_y);
    ui_label(ui_rect(65.0f, 235.0f, 170.0f, 30.0f), buf);
    if (ui_button(ui_rect(250.0f, 235.0f, 60.0f, 30.0f), "Y -"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_y -= 1.0f;
            mc_axis_move_rel(2, -1.0, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(320.0f, 235.0f, 60.0f, 30.0f), "Y +"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_y += 1.0f;
            mc_axis_move_rel(2, 1.0, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(400.0f, 235.0f, 90.0f, 30.0f), "Zero Y"))
    {
        mm_ui_state.pos_y = 0.0f;
        mc_axis_zero_position(2);
    }
    
    // Axis Z
    stbsp_snprintf(buf, sizeof(buf), "Z: %8.3f mm", mm_ui_state.pos_z);
    ui_label(ui_rect(65.0f, 275.0f, 170.0f, 30.0f), buf);
    if (ui_button(ui_rect(250.0f, 275.0f, 60.0f, 30.0f), "Z -"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_z -= 0.5f;
            mc_axis_move_rel(3, -0.5, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(320.0f, 275.0f, 60.0f, 30.0f), "Z +"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_z += 0.5f;
            mc_axis_move_rel(3, 0.5, mm_ui_state.feed_rate, 0.5, 0.5);
        }
    }
    if (ui_button(ui_rect(400.0f, 275.0f, 90.0f, 30.0f), "Zero Z"))
    {
        mm_ui_state.pos_z = 0.0f;
        mc_axis_zero_position(3);
    }
    
    // Home All
    if (ui_button(ui_rect(65.0f, 325.0f, 425.0f, 35.0f), "HOME ALL AXES (0, 0, 0)"))
    {
        if (mm_ui_state.servo_enabled)
        {
            mm_ui_state.pos_x = 0.0f;
            mm_ui_state.pos_y = 0.0f;
            mm_ui_state.pos_z = 0.0f;
            for (short a = 1; a <= 3; ++a) mc_axis_zero_position(a);
            stbsp_snprintf(mm_ui_state.status_message, sizeof(mm_ui_state.status_message), "All axes homed successfully.");
        }
    }

    
    //
    //~ Motion Parameters
    //
    Rng2F32 params_rect = ui_rect(50.0f, 390.0f, 460.0f, 160.0f);
    ui_group_box(params_rect, "Motion Parameters");
    
    ui_label(ui_rect(65.0f, 415.0f, 120.0f, 25.0f), "Feed Rate:");
    ui_slider(ui_rect(190.0f, 415.0f, 240.0f, 25.0f), "0", "2000 mm/min", &mm_ui_state.feed_rate, 0.0f, 2000.0f);
    
    ui_label(ui_rect(65.0f, 455.0f, 120.0f, 25.0f), "Spindle RPM:");
    ui_slider(ui_rect(190.0f, 455.0f, 240.0f, 25.0f), "0", "24000 RPM", &mm_ui_state.spindle_rpm, 0.0f, 24000.0f);
    
    ui_check_box(ui_rect(65.0f, 495.0f, 160.0f, 25.0f), "Show Diagnostics Panel", &mm_ui_state.show_diagnostics);
    
    //
    //~ Panel 2: Diagnostics (Optional right-side panel)
    //
    if (mm_ui_state.show_diagnostics)
    {
        Rng2F32 diag_rect = ui_rect(540.0f, 40.0f, 400.0f, 350.0f);
        ui_window_box(diag_rect, "DIAGNOSTICS & TELEMETRY");
        
        ui_label(ui_rect(560.0f, 80.0f, 360.0f, 25.0f), "Subsystem Status:");
        ui_label(ui_rect(570.0f, 110.0f, 340.0f, 25.0f), mc_card_is_open() ? "[OK] GTS Motion Card active" : "[OFFLINE] No motion card");
        ui_label(ui_rect(570.0f, 135.0f, 340.0f, 25.0f), mm_ui_state.servo_enabled ? "[OK] Drives energized" : "[STANDBY] Drives unpowered");
        ui_label(ui_rect(570.0f, 160.0f, 340.0f, 25.0f), mm_ui_state.emergency_stopped ? "[ALERT] E-STOP ACTIVE" : "[OK] Safety loop closed");
        
        ui_label(ui_rect(560.0f, 200.0f, 360.0f, 25.0f), "Architecture Layering:");
        ui_label(ui_rect(570.0f, 225.0f, 340.0f, 25.0f), "Application: machine_maker/mm_ui.cpp");
        ui_label(ui_rect(570.0f, 250.0f, 340.0f, 25.0f), "Core UI: ui/ui_core.h (Agnostic Wrapper)");
        ui_label(ui_rect(570.0f, 275.0f, 340.0f, 25.0f), "Backend: ui/ui_core.cpp (Raylib + Raygui)");
        ui_label(ui_rect(570.0f, 300.0f, 340.0f, 25.0f), "Motion HAL: machine/machine_core.h (GTS Wrapper)");
    }
}

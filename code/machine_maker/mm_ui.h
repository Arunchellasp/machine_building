#ifndef MM_UI_H
#define MM_UI_H

// ==============================================================================
// mm_ui.h - Machine Maker UI: Modern HMI & Windows Application Interface
// ==============================================================================
// Adaptable layout consisting of:
//   - Top Header Bar: Highlighted Project Name, Play/Stop/Resume controls, Action Icons
//   - Left Sidebar (File/Subsystem Tree): Hierarchical navigation (Overview, Motors, IO, etc.)
//   - Central Main Layout: Dynamic view rendering based on active selection
//   - Bottom Status Bar: Hardware telemetry, safety status, and user session
// ==============================================================================

#include "ui/ui_core.h"
#include "machine/machine_core.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//~ Navigation Tabs (Sidebar / Tree items)
//
typedef enum MM_NavTab
{
    MM_NavTab_Overview,     // Live Machine Dashboard & DRO
    MM_NavTab_Motors,       // Motor & Axis Detailed Status
    MM_NavTab_IO,           // Individual Digital & Analog IO Status
    MM_NavTab_Jog,          // Manual Jogging & Coordinates
    MM_NavTab_Config,       // Machine & GTS Controller Configuration
    MM_NavTab_Diagnostics,  // System Telemetry & Event Log
    MM_NavTab_COUNT
} MM_NavTab;

//
//~ Machine Execution State
//
typedef enum MM_ExecState
{
    MM_ExecState_Idle,
    MM_ExecState_Running,
    MM_ExecState_Paused,
    MM_ExecState_Stopped,
    MM_ExecState_EStop
} MM_ExecState;

//
//~ UI Application State
//
struct MM_UI_State
{
    b32 initialized;
    
    // Project & Session
    char project_name[64];
    char user_name[32];
    b32  logged_in;
    
    // Navigation & Machine State
    MM_NavTab   active_tab;
    MM_ExecState exec_state;
    b32         connected;
    b32         servo_enabled;
    b32         emergency_stopped;
    
    // Axis Coordinates & Dynamics (Axes 1..4: X, Y, Z, A)
    f32 pos_x;
    f32 pos_y;
    f32 pos_z;
    f32 pos_a;
    
    f32 target_x;
    f32 target_y;
    f32 target_z;
    f32 target_a;
    
    f32 vel_x;
    f32 vel_y;
    f32 vel_z;
    
    // Motion Parameters
    f32 feed_rate;         // mm/min
    f32 feed_override;     // 0 - 200%
    f32 spindle_rpm;       // RPM
    f32 spindle_override;  // 0 - 200%
    f32 jog_step;          // 0.01, 0.1, 1.0, 10.0 mm
    int jog_continuous;    // 0 = step, 1 = continuous
    
    // I/O States (Simulated or live from hardware)
    u16 gpi_bits;          // 16 digital input bits
    u16 gpo_bits;          // 16 digital output bits
    u8  limit_pos[4];
    u8  limit_neg[4];
    u8  home_sensor[4];
    u8  drive_alarm[4];
    
    // Modals / Overlays
    b32 show_login_modal;
    b32 show_config_modal;
    
    // Status Bar & Event Log
    char status_message[128];
    char log_messages[8][96];
    int  log_count;
};

extern MM_UI_State mm_ui_state;

//
//~ Lifecycle & Update
//
void mm_ui_init(void);
void mm_ui_update_and_render(void);
void mm_ui_add_log(const char *msg);

#ifdef __cplusplus
}
#endif

#endif // MM_UI_H

#ifndef MM_UI_H
#define MM_UI_H

#include "ui/ui_core.h"

// ==============================================================================
// Machine Maker UI: Application Layer
// ==============================================================================
// This application UI layer interacts exclusively with the abstract ui_core API.
// It has zero knowledge of Raylib or Raygui. If you swap Raylib for another
// backend in ui_core, this code remains completely untouched.
// ==============================================================================

struct MM_UI_State
{
    b32 initialized;
    
    // Machine status
    b32 connected;
    b32 servo_enabled;
    b32 emergency_stopped;
    
    // Axis positions & settings
    f32 pos_x;
    f32 pos_y;
    f32 pos_z;
    f32 feed_rate;
    f32 spindle_rpm;
    
    // UI controls
    b32 show_diagnostics;
    char status_message[128];
};

extern MM_UI_State mm_ui_state;

void mm_ui_init(void);
void mm_ui_update_and_render(void);

#endif // MM_UI_H

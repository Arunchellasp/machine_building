// ==============================================================================
// Machine Maker - Main Entry File (Unity Build)
// ==============================================================================
// All third-party and platform libraries are included here at the top in their
// natural dependency order. Subordinate files (base, os, ui, app) do NOT need to
// re-include external libraries; they inherit everything from this master file.
// ==============================================================================

//
//~ 1. C Standard Library Headers
//
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <stddef.h>
#include <string.h>


//
//~ 2. Third-Party: STB Libraries
//
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

//
//~ 3. Third-Party: Raylib & Raylib-GUI
//
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//
//~ 4. Platform: Windows Headers (with symbol isolation from Raylib)
//
#define Rectangle     __WindowsRectangle
#define CloseWindow   __WindowsCloseWindow
#define ShowCursor    __WindowsShowCursor
#define LoadImage     __WindowsLoadImage
#define DrawText      __WindowsDrawText
#define DrawTextEx    __WindowsDrawTextEx
#define PlaySound     __WindowsPlaySound

#include <windows.h>
#include <commdlg.h>
#include <xinput.h>
#include <iphlpapi.h>
#include <shobjidl.h>
#define SECURITY_WIN32
#include <Security.h>
#include <wininet.h>
#include <dwmapi.h>

#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef LoadImage
#undef DrawText
#undef DrawTextEx
#undef PlaySound

//
//~ 5. Third-Party: GoogolTech Motion Control
//
#include "gts.h"
#include "ExtMdl.h"

//
//~ 6. Codebase Subsystem Headers
//
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "machine/machine_core.h"
#include "ui/ui_core.h"
#include "mm_ui.h"

//
//~ 7. Codebase Subsystem Implementations (Unity Build)
//
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "machine/machine_core.cpp"
#include "ui/ui_core.cpp"
#include "mm_ui.cpp"

//
//~ 8. Application Main Entry Point
//
int wmain(int argc, wchar_t **argv)
{
    // Initialize window and rendering subsystem
    ui_init(1280, 720, "Machine Maker - CNC & Motion Controller");
    
    // Initialize Machine Maker application state
    mm_ui_init();
    
    // Main loop
    while (!ui_window_should_close())
    {
        ui_begin_frame();
        ui_clear_background(UI_COLOR_BG);
        
        // Render Machine Maker interface
        mm_ui_update_and_render();
        
        ui_end_frame();
    }
    
    // Cleanup
    ui_close();
    return 0;
}

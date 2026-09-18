
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <windows.h>


#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STBIR_USE_FMA
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define IMGUI_IMPLEMENTATION
#include "misc/single_file/imgui_single_file.h"

#include "glad/gl.h"
#include "GLFW/glfw3.h"


#include "backends/imgui_impl_opengl3.cpp"
#include "backends/imgui_impl_glfw.cpp"





//
//~ 5. Third-Party: GoogolTech Motion Control
//
#include "gts.h"
#include "ExtMdl.h"

//
//~ 6. Codebase Subsystem Headers
//
//#include "base/base_inc.h"
//#include "machine/machine_core.h"
//#include "ui/ui_core.h"
//#include "machine_maker/mm_ui.h"

//
//~ 7. Codebase Subsystem Implementations (Unity Build)
//
//#include "base/base_inc.cpp"
//#include "machine/machine_core.cpp"
//#include "ui/ui_core.cpp"
//#include "machine_maker/mm_ui.cpp"


// NOTE(ARUN): glfw_window

enum Window_Size{
    WINDOW_1080p = 0,
    WINDOW_1024p,
    WINDOW_720p,
    WINDOW_480p
};
enum Window_Type{
    WINDOW_FULL_SCREEN = 0,
    WINDOW_WINDOWED,
};

// NOTE(ARUN): base_type

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i08;
typedef int8_t i8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u08;
typedef uint8_t u8;

typedef i64 b64;
typedef i32 b32;
typedef i16 b16;
typedef i08 b08;
typedef i8 b8;

typedef float r32;
typedef double r64;

typedef r32 f32;
typedef r64 f64;

#define function static
#define local_persist static
#define global_variable static

#define Assert(exp) {if(!(exp)){*(int *)0 = 0;}}
#define Array_Count(arr) (sizeof(arr) / sizeof(arr[0]))

#define KB(val) (1024 * (val))
#define MB(val) (1024 * KB(val))
#define GB(val) (1024 * MB(val))



// NOTE(ARUN): memory




struct Memory_Arena{
    u8 *base;
    u32 size;
    u32 used;
};

struct App_Memory{
    Memory_Arena permanent;
    Memory_Arena temporary;
};


function u64 get_alignment_offset(Memory_Arena *arena, u64 alignment){
    u64 alignment_offset = 0;
    
    u64 result_pointer = (u64)arena->base + arena->used;
    u64 alignment_mask = alignment - 1;
    if(result_pointer & alignment_mask){
        alignment_offset = alignment - (result_pointer & alignment_mask);
    }
    
    return(alignment_offset);
}

#define push_struct(arena, type) (type *)push_size(arena, sizeof(type))
#define push_array(arena, type, num) (type *)push_size(arena, sizeof(type) * num)

function u8 *push_size(Memory_Arena *arena, u32 size_init, u64 alignment = 4){
    u32 size = size_init;
    
    i32 alignment_offset = (i32)get_alignment_offset(arena, alignment);
    size += alignment_offset;
    
    Assert((arena->used + size) <= arena->size);
    u8 *result = arena->base + arena->used + alignment_offset;
    arena->used += size;
    
    Assert(size >= size_init);
    
    return result;
}

function void zero_memory(u8 *location, u32 bytes){
    while(bytes--){
        *location++ = 0;
    }
}

function void copy_memory(u8 *from, u32 size, u8 *to){
    while(size--){
        *to++ = *from++;
    }
}

// NOTE(ARUN): app

struct Imgui_Data{
    char string[256];
};

struct App{
    b32 is_initialized;
    Imgui_Data imgui_data;
};

// NOTE(ARUN): GUI

#define Text_Left(...) AlignTextToFramePadding(); Text(__VA_ARGS__); SameLine();
#define Text_LeftEx(pos, size, ...) AlignTextToFramePadding(); Text(__VA_ARGS__); SameLine(pos); SetNextItemWidth(size);

function void imgui_help(const char* desc){
    using namespace ImGui;
    if (IsItemHovered(ImGuiHoveredFlags_DelayShort)){
        BeginTooltip();
        PushTextWrapPos(GetFontSize() * 35.0f);
        TextUnformatted(desc);
        PopTextWrapPos();
        EndTooltip();
    }
}

function void imgui_help_marker(const char* desc){
    ImGui::TextDisabled("(?)");
    imgui_help(desc);
}



function void update_imgui(Imgui_Data *data)
{
    using namespace ImGui;
    
    NewFrame();
    ImGuiIO *io = &GetIO();
    
    
    Begin("Demo", 0);
    Text("%s\n%.1f FPS", data->string, io->Framerate);
    End();
    
    Render();
}

function void init_imgui(){
    // NOTE(rsb): Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();
    
    // NOTE(rsb): Enable type of controls
    ImGuiIO *io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // NOTE(rsb): Set styles
    ImGuiStyle *style = &ImGui::GetStyle();
    style->WindowPadding.x = 15.0f;
    style->FrameRounding = 12.0f;
    style->FramePadding.y = 2.0f;
    style->ItemSpacing.x = 8.0f;
    style->ItemSpacing.y = 5.0f;
}



// NOTE(ARUN): app





function b32 update_app_and_imgui(App_Memory *memory){
    b32 exit_app = 0;
    App *app = (App *)memory->permanent.base;
    
    // NOTE(rsb): Init app
    if(!app->is_initialized){
        app->is_initialized = 1;
        
        app = push_struct(&memory->permanent, App);
        init_imgui();
    }
    
    // NOTE(rsb): App code
    Imgui_Data *imgui_data = &app->imgui_data;
    stbsp_sprintf(imgui_data->string, "%s", "Hello world!");
    
    // NOTE(rsb): Call imgui functions
    update_imgui(imgui_data);
    
    if(exit_app){
        // NOTE(rsb): Deinit functions
    }
    
    return exit_app;
}




// NOTE(ARUN): Window functions
// NOTE(ARUN): Window uses GLFW library.
function void glfw_error_callback(int error, const char* description){
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

function GLFWwindow *init_window(Window_Size size, Window_Type type, char *window_name){
    
    // NOTE(ARUN): Init GLFW
    
    glfwSetErrorCallback(glfw_error_callback);
    b8 is_successful = 0;
    
    is_successful = (b8) glfwInit();
    if(!is_successful){
        // // TODO(ARUN):  How to handle if init fails!
        Assert(0);
    }
    
    // NOTE(ARUN): GL 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    // NOTE(ARUN): Create window with graphics context
    i32 width = 0, height = 0;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    switch(size){
        case WINDOW_1080p:{
            monitor = (type == WINDOW_FULL_SCREEN) ? monitor : NULL;
            width = 1920;
            height = 1080;
        } break;
        case WINDOW_1024p:{
            monitor = (type == WINDOW_FULL_SCREEN) ? monitor : NULL;
            width = 1280;
            height = 1024;
        } break;
        case WINDOW_720p:{
            monitor = (type == WINDOW_FULL_SCREEN) ? monitor : NULL;
            width = 1280;
            height = 720;
        } break;
        case WINDOW_480p:{
            monitor = (type == WINDOW_FULL_SCREEN) ? monitor : NULL;
            width = 640;
            height = 480;
        } break;
        default: Assert(0);
    }
    
    GLFWwindow *handle = glfwCreateWindow(width, height, window_name, monitor, NULL);
    is_successful = (handle != NULL);
    if (!is_successful){
        // TODO(ARUN): How to handle if create window fails?
        Assert(0);
    }
    
    glfwMakeContextCurrent(handle);
    glfwSwapInterval(0); // 0/1 - Vsync off/on
    
    return handle;
}

function b8 is_window_open(GLFWwindow *handle){
    b8 is_open = !glfwWindowShouldClose(handle);
    return is_open;
}

function void poll_window_events(){
    glfwPollEvents();
}

function void clear_window(GLFWwindow *handle, r32 r = 1.0f, r32 g = 1.0f, r32 b = 1.0f, r32 a = 0.0f){
    int width, height;
    glfwGetFramebufferSize(handle, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(a*r, a*g, a*b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

function void update_window(GLFWwindow *handle){
    glfwSwapBuffers(handle);
}

function void close_window(GLFWwindow  *handle){
    glfwDestroyWindow(handle);
    glfwTerminate();
}



global_variable LARGE_INTEGER PERF_COUNTER_FREQUENCY;

function LARGE_INTEGER win32_get_cpu_clock_count(){
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

function r32 win32_get_seconds_elasped(LARGE_INTEGER start, LARGE_INTEGER end){
    r32 result = (r32)(end.QuadPart - start.QuadPart) / (r32)PERF_COUNTER_FREQUENCY.QuadPart;
    return result;
}

int wmain(void){
    App_Memory memory = {};
    memory.permanent.size = MB(4);
    memory.temporary.size = MB(2);
    
    memory.permanent.base = (u8 *)calloc(memory.permanent.size + memory.temporary.size, sizeof(u8));
    Assert(memory.permanent.base);
    memory.temporary.base = memory.permanent.base + memory.permanent.size;
    
    GLFWwindow *window = init_window(WINDOW_480p, WINDOW_WINDOWED, "Endobot");
    
    // NOTE(ARUN): Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    
    // NOTE(ARUN): Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    // NOTE(ARUN): Setup counters for fps
    r32 target_fps = 60.0f;
    r32 target_seconds_per_frame = 1.0f / target_fps;
    
    u32 desired_scheduler_ms = 1;
    b32 is_sleep_granular = (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);
    
    QueryPerformanceFrequency(&PERF_COUNTER_FREQUENCY);
    LARGE_INTEGER last_cpu_clock_count = win32_get_cpu_clock_count();;
    
    b32 exit_app = 0;
    
    while(is_window_open(window) && !exit_app){
        
        // NOTE(ARUN): Create new window frame for Imgui backend
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        
        // NOTE(ARUN): Update app and imgui
        exit_app = update_app_and_imgui(&memory);
        
        // NOTE(ARUN): Check for any resize / minimizing window
        poll_window_events();
        
        // NOTE(ARUN): Clear window with background color
        clear_window(window);
        
        // NOTE(ARUN): Generate gui image
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // NOTE(ARUN): Update gui image to window
        update_window(window);
        
        // NOTE(ARUN): Enforce fps
        r32 seconds_elasped_for_frame = win32_get_seconds_elasped(last_cpu_clock_count, win32_get_cpu_clock_count());
        
        if(seconds_elasped_for_frame < target_seconds_per_frame){
            while(seconds_elasped_for_frame < target_seconds_per_frame){
                if(is_sleep_granular){
                    i32 sleep_ms = (i32) (1000.0f * (target_seconds_per_frame - seconds_elasped_for_frame));
                    if(sleep_ms > 0){
                        Sleep(sleep_ms);
                    }
                }
                seconds_elasped_for_frame = win32_get_seconds_elasped(last_cpu_clock_count, win32_get_cpu_clock_count());
            }
        } else {
            // NOTE(ARUN): Missed frame rate
        }
        
        last_cpu_clock_count = win32_get_cpu_clock_count();
    }
    
    // NOTE(ARUN): Close gui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // NOTE(ARUN): Close window
    close_window(window);
    
    return 0;
}
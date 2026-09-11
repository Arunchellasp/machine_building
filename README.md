# 🛠️ Machine Maker - CNC & Motion Controller (Nuklear + GLFW)

A modern, adaptable **CNC & Motion Controller HMI** and Windows desktop application built with **Nuklear GUI**, **GLFW 3.4**, and **OpenGL 2**, integrated with the **GoogolTech (GTS)** motion control card platform.

Designed using a **data-oriented, layered architecture** with single-translation-unit (unity build) compilation via MSVC.

---

## 🏛️ Architecture & Layering

```
+-------------------------------------------------------------------------+
|                  Application Layer: Machine Maker UI                    |
|                (code/machine_maker/mm_ui.h & mm_ui.cpp)                 |
|   - Top Bar: Project Badge, Play/Pause/Stop, State Pill, Action Icons   |
|   - Left Navigation Tree: Status, Motion, Setup hierarchy               |
|   - Dynamic Content Views: DRO, Motor Telemetry, 16-bit IO, Jog Pendant |
|   - Bottom Status Bar: GTS800 status, drives, safety loop, telemetry    |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|              Backend-Agnostic UI Core Layer (Pure C API)                |
|                      (code/ui/ui_core.h)                                |
|   - ui_button, ui_slider, ui_label, ui_check_box, ui_window_box...      |
|   - ui_draw_rect, ui_draw_circle, ui_draw_text, ui_draw_fps             |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                Backend Implementation (code/ui/ui_core.cpp)             |
|                 Nuklear GUI + GLFW 3.4 + OpenGL 2 Pipeline              |
|   - Window Lifecycle: glfwInit, glfwCreateWindow, glfwPollEvents        |
|   - Nuklear Engine: nk_glfw3_init, nk_layout_space, nk_command_buffer   |
|   - Hardware Acceleration: Standard Windows OpenGL 2 (opengl32.lib)     |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|               Motion Control HAL (code/machine/machine_core.*)          |
|                   GoogolTech (GTS) Multi-Axis Controller                |
+-------------------------------------------------------------------------+
```

---

## 🖥️ Layout Features

- **Top Bar**: Highlighted Project Name badge, prominent Play / Pause / Resume / Stop execution controls, State Pill badge (`IDLE`, `RUNNING`, `PAUSED`, `STOPPED`, `E-STOP`), and extensible action icons (Connect GTS, Origin Home, Machine Config, User Login).
- **Left Navigation Tree**: Subsystem hierarchy (`▼ MACHINE STATUS`, `▼ MOTION CONTROL`, `▼ SYSTEM SETUP`).
- **Dynamic Center Layout**:
  - **Overview / DRO**: 4-Axis coordinate readouts (X, Y, Z, A), feed rate and spindle speed sliders with override % readouts, and quick actions.
  - **Motor Status**: Multi-axis telemetry table (target, actual, error diff, velocity, servo state, Limit+, Limit-, Home, Drive Alarm LEDs, Zero/Clear actions).
  - **Digital & Analog I/O**: 16-bit General Purpose Inputs (GPI) with sensor LEDs and simulation toggles, and 16-bit General Purpose Outputs (GPO) with interactive actuator toggles.
  - **Manual Jog**: Virtual pendant with step increments (`0.01 mm`, `0.10 mm`, `1.00 mm`, `10.0 mm`, `Continuous`), D-Pad for X/Y, vertical column for Z, and rotary column for A.
  - **Machine Config**: GTS controller settings, configuration file path, software travel limits per axis, and save/reload actions.
  - **Diagnostics & Logs**: Subsystem health checklist and a live audit log console.
- **Bottom Status Bar**: Multi-segment status display (GTS800 Online/Offline, Drive Power Stage, Safety Loop, Operator, and real-time FPS).
- **Offline Simulation**: Zero hardware calls at startup; full UI interaction and realistic motion interpolation without requiring physical hardware connected.

---

## ⚡ Quick Start

### 1️⃣ Prerequisites
- **Windows 10 / 11**
- **MSVC 64-bit (`cl.exe`)** in `PATH` (e.g. via Developer Command Prompt)

### 2️⃣ Build
```cmd
.\build.bat
```

### 3️⃣ Run
```cmd
.\build\machine_maker.exe
```


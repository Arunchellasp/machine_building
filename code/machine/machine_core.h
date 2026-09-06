// ==============================================================================
// machine_core.h - Hardware & Motion Control Abstraction Layer
// ==============================================================================
// Pure C wrapper for GoogolTech (GTS) motion controller cards and extension modules.
//
// Layering:
//   Located outside machine_maker so it can be utilized across different products.
//   Separates GTS functionality into:
//     1. Card Management   (mc_card_*)
//     2. Axis Management   (mc_axis_*)
//     3. IO Management     (mc_io_* & mc_ext_io_*)
//     4. System Lifecycle  (mc_init / mc_shutdown)
// ==============================================================================

#ifndef MACHINE_CORE_H
#define MACHINE_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

//
//~ Return Codes & Error Constants
//
typedef enum MC_Result
{
    MC_OK                  = 0,
    MC_ERR_NOT_OPEN        = -1,
    MC_ERR_INVALID_PARAM   = -2,
    MC_ERR_COMMUNICATION   = -3,
    MC_ERR_CONFIG_FAILED   = -4,
    MC_ERR_AXIS_DISABLED   = -5,
    MC_ERR_MOTION_FAILED   = -6,
    MC_ERR_ALARM_ACTIVE    = -7,
    MC_ERR_EXT_IO_FAILED   = -8,
} MC_Result;

//
//~ Axis Status Structure
//
typedef struct MC_AxisStatus
{
    int enabled;           // Servo / drive is energized
    int moving;            // Axis is currently in motion
    int in_position;       // Axis reached target position
    int alarm;             // Drive alarm active
    int limit_positive;    // Positive hardware limit switch triggered
    int limit_negative;    // Negative hardware limit switch triggered
    int home_sensor;       // Home / origin sensor triggered
    int error_active;      // Following error or general error
    unsigned long raw_bits;// Raw 32-bit status mask from controller
} MC_AxisStatus;

//
//~ Motion Parameters Structure
//
typedef struct MC_MotionParams
{
    double vel;            // Target velocity (pulse/ms or engineering units)
    double acc;            // Acceleration (pulse/ms^2)
    double dec;            // Deceleration (pulse/ms^2)
    short  smooth_time;    // S-curve smoothing parameter (0-50 ms)
} MC_MotionParams;

//
//~ Card Information Structure
//
typedef struct MC_CardInfo
{
    short is_open;
    short card_no;
    char  dll_version[64];
    unsigned long clock;
} MC_CardInfo;

// ==============================================================================
// 1. Card Management (mc_card_*)
// ==============================================================================
MC_Result mc_card_open(short card_no);
MC_Result mc_card_close(void);
MC_Result mc_card_reset(void);
MC_Result mc_card_load_config(const char *config_file_path);
int       mc_card_is_open(void);
short     mc_card_get_card_no(void);
MC_Result mc_card_get_info(MC_CardInfo *out_info);
MC_Result mc_card_get_clock(unsigned long *out_clock);

// ==============================================================================
// 2. Axis Management (mc_axis_*)
// ==============================================================================
MC_Result mc_axis_enable(short axis);
MC_Result mc_axis_disable(short axis);
int       mc_axis_is_enabled(short axis);
MC_Result mc_axis_clear_status(short axis);
MC_Result mc_axis_get_status(short axis, MC_AxisStatus *out_status);
MC_Result mc_axis_zero_position(short axis);

// Position & Velocity Feedback
double    mc_axis_get_target_pos(short axis);
double    mc_axis_get_actual_pos(short axis);
double    mc_axis_get_actual_vel(short axis);
MC_Result mc_axis_set_position(short axis, double pos);

// Motion Execution
MC_Result mc_axis_jog(short axis, double velocity, double acceleration);
MC_Result mc_axis_move_abs(short axis, double target_pos, double vel, double acc, double dec);
MC_Result mc_axis_move_rel(short axis, double delta_pos, double vel, double acc, double dec);
MC_Result mc_axis_stop(short axis, int emergency);
MC_Result mc_axes_stop_all(int emergency);
int       mc_axis_is_moving(short axis);

// ==============================================================================
// 3. IO Management - Onboard Controller IO (mc_io_*)
// ==============================================================================

// Digital Inputs (GPI, Limits, Home, Alarm)
MC_Result mc_io_get_gpi_all(unsigned long *out_bits);
int       mc_io_get_gpi_bit(short bit_index);
int       mc_io_get_limit_positive(short axis);
int       mc_io_get_limit_negative(short axis);
int       mc_io_get_home_sensor(short axis);
int       mc_io_get_alarm_sensor(short axis);

// Digital Outputs (GPO)
MC_Result mc_io_get_gpo_all(unsigned long *out_bits);
MC_Result mc_io_set_gpo_all(unsigned long bitmask);
MC_Result mc_io_set_gpo_bit(short bit_index, short state);
int       mc_io_get_gpo_bit(short bit_index);

// ==============================================================================
// 4. Extension Module IO - ExtMdl (mc_ext_io_*)
// ==============================================================================
MC_Result mc_ext_io_open(const char *dll_name);
MC_Result mc_ext_io_close(void);
int       mc_ext_io_is_open(void);
MC_Result mc_ext_io_read_value(short module_index, unsigned short *out_value);
MC_Result mc_ext_io_write_value(short module_index, unsigned short value);
int       mc_ext_io_read_bit(short module_index, short bit_index);
MC_Result mc_ext_io_write_bit(short module_index, short bit_index, short state);

// ==============================================================================
// 5. System Lifecycle Convenience Routines (mc_init / mc_shutdown)
// ==============================================================================
MC_Result mc_init(short card_no, const char *config_file_path);
MC_Result mc_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // MACHINE_CORE_H

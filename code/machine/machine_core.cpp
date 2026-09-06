// ==============================================================================
// machine_core.cpp - Hardware & Motion Control Abstraction Layer Implementation
// ==============================================================================
// Pure C implementation (in .cpp extension) wrapping GoogolTech GTS motion control API.
// ==============================================================================



// Fallback includes if not already in scope via unity build
#ifndef GTS_H
#if defined(__cplusplus)
extern "C" {
#endif
#include "gts.h"
#include "ExtMdl.h"
#if defined(__cplusplus)
}
#endif
#endif

//
//~ Internal State
//
static struct
{
    short is_open;
    short card_no;
    short ext_io_open;
    char  dll_version[64];
} g_mc_state = {0};

// ==============================================================================
// 1. Card Management
// ==============================================================================

MC_Result mc_card_open(short card_no)
{
    short ret = 0;
    char *p_ver = NULL;
    
    // Set active card index
    GT_SetCardNo(card_no);
    
    // Open controller card (mode 1: standard mode)
    ret = GT_Open(card_no, 1);
    if (ret != 0)
    {
        g_mc_state.is_open = 0;
        return MC_ERR_COMMUNICATION;
    }
    
    g_mc_state.is_open = 1;
    g_mc_state.card_no = card_no;
    
    // Query DLL Version string
    ret = GT_GetDllVersion(&p_ver);
    if (ret == 0 && p_ver != NULL)
    {
        strncpy(g_mc_state.dll_version, p_ver, sizeof(g_mc_state.dll_version) - 1);
        g_mc_state.dll_version[sizeof(g_mc_state.dll_version) - 1] = '\0';
    }
    else
    {
        strcpy(g_mc_state.dll_version, "Unknown");
    }
    
    return MC_OK;
}

MC_Result mc_card_close(void)
{
    if (!g_mc_state.is_open)
    {
        return MC_OK;
    }
    
    GT_Close();
    g_mc_state.is_open = 0;
    return MC_OK;
}

MC_Result mc_card_reset(void)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_Reset();
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_card_load_config(const char *config_file_path)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    if (!config_file_path || config_file_path[0] == '\0') return MC_ERR_INVALID_PARAM;
    
    short ret = GT_LoadConfig((char *)config_file_path);
    return (ret == 0) ? MC_OK : MC_ERR_CONFIG_FAILED;
}

int mc_card_is_open(void)
{
    return (int)g_mc_state.is_open;
}

short mc_card_get_card_no(void)
{
    return g_mc_state.card_no;
}

MC_Result mc_card_get_info(MC_CardInfo *out_info)
{
    if (!out_info) return MC_ERR_INVALID_PARAM;
    out_info->is_open = g_mc_state.is_open;
    out_info->card_no = g_mc_state.card_no;
    strncpy(out_info->dll_version, g_mc_state.dll_version, sizeof(out_info->dll_version));
    
    unsigned long clock = 0;
    if (g_mc_state.is_open)
    {
        GT_GetClock(&clock, NULL);
    }
    out_info->clock = clock;
    return MC_OK;
}

MC_Result mc_card_get_clock(unsigned long *out_clock)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    if (!out_clock) return MC_ERR_INVALID_PARAM;
    short ret = GT_GetClock(out_clock, NULL);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

// ==============================================================================
// 2. Axis Management
// ==============================================================================

MC_Result mc_axis_enable(short axis)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_AxisOn(axis);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_axis_disable(short axis)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_AxisOff(axis);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

int mc_axis_is_enabled(short axis)
{
    if (!g_mc_state.is_open) return 0;
    long sts = 0;
    short ret = GT_GetSts(axis, &sts, 1, NULL);
    if (ret != 0) return 0;
    return (sts & (1 << 9)) ? 1 : 0; // Bit 9: Servo ON
}

MC_Result mc_axis_clear_status(short axis)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_ClrSts(axis, 1);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_axis_get_status(short axis, MC_AxisStatus *out_status)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    if (!out_status) return MC_ERR_INVALID_PARAM;
    
    long sts = 0;
    short ret = GT_GetSts(axis, &sts, 1, NULL);
    if (ret != 0) return MC_ERR_COMMUNICATION;
    
    out_status->raw_bits       = (unsigned long)sts;
    out_status->alarm          = (sts & (1 << 1))  ? 1 : 0; // Bit 1: Drive alarm
    out_status->limit_positive = (sts & (1 << 5))  ? 1 : 0; // Bit 5: Positive limit
    out_status->limit_negative = (sts & (1 << 6))  ? 1 : 0; // Bit 6: Negative limit
    out_status->error_active   = (sts & (1 << 4))  ? 1 : 0; // Bit 4: Following error
    out_status->enabled        = (sts & (1 << 9))  ? 1 : 0; // Bit 9: Servo on
    out_status->moving         = (sts & (1 << 10)) ? 1 : 0; // Bit 10: Motion profile generating
    out_status->in_position    = (!out_status->moving && out_status->enabled) ? 1 : 0;
    out_status->home_sensor    = mc_io_get_home_sensor(axis);
    
    return MC_OK;
}

MC_Result mc_axis_zero_position(short axis)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_ZeroPos(axis, 1);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

double mc_axis_get_target_pos(short axis)
{
    if (!g_mc_state.is_open) return 0.0;
    double pos = 0.0;
    GT_GetPrfPos(axis, &pos, 1, NULL);
    return pos;
}

double mc_axis_get_actual_pos(short axis)
{
    if (!g_mc_state.is_open) return 0.0;
    double pos = 0.0;
    GT_GetAxisEncPos(axis, &pos, 1, NULL);
    return pos;
}

double mc_axis_get_actual_vel(short axis)
{
    if (!g_mc_state.is_open) return 0.0;
    double vel = 0.0;
    GT_GetAxisPrfVel(axis, &vel, 1, NULL);
    return vel;
}

MC_Result mc_axis_set_position(short axis, double pos)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_SetPos(axis, (long)pos);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_axis_jog(short axis, double velocity, double acceleration)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    
    // Set profile mode to Jog
    short ret = GT_PrfJog(axis);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    TJogPrm prm;
    prm.acc    = (acceleration > 0) ? acceleration : 0.1;
    prm.dec    = prm.acc;
    prm.smooth = 0.5;
    
    ret = GT_SetJogPrm(axis, &prm);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    ret = GT_SetVel(axis, velocity);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    ret = GT_Update(1 << (axis - 1));
    return (ret == 0) ? MC_OK : MC_ERR_MOTION_FAILED;
}

MC_Result mc_axis_move_abs(short axis, double target_pos, double vel, double acc, double dec)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    
    // Set profile mode to Trapezoidal
    short ret = GT_PrfTrap(axis);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    TTrapPrm prm;
    prm.acc        = (acc > 0) ? acc : 0.1;
    prm.dec        = (dec > 0) ? dec : prm.acc;
    prm.velStart   = 0.0;
    prm.smoothTime = 25; // 25ms S-curve smoothing
    
    ret = GT_SetTrapPrm(axis, &prm);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    ret = GT_SetPos(axis, (long)target_pos);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    ret = GT_SetVel(axis, vel);
    if (ret != 0) return MC_ERR_MOTION_FAILED;
    
    ret = GT_Update(1 << (axis - 1));
    return (ret == 0) ? MC_OK : MC_ERR_MOTION_FAILED;
}

MC_Result mc_axis_move_rel(short axis, double delta_pos, double vel, double acc, double dec)
{
    double current_pos = mc_axis_get_target_pos(axis);
    return mc_axis_move_abs(axis, current_pos + delta_pos, vel, acc, dec);
}

MC_Result mc_axis_stop(short axis, int emergency)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    long mask = 1 << (axis - 1);
    long option = emergency ? mask : 0;
    short ret = GT_Stop(mask, option);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_axes_stop_all(int emergency)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    long mask = 0xFF; // All 8 axes
    long option = emergency ? 0xFF : 0;
    short ret = GT_Stop(mask, option);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

int mc_axis_is_moving(short axis)
{
    if (!g_mc_state.is_open) return 0;
    long sts = 0;
    short ret = GT_GetSts(axis, &sts, 1, NULL);
    if (ret != 0) return 0;
    return (sts & (1 << 10)) ? 1 : 0; // Bit 10: motion running
}

// ==============================================================================
// 3. Onboard IO Management
// ==============================================================================

MC_Result mc_io_get_gpi_all(unsigned long *out_bits)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    if (!out_bits) return MC_ERR_INVALID_PARAM;
    long val = 0;
    short ret = GT_GetDi(MC_GPI, &val);
    *out_bits = (unsigned long)val;
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

int mc_io_get_gpi_bit(short bit_index)
{
    if (!g_mc_state.is_open) return 0;
    long val = 0;
    short ret = GT_GetDi(MC_GPI, &val);
    if (ret != 0) return 0;
    return (val & (1 << bit_index)) ? 1 : 0;
}

int mc_io_get_limit_positive(short axis)
{
    if (!g_mc_state.is_open) return 0;
    long val = 0;
    short ret = GT_GetDi(MC_LIMIT_POSITIVE, &val);
    if (ret != 0) return 0;
    return (val & (1 << (axis - 1))) ? 1 : 0;
}

int mc_io_get_limit_negative(short axis)
{
    if (!g_mc_state.is_open) return 0;
    long val = 0;
    short ret = GT_GetDi(MC_LIMIT_NEGATIVE, &val);
    if (ret != 0) return 0;
    return (val & (1 << (axis - 1))) ? 1 : 0;
}

int mc_io_get_home_sensor(short axis)
{
    if (!g_mc_state.is_open) return 0;
    long val = 0;
    short ret = GT_GetDi(MC_HOME, &val);
    if (ret != 0) return 0;
    return (val & (1 << (axis - 1))) ? 1 : 0;
}

int mc_io_get_alarm_sensor(short axis)
{
    if (!g_mc_state.is_open) return 0;
    long val = 0;
    short ret = GT_GetDi(MC_ALARM, &val);
    if (ret != 0) return 0;
    return (val & (1 << (axis - 1))) ? 1 : 0;
}

MC_Result mc_io_get_gpo_all(unsigned long *out_bits)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    if (!out_bits) return MC_ERR_INVALID_PARAM;
    long val = 0;
    short ret = GT_GetDo(MC_GPO, &val);
    *out_bits = (unsigned long)val;
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_io_set_gpo_all(unsigned long bitmask)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_SetDo(MC_GPO, (long)bitmask);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_io_set_gpo_bit(short bit_index, short state)
{
    if (!g_mc_state.is_open) return MC_ERR_NOT_OPEN;
    short ret = GT_SetDoBit(MC_GPO, bit_index, state ? 1 : 0);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

int mc_io_get_gpo_bit(short bit_index)
{
    if (!g_mc_state.is_open) return 0;
    long val = 0;
    short ret = GT_GetDo(MC_GPO, &val);
    if (ret != 0) return 0;
    return (val & (1 << bit_index)) ? 1 : 0;
}

// ==============================================================================
// 4. Extension Module IO (ExtMdl)
// ==============================================================================

MC_Result mc_ext_io_open(const char *dll_name)
{
    const char *path = (dll_name && dll_name[0] != '\0') ? dll_name : "ExtMdl.dll";
    short ret = GT_OpenExtMdl((char *)path);
    if (ret != 0)
    {
        g_mc_state.ext_io_open = 0;
        return MC_ERR_EXT_IO_FAILED;
    }
    
    g_mc_state.ext_io_open = 1;
    return MC_OK;
}

MC_Result mc_ext_io_close(void)
{
    if (!g_mc_state.ext_io_open) return MC_OK;
    GT_CloseExtMdl();
    g_mc_state.ext_io_open = 0;
    return MC_OK;
}

int mc_ext_io_is_open(void)
{
    return (int)g_mc_state.ext_io_open;
}

MC_Result mc_ext_io_read_value(short module_index, unsigned short *out_value)
{
    if (!g_mc_state.ext_io_open) return MC_ERR_NOT_OPEN;
    if (!out_value) return MC_ERR_INVALID_PARAM;
    short ret = GT_GetExtIoValue(module_index, out_value);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

MC_Result mc_ext_io_write_value(short module_index, unsigned short value)
{
    if (!g_mc_state.ext_io_open) return MC_ERR_NOT_OPEN;
    short ret = GT_SetExtIoValue(module_index, value);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

int mc_ext_io_read_bit(short module_index, short bit_index)
{
    if (!g_mc_state.ext_io_open) return 0;
    unsigned short val = 0;
    short ret = GT_GetExtIoValue(module_index, &val);
    if (ret != 0) return 0;
    return (val & (1 << bit_index)) ? 1 : 0;
}

MC_Result mc_ext_io_write_bit(short module_index, short bit_index, short state)
{
    if (!g_mc_state.ext_io_open) return MC_ERR_NOT_OPEN;
    short ret = GT_SetExtIoBit(module_index, bit_index, state ? 1 : 0);
    return (ret == 0) ? MC_OK : MC_ERR_COMMUNICATION;
}

// ==============================================================================
// 5. System Lifecycle Convenience Routines
// ==============================================================================

MC_Result mc_init(short card_no, const char *config_file_path)
{
    MC_Result res = mc_card_open(card_no);
    if (res != MC_OK) return res;
    
    mc_card_reset();
    
    if (config_file_path && config_file_path[0] != '\0')
    {
        mc_card_load_config(config_file_path);
    }
    
    // Clear status and reset positions on all 8 axes
    for (short a = 1; a <= 8; ++a)
    {
        mc_axis_clear_status(a);
        mc_axis_zero_position(a);
    }
    
    // Try opening extension module
    mc_ext_io_open("ExtMdl.dll");
    
    return MC_OK;
}

MC_Result mc_shutdown(void)
{
    if (!g_mc_state.is_open) return MC_OK;
    
    // Emergency stop all axes and disable drives
    mc_axes_stop_all(1);
    for (short a = 1; a <= 8; ++a)
    {
        mc_axis_disable(a);
    }
    
    // Close extension IO and card
    mc_ext_io_close();
    mc_card_close();
    
    return MC_OK;
}

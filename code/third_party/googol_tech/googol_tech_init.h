// ============================================================
// googol_tech_init.h - GoogolTech Motion Controller Master Header & Initializer
//
// Includes all GoogolTech subsystem headers (.h files), links vendor
// import libraries (.lib), and provides hardware initialization routines.
// ============================================================

#ifndef GOOGOL_TECH_INIT_H
#define GOOGOL_TECH_INIT_H

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Primary GoogolTech Motion Control API Declarations
#include "gts.h"

// Controller Resource & Configuration Structures
#include "config.h"

// GoogolTech Extension Module API (Digital/Analog IO)
#include "ExtMdl.h"

// Look-Ahead, Path Optimization, & Machine-Mode Extension API
#include "LookAheadEx.h"

#ifdef __cplusplus
}
#endif


// Automatically link required vendor import libraries (.lib) for MSVC
#if defined(_MSC_VER)
#pragma comment(lib, "gts.lib")
#pragma comment(lib, "ExtMdl.lib")
#endif

// ============================================================
// Helper Function: GoogolTech_Init
// Initializes GoogolTech motion controller card, clears axis states,
// loads configuration file, and initializes extension modules.
// Returns 0 on success, non-zero vendor error code on failure.
// ============================================================
static inline short GoogolTech_Init(short cardNo, const char *cfgFile)
{
    short sRtn = 0;
    char *pDllVersion = NULL;

    printf("[GoogolTech] Initializing GoogolTech Motion Control System...\n");

    // 1. Get DLL Version
    sRtn = GT_GetDllVersion(&pDllVersion);
    if (sRtn == 0 && pDllVersion)
    {
        printf("[GoogolTech] DLL Version: %s\n", pDllVersion);
    }

    // 2. Set Active Card Number
    sRtn = GT_SetCardNo(cardNo);
    if (sRtn != 0)
    {
        printf("[GoogolTech] Note/Result setting card number %d (Code: %d)\n", cardNo, sRtn);
    }

    // 3. Open Motion Controller Card
    sRtn = GT_Open(cardNo, 1);
    if (sRtn != 0)
    {
        printf("[GoogolTech] Note/Result opening card %d (Code: %d)\n", cardNo, sRtn);
    }
    else
    {
        printf("[GoogolTech] Motion card %d opened successfully.\n", cardNo);
    }

    // 4. Reset Controller
    sRtn = GT_Reset();
    if (sRtn != 0)
    {
        printf("[GoogolTech] Note/Result resetting controller (Code: %d)\n", sRtn);
    }

    // 5. Load Controller Configuration File (e.g. GTS800.cfg)
    if (cfgFile && cfgFile[0] != '\0')
    {
        printf("[GoogolTech] Loading config file: %s\n", cfgFile);
        sRtn = GT_LoadConfig((char *)cfgFile);
        if (sRtn != 0)
        {
            printf("[GoogolTech] Note/Result loading config file %s (Code: %d)\n", cfgFile, sRtn);
        }
        else
        {
            printf("[GoogolTech] Config file loaded successfully.\n");
        }
    }

    // 6. Clear Status & Reset Position for Axes 1-8
    GT_ClrSts(1, 8);
    GT_ZeroPos(1, 8);

    // 7. Initialize Extension Module (ExtMdl.dll)
    sRtn = GT_OpenExtMdl((char *)"ExtMdl.dll");
    if (sRtn != 0)
    {
        printf("[GoogolTech] Note/Result opening ExtMdl.dll (Code: %d)\n", sRtn);
    }
    else
    {
        printf("[GoogolTech] Extension module ExtMdl.dll opened successfully.\n");
    }

    printf("[GoogolTech] Initialization sequence complete.\n");
    return 0;
}

// ============================================================
// Helper Function: GoogolTech_Close
// Shuts down GoogolTech extension module and motion controller card.
// ============================================================
static inline short GoogolTech_Close(void)
{
    printf("[GoogolTech] Shutting down GoogolTech Motion Control System...\n");
    GT_CloseExtMdl();
    return GT_Close();
}

#endif // GOOGOL_TECH_INIT_H

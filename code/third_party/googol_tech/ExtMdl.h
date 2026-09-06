// ============================================================
// ExtMdl.h - GoogolTech extension module API
// Link requirements: ExtMdl.lib + ExtMdl.dll at runtime.
//
// Use guide:
// 1. GT_OpenExtMdl("ExtMdl.dll")
// 2. GT_SwitchtoCardNoExtMdl(card) when multiple controller cards are used.
// 3. GT_ResetExtMdl(), then GT_LoadExtConfig(configFile) if a config file is needed.
// 4. Use GT_SetExtIo*/GT_GetExtIo* for digital IO.
// 5. Use GT_GetExtAd*/GT_SetExtDa* for analog input/output.
// 6. Poll GT_GetStsExtMdl() when module/channel status matters.
// 7. GT_CloseExtMdl() before application exit.
//
// Return value convention: 0 means success; non-zero values indicate vendor API errors.
// ============================================================

#define GT_API extern "C" short __stdcall	//windows
//#define GT_API short				//dos

// Module lifecycle and card selection.
GT_API GT_OpenExtMdl(char *pDllName);
GT_API GT_CloseExtMdl();
GT_API GT_SwitchtoCardNoExtMdl(short card);

// Module reset, configuration, and digital IO.
GT_API GT_ResetExtMdl();
GT_API GT_LoadExtConfig(char *pFileName);
GT_API GT_SetExtIoValue(short mdl,unsigned short value);
GT_API GT_GetExtIoValue(short mdl,unsigned short *pValue);
GT_API GT_SetExtIoBit(short mdl,short index,unsigned short value);
GT_API GT_GetExtIoBit(short mdl,short index,unsigned short *pValue);

// Analog input/output and module status.
GT_API GT_GetExtAdValue(short mdl,short chn,unsigned short *pValue);
GT_API GT_GetExtAdVoltage(short mdl,short chn,double *pValue);
GT_API GT_SetExtDaValue(short mdl,short chn,unsigned short value);
GT_API GT_SetExtDaVoltage(short mdl,short chn,double value);
GT_API GT_GetStsExtMdl(short mdl,short chn,unsigned short *pStatus);

// Extension module mode and controller configuration transfer.
GT_API GT_GetExtMdlMode(short *pMode);
GT_API GT_SetExtMdlMode(short mode);
GT_API GT_UploadConfig();
GT_API GT_DownloadConfig();

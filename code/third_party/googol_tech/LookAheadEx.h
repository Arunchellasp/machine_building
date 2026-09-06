// ============================================================
// LookAheadEx.h - Look-ahead, path optimization, and machine-mode extension API.
// Runtime requirement: LAFunc.dll must be available to the process; use the
// matching vendor import library when the SDK provides one.
//
// Use guide:
// 1. Include this header when using GT_* look-ahead or path optimization APIs.
// 2. Initialize TLookAheadParameter with machine limits, pulse equivalents, and
//    coordinate mapping before enabling look-ahead processing.
// 3. Configure velocity/acceleration definition rules and optimization parameters.
// 4. Select the machine mode and coordinate mapping before preprocessing path data.
// 5. Check returned ErrorID/TErrorInfo values when preprocessing path data.
// 6. Use the same SDK version as gts.h/gts.dll to avoid ABI mismatches.
// ============================================================
#pragma once

#define LA_AXIS_NUM				8
#define LA_WORK_AXIS_NUM		6
#define LA_MACHINE_AXIS_NUM   5

#define AXIS_LIMIT_NONE       0       // No axis limit
#define AXIS_LIMIT_MAX_VEL    1       // Axis maximum velocity limit
#define AXIS_LIMIT_MAX_ACC    2       // Axis maximum acceleration limit
#define AXIS_LIMIT_MAX_DV     4       // Axis maximum velocity change limit

#define KIN_MSG_BUFFER_SIZE   32

// Whether the trajectory in the workpiece coordinate system limits velocity mode
enum EWorkLimitMode
{
	WORK_LIMIT_INVALID=0,		// No limit
	WORK_LIMIT_VALID,			// Limit enabled
};

// Velocity definition rule
enum EVelSettingDef
{
	NORMAL_DEF_VEL=0,			// Input is the composite velocity of all axes in the axis coordinate system
	NUM_DEF_VEL,				// Defined according to NUM system rules
	CUT_DEF_VEL,				// Velocity is cutting velocity
};

// Acceleration definition rule
enum EAccSettingDef
{
	NORMAL_DEF_ACC=0,             // Input is output
	LONG_AXIS_ACC,                // Long-axis maximum velocity
};

// Machine type
enum EMachineMode
{
	NORMAL_THREE_AXIS=0,		// Standard three-axis machine mode
	MULTI_AXES,					// Multi-axis interpolation mode
	FIVE_AXIS,					// Five-axis machine mode, axis coordinate system as primary and workpiece coordinate system as secondary
	FIVE_AXIS_WORK,				// Five-axis machine mode, workpiece coordinate system as primary and axis coordinate system as secondary
	ROBOT,
};

// Look-ahead parameter structure
struct TLookAheadParameter
{
	int lookAheadNum;					// Number of look-ahead segments
	double time;						// Time constant
	double radiusRatio;					// Curvature limit adjustment parameter
	double vMax[LA_AXIS_NUM];			// Maximum velocity of each axis
	double aMax[LA_AXIS_NUM];			// Maximum acceleration of each axis
	double DVMax[LA_AXIS_NUM];			// Maximum velocity change of each axis within the time constant
	double scale[LA_AXIS_NUM];			// Pulse equivalent of each axis
	short axisRelation[LA_AXIS_NUM];	// Mapping between input coordinates and internal coordinates
	char machineCfgFileName[128];		// Machine configuration file name
};

//////////////////////////////////////
typedef struct
{
	short RobotType;
	short reserved1;

	short KinParUse[18];
	double KinPar[18];
	short KinLimitUse[12];
	double KinLimitMin[12];
	double KinLimitMax[12];
	double KinLimitMinShift[12];
	double KinLimitMaxShift[12];

	short AxisUse[8];
	char AxisPosSignSwitch[8];
	double AxisPosOffset[8];

	short CartUnitUse[6];
	char CartPosKCSSignSwitch[6];
	short reserved2[3];
	double CartPosKCSOffset[6];
}RC_KIN_CONFIG;

typedef struct
{
	char Error;
	short ErrorID;
	char Message[129];
}RC_ERROR_INTERFACE;

typedef struct  
{
	short ErrorID;
	char Message[129];
	char LogTime[32];
	long InternalID;
}RC_MSG_BUFFER_ELEMENT;

typedef struct  
{
	short LastMsgIndex;
	RC_MSG_BUFFER_ELEMENT MsgElement[KIN_MSG_BUFFER_SIZE];
	long LastMsgID;
}RC_MSG_BUFFER;

// Rotation axis range settings
struct TRotationAxisRange
{
	int primaryAxisRangeOn;              // Whether the first rotary axis range limit is enabled, 0: disabled, 1: enabled
	int slaveAxisRangeOn;                // Whether the second rotary axis range limit is enabled, 0: disabled, 1: enabled
	double maxPrimaryAngle;              // Maximum value of the first rotary axis
	double minPrimaryAngle;              // Minimum value of the first rotary axis
	double maxSlaveAngle;                // Maximum value of the second rotary axis
	double minSlaveAgnle;                // Minimum value of the second rotary axis
};

// Solution selection parameters
enum EGroupSelect
{
	Continuous=0,
	Group_1,
	Group_2,
};

struct TPos
{
	double machinePos[LA_MACHINE_AXIS_NUM];
	double workPos[LA_WORK_AXIS_NUM];
};

// Velocity planning mode
enum EVelMode
{
	T_CURVE=0,
	S_CURVE,
	S_CURVE_NEW,                  // S-curve velocity look-ahead based on jerk and maximum acceleration, 2015.11.16

	VEL_MODE_MAX=0x10000,         // Ensure the length is 4 bytes
};

//////////////////////////////////
typedef enum{OPT_OFF, OPT_ON}OptimizeState;
typedef enum{NO_OPT, OPT_BLENDING, OPT_CIRCLEFITTING, OPT_CUBICSPLINE, OPT_BSPLINE}OptimizeMethod;

typedef enum{INIT_ERROR=1,		// Parameters were not initialized
			PASSWORD_ERROR,		// Password error. Run on the GoogolTech motion control platform
			INDATA_ERROR,		// Input data error (check whether arc data is correct)
			PRE_PROCESS_ERROR,	//
			TOOL_RADIUS_COMPENSATE_ERROR_INOUT,		// Tool radius compensation error: entry/end compensation cannot be an arc
			TOOL_RADIUS_COMPENSATE_ERROR_NOCROSS,	// Tool radius compensation error: unreasonable data; intersection cannot be calculated
			USERDATA_ERROR,
}ErrorID;

// Trajectory optimization parameter structure
typedef struct OptimizeParamUser 
{
	OptimizeState usePathOptimize;	// Whether to use path optimization: OPT_OFF: unused, OPT_ON: used

	float tolerance;				// Tolerance (suggest: rough:0.1, pre-finish:0.05, finish:0.01)

	OptimizeMethod optimizeMethod;	// Select curve optimization method

	OptimizeState keepLargeArc;		// Whether to keep large arcs: OPT_OFF: do not keep, OPT_ON: keep

	float blendingMinError;			// Minimum set error for blending

	float blendingMaxAngle;			// Maximum angle limit for blending; no blending when segment vector angle is greater than this value, unit: degrees

}TOptimizeParamUser;

struct TErrorInfo 
{
	ErrorID errorID;		// Error ID (INIT_ERROR: parameters not initialized; PRE_PROCESS_ERROR: preprocessing module error;
							// TOOL_RADIUS_COMPENSATE_ERROR: tool radius compensation error;)
	long errorRowNum;		// Error row number
};

struct TPreStartPos 
{
 double Pos[LA_AXIS_NUM];
};

typedef struct  
{
	unsigned short doType;
	unsigned short doMask;
	unsigned short doValue;
	unsigned short delayTime;
	short fifo;
}TBufIoDelayData;

typedef struct  
{
	short doType;
	short doIndex;
	unsigned short highLevelTime;
	unsigned short lowLevelTime;
	long pulseNum;
	short firstLevel;
	short fifo;
}TBufDoBitPulseData;

typedef struct  
{
	short channel;
	short daValue;
	short fifo;
}TBufDaData;

typedef struct  
{
	short channel;
	short fifo;
	short source;
	double laserPower;
	double ratio;
	double minPower;
	double maxPower;
	short tableId;
}TBufLaserData;

typedef struct  
{
	short axis;
	double deltaPos;
	short fifo;
	short smoothFlag;
	short accPercent;
	short decPercent;
}TBufGearData;

typedef struct  
{
	short axis;
	double pos;
	double vel;
	double acc;
	short modal;
	short fifo;
}TBufMoveData;

typedef struct
{
	long segNum;
	short fifo;
}TBufSegNumData;

typedef struct  
{
	short crdAxis;
	short masterIndex;
	short masterType;
	short fifo;
} TBufFollowMasterData;

typedef struct  
{
	long masterPos;
	long pad;
	short fifo;
} TBufFollowEventCrossData;

typedef struct  
{
	short triggerIndex;
	long triggerOffset;
	long pad;
	short fifo;
} TBufFollowEventTriggerData;

typedef struct
{
	long masterSegment;
	long slaveSegment;
	long masterFrameWidth;
	short fifo;
}TBufFollowStartData;

typedef struct
{
	long width;
	short fifo;

}TBufFollowNextData;

typedef struct
{
	double vel;
	double acc;
	short smoothPercent;
	short fifo;
}TBufFollowReturnData;

typedef struct  
{
	short index;
	short enable;
	short fifo;
}TBufSmartCutterData;

typedef struct
{
	short eventIndex;
	short count;
	short fifo;
}TBufEventData;

typedef struct
{
	short index;
	short fifo;
}TBufPosCompareData;

typedef struct
{
	short index;
	short fifo;
	long syncPos;
}TBufPosComparePsoPrmData;

typedef struct
{
	short channel;
	short hsio0;
	short hsio1;
	short fifo;
}TBufComparePortData;

typedef struct
{
	short channel;
	short enable;
	short fifo;
}TBufLaserDisEnable;

typedef struct
{
	short channel;
	short index;
	short fifo;
}TBufLaserDisTable;

typedef struct  
{
	short fifo;
	short chn;
	short compareFifo;
	long px;
	long py;
}TBuf2DCompareData;

typedef struct  
{
	short fifo;
	short chn;
	short compareFifo;
	double px;
	double py;
}TBuf2DCompareDataEx;

typedef struct  
{
	long mask;
	long option;
	short fifo;
}TBufStopData;

typedef struct
{
	short moveAxis;
	double vel;
	double acc;
	short modal;
	short fifo;
}TBufMoveJogData;

typedef struct  
{
	short fifo;
	short chn;
	short level;
	short outputType;
	short time;
}TBuf2DComparePulse;

typedef struct
{
	short index;
	long longVar;
	double doubleVar;
	long overTime;
	short flagMode;
	long segNum;
	short fifo;
}TBufWaitVarData;

//Look ahead Ex
GT_API GT_SetupLookAheadCrd(short crd,EMachineMode machineMode);
GT_API GT_SetFollowAxisParaLa(short crd, int *pAxisLimitMode, double *pVmax, double *pAmax, double *pDVmax);
GT_API GT_SetVelDefineModeLa(short crd,EVelSettingDef velDefMode);
GT_API GT_SetAxisLimitModeLa(short crd,int *pAxisLimitMode);
GT_API GT_SetWorkLimitModeLa(short crd,EWorkLimitMode workLimitMode);
GT_API GT_SetAxisVelValidModeLa(short crd,int velValidAxis);
GT_API GT_SetVelSmoothMode(int crd,int smoothMode);
GT_API GT_InitLookAheadEx(short crd,TLookAheadParameter *pLookAheadPara,short fifo=0, short motionMode =0,TPreStartPos *pPreStartPos =NULL);
GT_API GT_PrintLACmdLa(short crd,int printFlag, int clearFile);
GT_API GT_InitLookAheadPara(short crd,long lookAheadNum,double time,double radiusRatio,double scale,short fifo=0);
GT_API GT_SetAxisFollowModeLa(short crd,int *pFollowMode);
GT_API GT_SetBlendingParaLa(short crd,short paraType,double para,double minAngle,double maxAngle);

GT_API GT_LnXYEx(short crd,double x,double y,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_LnXYG0Ex(short crd,double x,double y,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_LnXYZEx(short crd,double x,double y,double z,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_LnXYZG0Ex(short crd,double x,double y,double z,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_LnXYZAEx(short crd,double x,double y,double z,double a,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_LnXYZAG0Ex(short crd,double x,double y,double z,double a,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_LnXYZACEx(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_LnXYZACG0Ex(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);

GT_API GT_LnXYZACUVWEx(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_LnXYZACUVWG0Ex(short crd,double *pPos,short posMask,double synVel,double synAcc,long segNum,short override2,short fifo);

GT_API GT_ArcXYREx(short crd,double x,double y,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcYZREx(short crd,double y,double z,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcZXREx(short crd,double z,double x,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_ArcXYCEx(short crd,double x,double y,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcYZCEx(short crd,double y,double z,double yCenter,double zCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_ArcZXCEx(short crd,double z,double x,double zCenter,double xCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_ArcXYZEx(short crd,double x,double y,double z,double interX,double interY,double interZ,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_HelixXYRZEx(short crd,double x,double y,double z,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);
GT_API GT_HelixXYCZEx(short crd,double x,double y,double z,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo=0);

GT_API GT_BufDelayEx(short crd,unsigned short delayTime,short fifo=0);
GT_API GT_BufGearEx(short crd,short gearAxis,double deltaPos,short fifo=0);
GT_API GT_BufGearPercentEx(short crd,short gearAxis,double deltaPos,short accPercent,short decPercent,short fifo=0);
GT_API GT_BufMoveEx(short crd,short moveAxis,double pos,double vel,double acc,short modal,short fifo=0);
GT_API GT_BufIOEx(short crd,unsigned short doType,unsigned short doMask,unsigned short doValue,short fifo=0);
GT_API GT_BufDAEx(short crd,short chn,short daValue,short fifo=0);
GT_API GT_SetUserSegNumEx(short crd,long segNum,short fifo=0);

//Add By lin.ga 20150330
GT_API GT_BufLaserOnEx(short crd,short fifo=0,short channel=0);
GT_API GT_BufLaserOffEx(short crd,short fifo=0,short channel=0);
GT_API GT_BufLaserPrfCmdEx(short crd,double laserPower,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowRatioEx(short crd,double ratio,double minPower,double maxPower,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowModeEx(short crd,short source =0,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowSplineEx(short crd,short tableId,double minPower,double maxPower,short fifo=0,short channel=0);
GT_API GT_BufLaserFollowOffEx(short crd,short fifo=0,short channel=0);

GT_API GT_BufDisableDoBitPulseEx(short crd,short doType,short doIndex,short fifo);
GT_API GT_BufEnableDoBitPulseEx(short crd,short doType,short doIndex,unsigned short highLevelTime,unsigned short lowLevelTime,long pulseNum,short firstLevel,short fifo);

GT_API GT_CrdDataEx(short crd,TCrdData *pCrdData,short fifo=0);
GT_API GT_GetLookAheadSegCountEx(short crd,long *pSegCount,short fifo=0);

GT_API GT_GetMotionTimeEx(short crd,double *pTime,short fifo);

GT_API GT_BufFollowMasterEx(short crd,TBufFollowMaster *pBufFollowMaster,short fifo=0);
GT_API GT_BufFollowEventCrossEx(short crd,TBufFollowEventCross *pEventCross,short fifo=0);
GT_API GT_BufFollowEventTriggerEx(short crd,TBufFollowEventTrigger *pEventTrigger,short fifo=0);
GT_API GT_BufFollowStartEx(short crd,long masterSegment,long slaveSegment,long masterFrameWidth,short fifo=0);
GT_API GT_BufFollowNextEx(short crd,long width,short fifo=0);
GT_API GT_BufFollowReturnEx(short crd,double vel,double acc,short smoothPercent,short fifo=0);

GT_API GT_BufSmartCutterOnEx(short crd,short smartCutterIndex,short fifo=0);
GT_API GT_BufSmartCutterOffEx(short crd,short smartCutterIndex,short fifo=0);

GT_API GT_BufEventOnEx(short crd,short eventIndex,short count,short fifo);
GT_API GT_BufEventOffEx(short crd,short eventIndex,short count,short fifo);
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////lin.ga 20150701 Add PathOpt Fuction/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
GT_API GT_SetPathOptPara(short crd, TOptimizeParamUser *optPrm,short fifo);
GT_API GT_GetPathOptErrorInfo(short crd,TErrorInfo *errorInfo,short fifo);


//5 Axis
GT_API GT_CrdRTCPOn(short crd,short fifo=0);
GT_API GT_CrdRTCPOff(short crd,short fifo=0);
GT_API GT_UpdateMachineBuildingFile(short crd, int update);
GT_API GT_InitialMachineBuilding(short crd, char *pMachineCfgFileName,double *machineCoordCenter, double *workCoordCenter, double toolLength);
GT_API GT_SetRotationAxisRange(short crd, TRotationAxisRange *pRotationAxisRange);
GT_API GT_SetInverseSolutionSelectPara(short crd, EGroupSelect groupSelect, int priorAxisSet);
GT_API GT_MachineForwardTrans(short crd, double *pMachinePos, double *pWorkPos);
GT_API GT_MachineRTCPTrans(short crd, double *pInputPos, double *pMachinePos, double *pWorkPos);
GT_API GT_MachineTransformation(short crd, int posType, double *pPrePos, double *pPos, int *pPosNum, TPos **pReturnPos);
GT_API GT_MachineTransformationToLabView(short crd, int posType, double *pPrePos, double *pPos, int *pPosNum,double *machinePos,double *workPos);
GT_API GT_SetCompToolLength(short crd, double compToolLength);
GT_API GT_SetCompWorkCoordOffset(short crd, double *pCompWorkOffset);
GT_API GT_SetNonlinearErrorControl(short crd, int enable, double nonlinearError);
GT_API GT_EnableDiscreateArc(short crd,short enable,double arcError);
GT_API GT_StartXYCMachineMode(short crd, short dir, double contactAngle, double rotationAngle, double *pTranslation, double aValue,short fifo);// Start XYC mode
GT_API GT_EndXYCMachineMode(short crd,short fifo);                                                   // Exit XYC mode
GT_API GT_SetWorkCrdPlane(short crd, short enable, double *pNormVector, double *pPoint,short fifo);    // Set glass processing plane parameters
GT_API GT_SetStartPointProcessMode(short crd, short enable, double z,short fifo);                      // Preprocess the first XYC data segment
GT_API GT_InitialMachineBuildingEx(short crd, char *pMachineCfgFileName,double *machineCoordCenter, double *workCoordCenter, double toolLength);
GT_API GT_SetWorkCrdLaserFollowMode(short crd,short enbale,short fifo,short chn);
GT_API GT_ArcXYRACEx(short crd,double x,double y,double a,double c,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcXYCACEx(short crd,double x,double y,double a,double c,double xCenter,double yCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcXYZACEx(short crd,double x,double y,double z,double a,double c,double interX,double interY,double interZ,double interA,double interC,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcYZRACEx(short crd,double y,double z,double a,double c,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcYZCACEx(short crd,double y,double z,double a,double c,double yCenter,double zCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcZXRACEx(short crd,double z,double x,double a,double c,double radius,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);
GT_API GT_ArcZXCACEx(short crd,double z,double x,double a,double c,double zCenter,double xCenter,short circleDir,double synVel,double synAcc,long segNum,short override2,short fifo);

GT_API GT_BufPosCompareStartEx(short crd,short fifo,short index);
GT_API GT_BufPosCompareStopEx(short crd,short fifo,short index);
GT_API GT_BufPosComparePsoPrmEx(short crd,short index,TPosComparePsoPrm *pPrm,short fifo);
GT_API GT_BufSetComparePortEx(short crd,short fifo,short channel,short hsio0,short hsio1);

GT_API GT_BufSetLaserDisEnableEx(short crd,short fifo,short channel,short enable);
GT_API GT_BufSelectLaserDisTableEx(short crd,short fifo,short channel,short index);

GT_API GT_SetFollowAxisProcessMode(short crd,short mode);

GT_API GT_Buf2DCompareDataEx(short crd,short fifo,short chn,T2DCompareDataEx data,short compareFifo);

GT_API GT_BufLaserFollowTableEx(short crd,short tableId,double minPower,double maxPower,short fifo=0,short channel=0);

GT_API GT_BufStopEx(short crd,long mask,long option,short fifo=0);
GT_API GT_BufMoveJogEx(short crd,short moveAxis,double vel,double acc,short modal,short fifo=0);

GT_API GT_Buf2DComparePulseEx(short crd,short fifo,short chn,short level,short outputType,short time);

GT_API GT_BufWaitLongVarEx(short crd,short index,long value,long overTime,short flagMode,long segNum,short fifo);

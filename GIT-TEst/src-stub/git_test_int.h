/**
********************************************************************************
* @file     git_test_int.h
* @author   Bachmann electronic GmbH
*
* @brief    This file contains all definitions and declarations,
*           which are global within the SW-module.
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH 2015
*******************************************************************************/

/* Avoid problems with multiple including */
#ifndef GIT_TEST_INT__H
#define GIT_TEST_INT__H

#include "..\src-gen\git_test_pi.h"

/* Defines: SMI server */
#define GIT_TEST_MINVERS     2        /* min. version number */
#define GIT_TEST_MAXVERS     2        /* max. version number */
#define GIT_TEST_PROTVERS    2        /* Version number */

/* Structure for module base configuration values */
typedef struct GIT_TEST_BASE_PARMS
{
    CHAR    AppName[M_MODNAMELEN_A];    /* Instance name of module */
    CHAR    CfgFileName[M_PATHLEN_A];   /* Path/Name of config file, NULL = MCONFIG.INI */
    SINT32  CfgLineNbr;                 /* Start line number in mconfig file */
    UINT32  DefaultPriority;            /* Default priority for all worker tasks */
    SINT32  *pDebugMode;                /* Debug mode from mconfig parameters */
} GIT_TEST_BASE_PARMS;

extern GIT_TEST_BASE_PARMS git_test_BaseParams;

/* Logging macros (single line, so that it does not need parentheses in the code */
#define LOG_I(Level, FuncName, Text, Args...) do{ if(git_test_DebugMode >= Level) { log_Info("%s: %s: " Text, "git_test", FuncName, ## Args); }}while(0)
#define LOG_W(Level, FuncName, Text, Args...) do{ if(git_test_DebugMode >= Level) { log_Wrn ("%s: %s: " Text, "git_test", FuncName, ## Args); }}while(0)
#define LOG_E(Level, FuncName, Text, Args...) do{ if(git_test_DebugMode >= Level) { log_Err ("%s: %s: " Text, "git_test", FuncName, ## Args); }}while(0)
#define LOG_U(Level, FuncName, Text, Args...) do{ if(git_test_DebugMode >= Level) { log_User("%s: %s: " Text, "git_test", FuncName, ## Args); }}while(0)

typedef struct CYCLIC_CFG
{
    UINT32  CycleTime;                  /* cycle time in ticks or syncs */
    REAL32  CycleTime_ms;               /* cycle time for this task in ms */
    UINT32  NextCycleStart;             /* tick/sync counter for next cycle start */
    UINT32  PrevCycleStart;             /* tick/sync counter for next cycle start */
} CYCLIC_CFG;

typedef struct INTERRUPT_CFG
{
    SINT32  SyncSessionId;              /* session id in case of using sync */
    UINT32  SyncEdge;                   /* sync edge selection */
    UINT32  SyncCounter;                /* number of sync interrupts, which have to occur until the ISR is called */
} SYNC_CFG;

/* Structure for task settings and actual data */
typedef struct TASK_PROPERTIES
{
    /* set values, to be specified */
    CHAR    Name[M_TSKNAMELEN_A];       /* unique visible name for task */
    CHAR    CfgGroup[PF_KEYLEN_A];      /* configuration group name in mconfig */
    VOIDFUNCPTR pMainFunc;              /* function pointer to main function of this task */
    UINT32  Priority;                   /* priority for this task */
    UINT32  WDogRatio;                  /* WDogTime = CycleTime * WDogMultiple */
    UINT32  StackSize;                  /* stack size of this task in bytes */
    UINT32  UseFPU;                     /* this task uses the FPU */
    /* actual data, calculated by application */
    SINT32  TaskId;                     /* id returned by task spawn */
    UINT32  WdogId;                     /* watchdog id returned by create wdog */
    SINT32  UnitsToWait;                /* number of ticks/syncs to wait (delay) */
    SEM_ID  CycleSema;                  /* semaphore for cycle timing */
    UINT32  Quit;                       /* task deinit is requested */
    UINT32  NbOfCycleBacklogs;          /* total nb of cycles within a backlog */
    UINT32  NbOfSkippedCycles;          /* total nb of cycles skipped due to backlog */
    UINT32  TimeBase;                   /* selection of time base */
    CYCLIC_CFG *pCyclicCfg;             /* information about cyclic-configuration, NULL if not used */
    SYNC_CFG *pSyncCfg;                 /* information about interrupt-configuration, NULL if not used */
    IN_VARS inVars;                     /* process image input data */
    OUT_VARS outVars;                   /* process image output data*/
} TASK_PROPERTIES;

/* specifies TimeBase in TASK_PROPERTIES */
enum taskTimeBase {TIME_BASE_CYCLIC, TIME_BASE_SYNC, TIME_BASE_EVENT, TIME_BASE_ERROR, TIME_BASE_SYNC_CALCULATED};

/* SVI parameter function declarations */
typedef SINT32(*SVIFKPTSTART) (SVI_VAR * pVar, UINT32 UserParam);
typedef void(*SVIFKPTEND) (SVI_VAR * pVar, UINT32 UserParam);

/* Settings for a single global SVI variable */
typedef struct SVI_GLOBVAR
{
    CHAR                 *VarName;     /* Visible name of exported variable */
    UINT32               Format;       /* Format and access type, use defines SVI_F_git_test */
    UINT32               Size;         /* Size of exported variable in bytes */
    void                 *pVar;        /* Pointer to exported variable */
    UINT32               Mode;         /* Mode of SVI variable for svi_AddGlobVar e.g. SVI_HM_NAME_ALLOC */
    UINT32               UserParam;    /* User parameter for pSviStart and pSviEnd */
    SVIFKPTSTART         pSviStart;    /* Function pointer to lock the access to the variable */
    SVIFKPTEND           pSviEnd;      /* Function pointer to release the lock function */
    SINT32               VarHandle;    /* Handle of the SVI variable (returned by svi_AddGlob/VirtVar()) */
    LST_ID              *pOutChanLst;  /* Pointer to list of output channels */
} SVI_GLOBVAR;

typedef struct GLOBINVAR
{
    CHAR                 *VarName;     /* Visible name of exported variable */
    IN_CHAN              InChan;       /* input channel */
} GLOBINVAR;

/* Settings for a single virtual SVI variable */
typedef struct SVI_VIRTVAR
{
    CHAR   *VarName;                    /* Visible name of exported variable */
    UINT32  Format;                     /* Format and access type, use defines SVI_F_git_test */
    UINT32  Size;                       /* Displayed size in bytes */
    UINT32  Rpar1;                      /* 2 Parameters will be passed with the read function */
    UINT32  Rpar2;
    UINT32  Wpar1;                      /* 2 Parameters will be passed with the write function */
    UINT32  Wpar2;
    SINT32(*ReadFunction) ();           /* Pointer to read-access function */
    SINT32(*WriteFunction) ();          /* Pointer to write-access function */
} SVI_VIRTVAR;

/* Parameters for application specific SMI reply functions */
typedef struct
{
    SMI_MSG *pMsg;
    UINT32  IpAddr;
    UINT32  SessionId;
    UINT32  ReplySize;
    UINT32  UsrMgrReq;
    CHAR    SmiReplyData[SMI_DATALEN];
    CHAR    SmiMsgName[32];
}APP_SMI_R;

/*--- Variables ---*/

/* Variable definitions: general */
extern SMI_ID *git_test_pSmiId;       /* Id for standard module interface */
extern UINT32 git_test_ModState;      /* Module state */
extern SEM_ID git_test_StateSema;     /* Semaphore for halting tasks */
extern CHAR git_test_Version[M_VERSTRGLEN_A]; /* Module version string */

/* Function pointer to application specific smi server */
extern  SINT32(*git_test_fpAppSmiSvr) (SMI_MSG * pMsg, UINT32 SessionId);

/* Variable definitions: module parameters */
extern SINT32 git_test_DebugMode;
extern CHAR8 git_test_ModuleInfoDesc[SMI_DESCLEN_A];

/* Variable definitions: SVI server */
extern UINT32 git_test_SviHandle;

/* Variable definitions: Application specific SMI server */
extern SINT32 git_test_AppSmiSvr(SMI_MSG * pMsg, UINT32 SessionId);

/* Functions: system global, defined in git_test_app.c */
extern SINT32 git_test_AppEOI(void);
extern void git_test_AppDeinit(void);
extern SINT32 git_test_CfgRead(void);


#endif /* Avoid problems with multiple include */

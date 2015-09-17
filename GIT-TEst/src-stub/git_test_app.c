/**
********************************************************************************
* @file     git_test_app.c
* @author   Bachmann electronic GmbH
*
* @brief    This file contains the application algorithms
*           and the application specific SVI interface.
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH 2015
*******************************************************************************/

/* VxWorks includes */
#include <vxWorks.h>
#include <taskLib.h>
#include <tickLib.h>
#include <intLib.h>
#include <semLib.h>
#include <sysLib.h>
#include <inetLib.h>
#include <string.h>
#include <stdio.h>
#include <symLib.h>
#include <sysSymTbl.h>

/* MSys includes */
#include <mtypes.h>
#include <msys_e.h>
#include <mio.h>
#include <mio_e.h>
#include <res_e.h>
#include <svi_e.h>
#include <log_e.h>
#include <prof_e.h>
#include <lst_e.h>

/* Project includes */
#include "../src-gen/git_test_pi_int.h"
#include "../src-gen/git_test_direct_int.h"
#include "../src-gen/git_test_config.h"
#include "git_test_e.h"
#include "git_test_int.h"
#include "git_test_control.h"



/* Functions: administration, to be called from outside this file */
SINT32  git_test_AppEOI(void);
void    git_test_AppDeinit(void);
SINT32  git_test_CfgRead(void);
void    git_test_IsrSemGive(UINT32 UserPara, UINT32 Type);

/* Functions: task administration, being called only within this file */
MLOCAL SINT32 Task_CreateAll(void);
MLOCAL void Task_DeleteAll(void);
MLOCAL SINT32 Task_CfgRead(void);
MLOCAL SINT32 Task_InitTiming(TASK_PROPERTIES *pTaskData);
MLOCAL SINT32 Task_InitTiming_Tick(TASK_PROPERTIES *pTaskData);
MLOCAL SINT32 Task_InitTiming_Sync(TASK_PROPERTIES *pTaskData);
MLOCAL void Task_WaitCycle(TASK_PROPERTIES *pTaskData);

/* Functions: worker task "Control" */
MLOCAL void Control_Main(TASK_PROPERTIES *pTaskData);
MLOCAL void Control_CycleInit(void);
MLOCAL void Control_CycleStart(void);
MLOCAL void Control_Cycle(TASK_PROPERTIES *pTaskData);
MLOCAL void Control_CycleEnd(TASK_PROPERTIES *pTaskData);

/* Functions: SVI client */
MLOCAL SINT32 SviClnt_Init(void);
MLOCAL void SviClnt_Deinit(void);


/* Functions: handle incoming user specific SMI calls */
SINT32  git_test_AppSmiSvr(SMI_MSG *pMsg, UINT32 SessionId);

/* Global variables: SVI client */
MLOCAL  SINT32(**pSviLib) () = NULL;    /* Information about external SVI server */

/*
 * Global variables: Settings for application task
 * A reference to these settings must be registered in TaskList[], see below.
 * If no configuration group is being specified, all values must be set properly
 * in this initialization.
 */
MLOCAL TASK_PROPERTIES TaskProperties_aControl = {
    "",                 /* unique task name, maximum length 14 */
    "ControlTask",                      /* configuration group name */
    Control_Main,                       /* task entry function (function pointer) */
    0,                                  /* default task priority (->Task_CfgRead) */
    5,                                  /* default ratio of watchdog time / cycle time
                                         * (->Task_CfgRead) */
    10000,                              /* task stack size in bytes, standard size is 10000 */
    TRUE                                /* task uses floating point operations */
};

/*
 * Global variables: List of all application tasks
 * TaskList[] is being used for all task administration functions.
 */
MLOCAL TASK_PROPERTIES *TaskList[] = {
    &TaskProperties_aControl
};



/**
********************************************************************************
* @brief Main entry function of the aTask of the application.
*        The input parameter is passed by the task spawn call.
*
* @param[in]  pointer to task properties data structure
*******************************************************************************/
MLOCAL void Control_Main(TASK_PROPERTIES *pTaskData)
{
    /* Initialization upon task entry */
    Control_CycleInit();

    /*
     * This loop is executed endlessly
     * as long as there is no request to quit the task
     */
    while (!pTaskData->Quit)
    {
        /* cycle start administration */
        Control_CycleStart();

        /* operational code */
        Control_Cycle(pTaskData);

        /* cycle end administration */
        Control_CycleEnd(pTaskData);
    }
}

/**
********************************************************************************
* @brief Administration code to be called once before first cycle start.
*******************************************************************************/
MLOCAL void Control_CycleInit(void)
{

    /* TODO: add what is necessary before cyclic operation starts */

}

/**
********************************************************************************
* @brief Administration code to be called once at each task cycle start.
*******************************************************************************/
MLOCAL void Control_CycleStart(void)
{

    /* TODO: add what is necessary at each cycle start */
    git_test_pi_read();

}

/**
********************************************************************************
* @brief Cyclic application code.
*******************************************************************************/
MLOCAL void Control_Cycle(TASK_PROPERTIES *pTaskData)
{


    //TODO: add parameters
    git_test_control_cycle(&pTaskData->inVars, &pTaskData->outVars);


}

/**
********************************************************************************
* @brief Administration code to be called at each task cycle end
*
* @param[in]  pointer to task properties data structure
*******************************************************************************/
MLOCAL void Control_CycleEnd(TASK_PROPERTIES *pTaskData)
{

    /* TODO: add what is to be called at each cycle end */
    git_test_pi_write(&pTaskData->outVars);

    /*
     * This is the very end of the cycle
     * Delay task in order to match desired cycle time
     */
    Task_WaitCycle(pTaskData);
}

/**
********************************************************************************
* @brief Performs the second phase of the module initialization.
* 		  Called at "End Of Init" by the bTask.
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
SINT32 git_test_AppEOI(void)
{
    /* do while(0), to be left as soon as there is an error */
    do
    {
        /* TODO: set module info string, maximum length is SMI_DESCLEN_A */
        sprintf(git_test_ModuleInfoDesc, "TODO: set application specific module info string");

        /* Initialize variables */
        if (git_test_direct_init() < 0)
        {
            break;
        }

        /* Initialize SVI access to variables of other software modules */
        if (SviClnt_Init() < 0)
        {
            break;
        }

        /* Start all application tasks listed in TaskList */
        if (Task_CreateAll() < 0)
        {
            break;
        }

        /* At this point, all init actions are done successfully */
        return (OK);
    }
    while (0);

    /*
     * At this point, an init action returned an error.
     * The application code is being de-initialized.
     */
    git_test_AppDeinit();
    return (ERROR);
}

/**
********************************************************************************
* @brief Frees all resources of the application
*        Called at De-Init of the module by the bTask.
*        The function does not quit on an error.
*******************************************************************************/
void git_test_AppDeinit(void)
{

    /* TODO: Free all resources which have been allocated by the application */

    git_test_direct_deinit();

    git_test_pi_deinit();

    /* Delete all application tasks listed in TaskList */
    Task_DeleteAll();

    /* Delete all SVI client access data */
    SviClnt_Deinit();

}

/**
********************************************************************************
* @brief Calls all configuration read functions of the application
*        Being called at module init by the bTask.
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
SINT32 git_test_CfgRead(void)
{
    SINT32  ret;

    /* Read component specific configuration */
    ret = git_test_config_read();
    if (ret < 0)
    {
        return (ret);
    }

    /* Read task configuration settings from configuration file (BaseParam.CfgFileName) */
    ret = Task_CfgRead();
    if (ret < 0)
    {
        return (ret);
    }

    return (OK);
}

/**
********************************************************************************
* @brief Reads the settings from configuration file mconfig
*        for all tasks registered in TaskList[].
*        The task name in TaskList[] is being used as configuration group name.
*        The initialization values in TaskList[] are being used as default values.
*        For general configuration data, git_test_CfgParams is being used.
*        Being called by git_test_CfgRead.
*        All parameters are stored in the task properties data structure.
*        All parameters are being treated as optional.
*        There is no limitation checking of the parameters, the limits are being
*        specified in the cru and checked by the configurator.
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_CfgRead(void)
{
    static const CHAR *pFunc = __func__;
    UINT32  idx;
    UINT32  NbOfTasks = sizeof(TaskList) / sizeof(TASK_PROPERTIES *);
    SINT32  ret;
    CHAR    *section;
    CHAR    *group;
    CHAR    key[PF_KEYLEN_A];
    CHAR    TmpStrg[32] = {0};
    UINT32  Error = FALSE;
    SINT32  TmpVal;

    /* section name is the application name; same for all tasks */
    section = git_test_BaseParams.AppName;

    /* For all application tasks listed in TaskList */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (!TaskList[idx])
        {
            LOG_E(0, pFunc, "Invalid task properties pointer in task list entry #%d!", idx);
            Error = TRUE;
            continue;
        }

        /* group name is specified in the task properties */
        group = TaskList[idx]->CfgGroup;

        /* if no group name has been specified: skip configuration reading for this task */
        if (strlen(group) < 1)
        {
            LOG_I(0, pFunc, "Could not find task configuration for task '%s' in mconfig ",
                  TaskList[idx]->Name);
            continue;
        }

        /* First of all, get TaskMode.
         * TaskMode can be TIME_BASE_CYCLIC, TIME_BASE_SYNC, TIME_BASE_EVENT, TIME_BASE_ERROR
         */
        sprintf(key, "TaskMode");
        ret = pf_GetStrg(section, group, key, "", TmpStrg, sizeof(TmpStrg),
                git_test_BaseParams.CfgLineNbr, git_test_BaseParams.CfgFileName);

        /* Using strcmp is safe because string literals are guaranteed null terminated. */
        if(strcmp(TmpStrg, "Cyclic") == 0)
        {
            TaskList[idx]->TimeBase = TIME_BASE_CYCLIC;
        }
        else if(strcmp(TmpStrg, "Sync") == 0)
        {
            TaskList[idx]->TimeBase = TIME_BASE_SYNC;
        }
        else
        {
            LOG_E(0, pFunc, "Bad task-configuration: %s not allowed for '%s'", TmpStrg, key);
            return MIO_ER_BADCONF;
        }

        if(TaskList[idx]->TimeBase == TIME_BASE_CYCLIC)
        {
            /* Alloc memory for cyclic configuration */
            TaskList[idx]->pCyclicCfg = sys_MemXAlloc(sizeof(CYCLIC_CFG));
            memset(TaskList[idx]->pCyclicCfg, 0, sizeof(CYCLIC_CFG));

            sprintf(key, "CycleTime");
            ret = pf_GetStrg(section, group, key, "", TmpStrg, sizeof(TmpStrg),
                    git_test_BaseParams.CfgLineNbr, git_test_BaseParams.CfgFileName);
            /* keyword has been found */
            if (ret >= 0 && strlen(TmpStrg) > 0)
            {
                *((REAL32 *) & TaskList[idx]->pCyclicCfg->CycleTime_ms) = atof(TmpStrg);
            }
            /* keyword has not been found */
            else
            {
                LOG_W(0, pFunc, "Missing configuration parameter '[%s](%s)%s'", section, group, key);
                return MIO_ER_BADCONF;
            }

        }
        else
        {
            LOG_E(0, pFunc, "Bad task-configuration: TimeBase unknown");
            return MIO_ER_BADCONF;
        }

        /*
         * Read the desired value for the task priority.
         * If the keyword has not been found, the initialization value remains
         * in the task properties.
         * As an additional fall back, the priority in the base parms will be used.
         */
        sprintf(key, "Priority");
        ret = pf_GetInt(section, group, key, TaskList[idx]->Priority, &TmpVal,
                git_test_BaseParams.CfgLineNbr, git_test_BaseParams.CfgFileName);
        /* keyword has been found */
        if (ret >= 0 && TmpVal > 0)
        {
            TaskList[idx]->Priority = TmpVal;
        }
        /* keyword has not been found */
        else
        {
            LOG_W(0, pFunc, "Missing configuration parameter '[%s](%s)%s'", section, group, key);
            return MIO_ER_BADCONF;
        }
    }

    /* Evaluate overall error flag */
    if (Error)
    {
        return (ERROR);
    }
    else
    {
        return (OK);
    }
}

/**
********************************************************************************
* @brief Starts all tasks which are registered in the global task list
*        - task watchdog is being created if specified
*        - priority is being checked and corrected if necessary
*        - semaphore for cycle timing is being created
*        - sync session is being started if necessary
*        - sync ISR is being attached if necessary
*        If there is an error creating a task, no further tasks will be started.
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_CreateAll(void)
{
    UINT32  idx;
    UINT8   TaskName[M_TSKNAMELEN_A];
    UINT32  NbOfTasks = sizeof(TaskList) / sizeof(TASK_PROPERTIES *);
    UINT32  TaskOptions;
    UINT32  wdogtime_us;
    static const CHAR *pFunc = __FUNCTION__;

    /* For all application tasks listed in TaskList */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (!TaskList[idx])
        {
            LOG_E(0, pFunc, "Invalid task properties pointer!");
            return (ERROR);
        }

        /* Initialize process image */
        if (git_test_pi_init(&TaskList[idx]->inVars, &TaskList[idx]->outVars) < 0)
        {
            return (ERROR);
        }

        /* Initialize what is necessary */
        //TaskList[idx]->SyncSessionId = ERROR;
        TaskList[idx]->TaskId = ERROR;
        TaskList[idx]->WdogId = 0;
        TaskList[idx]->Quit = FALSE;

        /* Create software watchdog if required */
        if (TaskList[idx]->WDogRatio > 0)
        {
            /* check watchdog ratio, minimum useful value is 2 */
            if (TaskList[idx]->WDogRatio < 3)
            {
                TaskList[idx]->WDogRatio = 3;
                LOG_W(0, pFunc, "Watchdog ratio increased to 3!");
            }

            wdogtime_us = (UINT32)((TaskList[idx]->pCyclicCfg->CycleTime_ms * 1000) * TaskList[idx]->WDogRatio);
            TaskList[idx]->WdogId = sys_WdogCreate(git_test_BaseParams.AppName, wdogtime_us);
            if (TaskList[idx]->WdogId == 0)
            {
                LOG_E(0, pFunc, "Could not create watchdog!");
                return (ERROR);
            }
        }

        /* Create binary semaphore for cycle timing */
        TaskList[idx]->CycleSema = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
        if (!TaskList[idx]->CycleSema)
        {
            LOG_E(0, pFunc, "Could not create cycle timing semaphore for task '%s'!",
                  TaskList[idx]->Name);
            return (ERROR);
        }

        /* Initialize task cycle timing infrastructure */
        (void)Task_InitTiming(TaskList[idx]);

        /* In case the priority has not been properly set */
        if (TaskList[idx]->Priority == 0)
        {
            LOG_E(0, pFunc, "Invalid priority for task '%s'", TaskList[idx]->Name);
            return (ERROR);
        }

        /* make sure task name string is terminated */
        TaskList[idx]->Name[M_TSKNAMELEN] = 0;

        /* If no task name has been set: use application name and index */
        if (strlen(TaskList[idx]->Name) < 1)
        {
            sprintf(TaskList[idx]->Name, "a%s_%d", git_test_BaseParams.AppName, idx + 1);
        }

        sprintf(TaskName, "%s", TaskList[idx]->Name);

        /* Task options */
        TaskOptions = 0;
        if (TaskList[idx]->UseFPU)
        {
            TaskOptions |= VX_FP_TASK;
        }

        /* Spawn task with properties set in task list */
        TaskList[idx]->TaskId = sys_TaskSpawn(git_test_BaseParams.AppName, TaskName,
                                              TaskList[idx]->Priority, TaskOptions,
                                              TaskList[idx]->StackSize,
                                              (FUNCPTR)TaskList[idx]->pMainFunc, TaskList[idx]);

        /* Check if task has been created successfully */
        if (TaskList[idx]->TaskId == ERROR)
        {
            LOG_E(0, pFunc, "Error in sys_TaskSpawn for task '%s'!", TaskName);
            return (ERROR);
        }
    }

    /* At this point, all tasks have been started successfully */
    return (OK);
}

/**
********************************************************************************
* @brief Deletes all tasks which are registered in the global task list
*        Undo for all operations in Task_CreateAll
*        The function will not be left upon an error.
*******************************************************************************/
MLOCAL void Task_DeleteAll(void)
{
    UINT32  idx;
    UINT32  NbOfTasks = sizeof(TaskList) / sizeof(TASK_PROPERTIES *);
    UINT32  RequestTime;
    static const CHAR *pFunc = __FUNCTION__;

    /*
     * Delete software watchdog if present
     * This must be done first, because tasks will end their cyclic operation
     * and thus won't trigger their watchdogs any more.
     */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (TaskList[idx]->WdogId)
        {
            sys_WdogDelete(TaskList[idx]->WdogId);
        }
    }

    /*
     * Set quit request for all existing tasks
     * This should make tasks complete their cycle and quit operation.
     */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        TaskList[idx]->Quit = TRUE;
    }

    /*
     * Give all cycle semaphores of listed tasks
     * This wakes up all tasks and thus speeds up the completion.
     */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if (TaskList[idx]->CycleSema)
        {
            (void)semGive(TaskList[idx]->CycleSema);
        }
    }

    /* Take a time stamp for the timeout check */
    RequestTime = m_GetProcTime();

    /*
     * Wait for all tasks to quit their cycle.
     * Apply a timeout of 500ms
     * Wait one tick in case of missing task quit
     */
    for(;;)
    {
        UINT32  AllTasksQuitted = TRUE;

        /* allow one tick for tasks to complete */
        (void)taskDelay(1);

        /* Check if all tasks have terminated their cycles */
        for (idx = 0; idx < NbOfTasks; idx++)
        {
            AllTasksQuitted = AllTasksQuitted && (taskIdVerify(TaskList[idx]->TaskId) == ERROR);
        }

        /* If all tasks have terminated themselves */
        if (AllTasksQuitted)
        {
            if (git_test_DebugMode & APP_DBG_INFO1)
            {
                LOG_I(0, pFunc, "All tasks have terminated by themselves");
            }
            break;
        }
        /* If timeout waiting for task self termination is over */
        else if ((m_GetProcTime() - RequestTime) > 500000)
        {
            LOG_W(0, pFunc, "Timeout at waiting for tasks to terminate by themselves");
            break;
        }
        else
        {
            /* No change */
        }
    }

    /* Cleanup resources and delete all remaining tasks */
    for (idx = 0; idx < NbOfTasks; idx++)
    {
        if(TaskList[idx]->TimeBase == TIME_BASE_CYCLIC)
        {
            sys_MemXFree(TaskList[idx]->pCyclicCfg);
        }
        else if(TaskList[idx]->TimeBase == TIME_BASE_SYNC)
        {
//            /* Stop sync session if present and detach ISR */
//            if (TaskList[idx]->SyncSessionId >= 0)
//            {
//                LOG_I(0, pFunc, "Stopping sync session for task %s", TaskList[idx]->Name);
//                (void)mio_StopSyncSession(TaskList[idx]->SyncSessionId);
//            }
        }

        /* Delete semaphore for cycle timing */
        if (TaskList[idx]->CycleSema)
        {
            if (semDelete(TaskList[idx]->CycleSema) < 0)
            {
                LOG_W(0, pFunc, "Could not delete cycle semaphore of task %s!", TaskList[idx]->Name);
            }
            else
            {
            	TaskList[idx]->CycleSema = 0;
            }
        }

        /* Remove application tasks which still exist */
        if (taskIdVerify(TaskList[idx]->TaskId) == OK)
        {
            if (taskDelete(TaskList[idx]->TaskId) == ERROR)
            {
                LOG_E(0, pFunc, "Could not delete task %s!", TaskList[idx]->Name);
            }
            else
            {
                LOG_W(0, pFunc, "Task %s had to be deleted!", TaskList[idx]->Name);
            }

            TaskList[idx]->TaskId = ERROR;
        }
    }
}

/**
********************************************************************************
* @brief Initializes infrastructure for task timing
*
* @param[in]  pointer to task properties data structure
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_InitTiming(TASK_PROPERTIES *pTaskData)
{
    static const CHAR *pFunc = __FUNCTION__;

    if (!pTaskData)
    {
        LOG_E(0, pFunc, "Invalid input pointer!");
        return (ERROR);
    }

    /* independent of timing model */
    pTaskData->NbOfCycleBacklogs = 0;
    pTaskData->NbOfSkippedCycles = 0;

    /* depending on timing model */
    switch (pTaskData->TimeBase)
    {
        /* Tick based timing */
        case TIME_BASE_CYCLIC:
            return (Task_InitTiming_Tick(pTaskData));
        /* Sync based timing */
        case TIME_BASE_SYNC:

        case TIME_BASE_SYNC_CALCULATED:
            //pTaskData->SyncSessionId = ERROR;
            return (Task_InitTiming_Sync(pTaskData));
        /* Undefined */
        default:
            LOG_E(0, pFunc, "Unknown timing model!");
            return (ERROR);
    }
}

/**
********************************************************************************
* @brief Initializes infrastructure for task timing with tick timer
*
* @param[in]  pointer to task properties data structure
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 Task_InitTiming_Tick(TASK_PROPERTIES *pTaskData)
{
    REAL32  TmpReal;
    static const CHAR *pFunc = __FUNCTION__;

    if (!pTaskData)
    {
        LOG_E(0, pFunc, "Invalid input pointer!");
        return (ERROR);
    }

    /*
     * Calculate and check cycle time in ticks as integer value
     */
    TmpReal = ((pTaskData->pCyclicCfg->CycleTime_ms / 1000.0) * sysClkRateGet()) + 0.5;
    pTaskData->pCyclicCfg->CycleTime = (UINT32)TmpReal;

    /* If cycle time is less than a full tick */
    if (pTaskData->pCyclicCfg->CycleTime < 1)
    {
        pTaskData->pCyclicCfg->CycleTime = 1;
        LOG_W(0, pFunc, "Cycle time too small for tick rate %d, increased to 1 tick!",
              sysClkRateGet());
    }

    /* Take first cycle start time stamp */
    pTaskData->pCyclicCfg->PrevCycleStart = tickGet();

    /* Initialize cycle time grid */
    pTaskData->pCyclicCfg->NextCycleStart = tickGet() + pTaskData->pCyclicCfg->CycleTime;

    return (OK);
}

/**
********************************************************************************
* @brief Initializes infrastructure for task timing with sync event.
*
* @param[in]  pointer to task properties data structure
*
* @retval     = 0 .. OK
* @retval     < 0 .. Error
*******************************************************************************/
MLOCAL SINT32 Task_InitTiming_Sync(TASK_PROPERTIES *pTaskData)
{
    SINT32  ret;
    REAL32  SyncCycle_us, TmpReal;
    SYS_CPUINFO CpuInfo;
    static const CHAR *pFunc = __FUNCTION__;

    if (!pTaskData)
    {
        LOG_E(0, pFunc, "Invalid input pointer!");
        return (ERROR);
    }

    /* Start sync session for this module (multiple starts are possible) */
    pTaskData->pSyncCfg->SyncSessionId = mio_StartSyncSession(git_test_BaseParams.AppName);
    if (pTaskData->pSyncCfg->SyncSessionId < 0)
    {
        LOG_E(0, pFunc, "Could not start sync session for task '%s'!", pTaskData->Name);
        return (ERROR);
    }

    /* Get cpu info, contains sync timer settings (always returns OK) */
    (void)sys_GetCpuInfo(&CpuInfo);

    /* Calculate and check period time of sync timer */
    SyncCycle_us = CpuInfo.pExtCpuInfo->SyncHigh + CpuInfo.pExtCpuInfo->SyncLow;
    if ((SyncCycle_us == 0) || (CpuInfo.pExtCpuInfo->SyncHigh == 0)
        || (CpuInfo.pExtCpuInfo->SyncLow == 0))
    {
        LOG_E(0, pFunc, "System sync configuration invalid, can't use sync for task '%s'!",
              pTaskData->Name);
        return (ERROR);
    }

    /* Calculate multiple of specified task cycle time */
    TmpReal = ((pTaskData->pCyclicCfg->CycleTime_ms * 1000) / SyncCycle_us) + 0.5;
    pTaskData->pCyclicCfg->CycleTime = (UINT32)TmpReal;

    /* If cycle time is less than a full sync */
    if (pTaskData->pCyclicCfg->CycleTime < 1)
    {
        pTaskData->pCyclicCfg->CycleTime = 1;
        LOG_W(0, pFunc, "Cycle time too small for sync cycle %d us, increased to 1 sync!",
              SyncCycle_us);
    }

    /*
     * For application tasks,
     * MIO_SYNC_IN (falling edge of sync signal) is the normal option.
     */
    pTaskData->pSyncCfg->SyncEdge = MIO_SYNC_IN;

    /*
     * Attach semGive to sync event
     * -> semGive will be called according to the sync attach settings below.
     * -> semGive will give the semaphore pTaskData->CycleSema.
     * -> the task will be triggered as soon as this semaphore is given.
     */
    ret = mio_AttachSync(pTaskData->pSyncCfg->SyncSessionId,      /* from mio_StartSyncSession */
                         pTaskData->pSyncCfg->SyncEdge,   /* selection of sync edge */
                         pTaskData->pCyclicCfg->CycleTime,  /* number of sync cycles */
                         (VOIDFUNCPTR)git_test_IsrSemGive,      /* register semGive as ISR */
                         (UINT32)pTaskData->CycleSema);        /* semaphore id for semGive */
    if (ret < 0)
    {
        LOG_W(0, pFunc, "Could not attach to sync for task '%s'!", pTaskData->Name);
        if (pTaskData->pSyncCfg->SyncSessionId >= 0)
            (void) mio_StopSyncSession(pTaskData->pSyncCfg->SyncSessionId);
        pTaskData->pSyncCfg->SyncSessionId = ERROR;
        return (ERROR);
    }

    return (OK);
}

/**
********************************************************************************
* @brief Calls the function semGive() - give a semaphore.
*        Register semGive as ISR.
*
* @param[in]  UserPara  semaphore ID to give
*******************************************************************************/
void git_test_IsrSemGive(UINT32 UserPara, UINT32 Type)
{
    (void) semGive((SEM_ID)UserPara);
}



/**
********************************************************************************
* @brief Performs the necessary wait time for the specified cycle.
*        The wait time results from cycle time minus own run time.
*        NOTE: The time unit depends on the used time base (ticks or sync periods).
*
* @param[in]  pointer to task properties data structure
*******************************************************************************/
MLOCAL void Task_WaitCycle(TASK_PROPERTIES *pTaskData)
{
    UINT32  PrevCycleStart = 0;
    UINT32  NextCycleStart = 0;
    UINT32  CycleTime = 0;
    SINT32  TimeToWait = 0;
    UINT32  SkipNow = 0;
    UINT32  TimeNow = 0;
    UINT32  Backlog = 0;
    UINT32  MaxBacklog = 0;
    UINT32  CyclesSkipped = 0;

    /* Emergency behavior in case of missing task settings */
    if (!pTaskData)
    {
        LOG_E(0, "Task_WaitCycle", "Invalid input pointer, using alternative taskDelay(1000)");
        (void)taskDelay(1000);
        return;
    }

    /* Trigger software watchdog if existing */
    if (pTaskData->WdogId)
    {
        (void)sys_WdogTrigg(pTaskData->WdogId);
    }

    /*
     * Handle tick based cycle timing ("Time" unit is ticks)
     */
    if (pTaskData->TimeBase == TIME_BASE_CYCLIC)
    {
        /* Use local variables to make calculation as compressed as possible */
        PrevCycleStart = pTaskData->pCyclicCfg->PrevCycleStart;
        NextCycleStart = pTaskData->pCyclicCfg->NextCycleStart;
        CycleTime = pTaskData->pCyclicCfg->CycleTime;
        MaxBacklog = CycleTime * 2;

        /* Calculate the time to wait before the next cycle can start. */
        NextCycleStart = PrevCycleStart + CycleTime;
        PrevCycleStart = NextCycleStart;

        /*
         * As soon as the current time stamp has been taken,
         * the code until semTake processing should be kept as short as possible,
         * so that the probability of being interrupted is as small as possible.
         */
        TimeNow = tickGet();

        /* This is the amount of until the next scheduled cycle start. */
        TimeToWait = NextCycleStart - TimeNow;

        /* Limit wait time to minimum 1 */
        if (!TimeToWait)
        {
            TimeToWait++;
        }

        /*
         * As long as the next scheduled cycle start lies in the future,
         * the resulting wait time must be smaller than the cycle time.
         * If the resulting wait time is higher than the cycle time,
         * the next scheduled cycle start already lies in the past.
         * This means that there is a cycle backlog.
         * NOTE: since the wait time is an unsigned value, a negative difference
         * is being interpreted as a large positive value.
         */
        if (TimeToWait > CycleTime)
        {
            /* Calculate cycle backlog */
            Backlog = TimeNow - NextCycleStart;

            /* As long as the backlog is below the limit */
            if (Backlog <= MaxBacklog)
            {
                /* Try to catch, but still use a small task delay */
                TimeToWait = 1;
            }
            /* If the backlog is beyond the limit */
            else
            {
                /* Skip the backlog and recalculate next cycle start */
                SkipNow = (Backlog / CycleTime) + 1;
                NextCycleStart = NextCycleStart + (SkipNow * CycleTime);
                PrevCycleStart = NextCycleStart;
                TimeToWait = NextCycleStart - TimeNow;
                CyclesSkipped += SkipNow;
            }
        }
    }

    /*
     * Handle sync based cycle timing ("Time" unit is syncs)
     */
    else if (pTaskData->TimeBase == 1)
    {
        /*
         * In case of sync timing, it is assumed, that the sync interrupt
         * directly determines the cycle time.
         * It would also be possible to use a multiple of sync's as cycle time.
         * The logic necessary for that is not yet implemented.
         */
        TimeToWait = WAIT_FOREVER;
    }
    else
    {
        /* No change */
    }

    /* Register cycle end in system timing statistics */
    sys_CycleEnd();

    /*
     * Wait for the calculated number of time units
     * by taking the cycle semaphore with a calculated timeout
     */
    (void)semTake(pTaskData->CycleSema, TimeToWait);

    /*
     * Waiting for the cycle semaphore has now timed out in case of tick
     * or been given in case of sync.
     */

    /* Register cycle start in system timing statistics */
    sys_CycleStart();

    if (pTaskData->TimeBase == TIME_BASE_CYCLIC)
    {
        pTaskData->pCyclicCfg->PrevCycleStart = PrevCycleStart;
        pTaskData->pCyclicCfg->NextCycleStart = NextCycleStart;
    }
    /*
     * The above logic uses local variables in order to keep the processing short.
     * Some of these local variables must be rescued for the next call of this
     * function.
     */
    pTaskData->NbOfSkippedCycles += CyclesSkipped;
    if (Backlog)
    {
        pTaskData->NbOfCycleBacklogs++;
    }

    /*
     * Consideration of software module state
     * If the RcpStop call has been sent to the software module,
     * all cyclic tasks of this module shall be stopped.
     * If the software module receives the RpcStart call,
     * it will give the state semaphore, and all tasks will continue.
     */
    if ((git_test_ModState != RES_S_RUN) && !pTaskData->Quit)
    {
        /* Disable software watchdog if present */
        if (pTaskData->WdogId)
        {
            sys_WdogDisable(pTaskData->WdogId);
        }

        /*
         * semaphore will be given by SMI server with calls
         * RpcStart or RpcEndOfInit
         */
        (void)semTake(git_test_StateSema, WAIT_FOREVER);
    }
}

/**
********************************************************************************
* @brief Server function for all application specific SMI calls.
*        Contains all common call handling code.
*        This function is being called by the general SMI server task in
*        git_test_module.c.
*
* @param[in]  pMsg          pointer to data structure of incoming SMI message
* @param[in]  SessionId     User session identification
*
* @retval     = 0 .. Message has been accepted and processed
* @retval     < 0 .. Unknown message
*******************************************************************************/
SINT32 git_test_AppSmiSvr(SMI_MSG *pMsg, UINT32 SessionId)
{
    void   *pReply = 0;
    SINT32 *pReplyRetCode = 0;
    UINT32  CallDataOk;
    SINT32  RetVal;
    SINT32(*SmiReplyFunc) (APP_SMI_R * pSmiSrvReply) = 0;
    /* for local data exchange with reply function */
    APP_SMI_R SmiSrvReply;

    /*
     * pMsg->ProcRetCode contains the SMI message id.
     * Depending on the message ID, the reply function is being selected
     * by setting a function pointer.
     */
    switch (pMsg->ProcRetCode)
    {

        default:
            /* Unknown message */
            LOG_I(2, "git_test_AppSmiSvr", "Received unknown SMI call with id %d", pMsg->ProcRetCode);
            /* SMI message can not be processed */
            return (ERROR);
    }

    /*lint --e(527) Unreachable code */

    /* At this point, the individual reply function has been selected */

    /* Init parameters for specific reply functions and clear reply data */
    SmiSrvReply.pMsg = pMsg;
    SmiSrvReply.SessionId = SessionId;
    SmiSrvReply.ReplySize = 0;
    /*lint --e(545) Suspicious use of & */
    bzero((CHAR *) & SmiSrvReply.SmiReplyData, sizeof(SmiSrvReply.SmiReplyData));

    /* Call of specific reply function, returns OK if answer is ready to send */
    CallDataOk = (SmiReplyFunc(&SmiSrvReply) == OK);

    /* Init SMI reply data and send immediate reply in case of error */
    pReply = smi_MemAlloc(SmiSrvReply.ReplySize);
    if (pReply)
    {
        /* Copy content set by individual reply function to final reply data */
        /*lint --e(545) Suspicious use of & */
        memcpy(pReply, &SmiSrvReply.SmiReplyData, SmiSrvReply.ReplySize);
        /* The return code is always at the start of the reply data structure */
        pReplyRetCode = pReply;
    }
    else
    {
        /* Serious error, unconditional log message and according reply */
        LOG_E(0, "git_test_AppSmiSvr", "Not enough memory for SMI reply!");

        if (smi_SendReply(git_test_pSmiId, pMsg, SMI_E_ARGS, 0, 0) == ERROR)
        {
            LOG_E(0, "git_test_AppSmiSvr", "smi_SendReply failed!");
        }
        /* Message has been processed even though there was an error */
        return (OK);
    }

    /* Evaluate all processing steps */
    if (!CallDataOk)
    {
        *pReplyRetCode = SMI_E_FAILED;
        LOG_E(1, "git_test_AppSmiSvr", "SMI call data evaluation has failed!");
    }
    /* everything ok and request accepted */
    else
    {
        *pReplyRetCode = SMI_E_OK;
    }

    /* must be done before sending reply */
    smi_FreeData(pMsg);

    /* Send SMI reply */
    RetVal = smi_SendReply(git_test_pSmiId, pMsg, SMI_E_OK, pReply, SmiSrvReply.ReplySize);

    if (RetVal == ERROR)
    {
        LOG_E(1, "git_test_AppSmiSvr", "smi_SendReply failed!");
    }
    else
    {
        LOG_I(2, "git_test_AppSmiSvr", "Sent SMI reply: retcode = %x", RetVal);
    }

    /* No matter what result, message has been processed */
    return (OK);
}




/**
********************************************************************************
* @brief This function demonstrates the implementation of an SVI client.
*        It will connect to the server of another software module (RES)
*        and will get the address of one SVI variable. The variable is
*        accessed in AppMain(void)
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL SINT32 SviClnt_Init(void)
{
    /* Get library pointer to access SVI server of other software module. */
    pSviLib = svi_GetLib("RES");
    if (!pSviLib)
    {
        LOG_W(0, __func__, "Could not get SVI of module 'RES'!");
        return (ERROR);
    }


    return (OK);
}

/**
********************************************************************************
* @brief This function informs the RES that the function library of
*        the server (in this example the server is also RES) is no
*        longer required
*
* @retval     = 0 .. OK
* @retval     < 0 .. ERROR
*******************************************************************************/
MLOCAL void SviClnt_Deinit(void)
{
    if (pSviLib)
    {
        if (svi_UngetLib(pSviLib) < 0)
        {
            LOG_E(0, "__func__", "svi_UngetLib Error!");
        }
        pSviLib = NULL;
    }

}

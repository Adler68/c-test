/**
********************************************************************************
* @file     git_test_control.c
* @author   Bachmann electronic GmbH
* @version  $Revision: 3.90 $ $LastChangedBy: BE $
* @date     $LastChangeDate: 2013-06-10 11:00:00 $
*
* @brief    This file contains
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH 2013
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
#include "src-gen/git_test_direct.h"
#include "src-gen/git_test_svi.h"
#include "src-gen/git_test_pi.h"
#include "src-stub/git_test_control.h"

/**
********************************************************************************
* @brief Cyclic application function. Implement your business logic here.
*
* @param[in]  pInVars   process image in data structure
* @param[in]  pOutVars  process image out data structure
*******************************************************************************/
void git_test_control_cycle(const IN_VARS *pInVars, OUT_VARS *pOutVars)
{
    //TODO: Add your business logic here.
}

/**
********************************************************************************
 * @brief This is your callback function to detect errors in your In process 
 *        image.
 *        The function is called, if a matching state change at one ore more of 
 *        the added input process image variables occurred. The state change will 
 *        be detected at the beginning of the process image cycle, and is 
 *        executed in the same context (memory partition and priority) as your 
 *        cyclic application task.
 *        
 *        Keep callback execution time small because it may influence your 
 *        cycle time.
 *
 *        This function is called if error occurred and also if errors
 *        changed from bad to good.
*******************************************************************************/
void git_test_pi_cbf_errorStateChangeIn(void)
{
    if (git_test_pi_getErrorIn())
    {
        PI_IN_CHAN *pChan;

        for(pChan = git_test_pi_errorFirstIn();
            pChan != NULL;
            pChan = git_test_pi_errorNextIn(pChan))
        {
            //TODO: Add your error handling code here

//            /*
//             * Example:
//             * This example prints the name of the component variable and
//             * the name of the mapped SVI-variable of the erroneous channel.
//             */
//            SVI_PVINF pvInf;
//            git_test_pi_getPvInfIn(pChan, &pvInf);
//            printf("Name of component-variable:  %s\n", pChan->CODVarName);
//            printf("Name of mapped SVI-variable: %s\n\n", pvInf.Name);
        }
    }
    else
    {
        // Everything returned to ok
    }
}

/**
********************************************************************************
 * @brief This is your callback function to detect errors in your Out process 
 *        image.
 *        The function is called, if a matching state change at one ore more of 
 *        the added output process image variables occurred. The state change will 
 *        be detected at the end of the process image cycle, and is executed in 
 *        the same context (memory partition and priority) as your cyclic 
 *        application task.
 *        
 *        Keep callback execution time small because it may influence your 
 *        cycle time.
 *
 *        The function is called if errors occurred and also if errors
 *        changed from bad to good.
*******************************************************************************/
void git_test_pi_cbf_errorStateChangeOut(void)
{
    if (git_test_pi_getErrorOut())
    {
        PI_OUT_CHAN      *pChan;
        PI_ERROR_IDX_OUT idx;

        for(pChan = git_test_pi_errorFirstOut(&idx);
            pChan != NULL;
            pChan = git_test_pi_errorNextOut(&idx))
        {
            //TODO: Add your error handling code here

//            /*
//             * Example:
//             * This example prints the name of the component variable and
//             * the name of the mapped SVI-variable of the erroneous channel.
//             */
//            SVI_PVINF pvInf;
//            git_test_pi_getPvInfOut(pChan, &pvInf);
//            printf("Name of component-variable:  %s\n", pChan->CODVarName);
//            printf("Name of mapped SVI-variable: %s\n\n", pvInf.Name);
        }
    }
    else
    {
        // Everything returned to ok
    }
}

/**
********************************************************************************
 * @brief This is your callback function to validate the configuration values.
 *        The function is called only once after the configuration values had
 *        been read from the mconfig.ini.
*******************************************************************************/
SINT32 git_test_config_cbf_validate(CONFIG *pConfig)
{
    //TODO: add your validation code here

    return (OK);
}

/**
********************************************************************************
 * @brief This is your function to detect errors in your direct output channels.
 *
 *@param[in]  ErrorIndex   Index returned by a function git_test_write_*
*******************************************************************************/
void git_test_direct_getErrorOutChans(SINT32 ErrorIndex)
{
    if (ErrorIndex < 0)
    {
        OUT_CHAN *pChan;

        for (pChan = git_test_direct_errorFirstChanOut(ErrorIndex);
             pChan != NULL;
             pChan = git_test_direct_errorNextChanOut(pChan))
        {
            if (pChan->logCount == 0)
            {
            //TODO: Add your error handling code here

//                /*
//                 * Example:
//                 * This example prints the name of the component variable and
//                 * the name of the mapped SVI-variable of the erroneous channel.
//                 */
//                SVI_PVINF pvInf;
//                git_test_direct_getPvInfOut(pChan, &pvInf);
//                printf("Name of component-variable:  %s\n", pChan->CODVarName);
//                printf("Name of mapped SVI-variable: %s\n\n", pvInf.Name);

                /*
                 * increase logCount to avoid output of this erroneous channel
                 * the next time this function is called.
                 */
                pChan->logCount++;
            }


        }
    }
    else
    {
        // Everything returned to ok
    }
}

/**
********************************************************************************
 * @brief This is your function to detect errors in one of your
 *        direct input channels.
 *
 *@param[in]  ErrorIndex   Index returned by a function git_test_read_*
*******************************************************************************/
void git_test_direct_getErrorInChan(SINT32 ErrorIndex)
{
    if (ErrorIndex < 0)
    {
        IN_CHAN *pChan;
        pChan = git_test_direct_errorChanIn(ErrorIndex);

        if (pChan->logCount == 0)
        {
            //TODO: Add your error handling code here

//            /*
//             * Example:
//             * This example prints the name of the component variable and
//             * the name of the mapped SVI-variable of the erroneous channel.
//             */
//            SVI_PVINF pvInf;
//            git_test_direct_getPvInfIn(pChan, &pvInf);
//            printf("Name of component-variable:  %s\n", pChan->CODVarName);
//            printf("Name of mapped SVI-variable: %s\n\n", pvInf.Name);

            /*
             * increase logCount to avoid output of this erroneous channel
             * the next time this function is called.
             */
            pChan->logCount++;
        }
    }
    else
    {
        // Everything returned to ok
    }
}

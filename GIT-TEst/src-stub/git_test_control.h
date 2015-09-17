/**
********************************************************************************
* @file     git_test_control.h
* @author   Bachmann electronic GmbH
*
* @brief    This file contains all definitions and declarations,
*           which are global and public for user within the SW-module.
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH 2015
*******************************************************************************/

#ifndef git_test_CONTROL__H  /* Avoid problems with multiple include */
#define git_test_CONTROL__H

#include "../src-gen/git_test_direct.h"
#include "../src-gen/git_test_config.h"

void git_test_control_cycle(const IN_VARS *pInVars, OUT_VARS *pOutVars);
void git_test_pi_cbf_errorStateChangeIn(void);
void git_test_pi_cbf_errorStateChangeOut(void);
SINT32 git_test_config_cbf_validate(CONFIG *pConfig);
void git_test_direct_getErrorOutChans(SINT32 ErrorIndex);
void git_test_direct_getErrorInChan(SINT32 ErrorIndex);

#endif

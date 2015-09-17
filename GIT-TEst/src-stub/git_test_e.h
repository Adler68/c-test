/**
********************************************************************************
* @file     git_test_e.h
* @author   Bachmann electronic GmbH
*
* @brief    This file contains all definitions and declarations which
*           are exported by the module to other modules on the same CPU.
*
********************************************************************************
* COPYRIGHT BY BACHMANN ELECTRONIC GmbH 2015
*******************************************************************************/

/* Avoid problems with multiple including */
#ifndef GIT_TEST_E__H
#define GIT_TEST_E__H


/*--- Defines ---*/

/*
 * Project specific code for error source.
 * All error codes which are visible outside of the project must contain this
 * code as "error source". The error source for customer application projects
 * must be in the range 0x04000000 to 0x0e7FF0000.
 * The low word shall contain the error reason.
*/
#define M_ES_GIT_TEST  0x04000000




/*--- Structures ---*/


/*--- Function prototypes ---*/


/*--- Variable definitions ---*/


#endif

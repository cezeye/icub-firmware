/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Valentina Gaggero, Marco Accame
 * email:   valentina.gaggero@iit.it, marco.accame@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _HAL_WATCHDOG_H_
#define _HAL_WATCHDOG_H_

// - doxy begin -------------------------------------------------------------------------------------------------------

/** @file       hal_watchdog.h
    @brief      This header file implements public interface to the hal watchdog module.
    @author     marco.accame@iit.it, valentina.gaggero@iit.it
    @date       21/10/2011
**/

/** @defgroup hal_watchdog HAL watchdog

    The HAL watchdog ...
 
    @todo acemor-facenda: review documentation.
    
    @{        
 **/


// - external dependencies --------------------------------------------------------------------------------------------

#include "hal_common.h"



// - public #define  --------------------------------------------------------------------------------------------------
// empty-section
  

// - declaration of public user-defined types ------------------------------------------------------------------------- 


/** @typedef    typedef enum hal_watchdog_t 
    @brief      hal_watchdog_t contains every possible watchdog peripheral.
 **/ 
typedef enum  
{ 
    hal_watchdog1_normal = 0,       /**< if not refreshed within its countdown, then it forces a reset of the system */
    hal_watchdog2_window = 1        /**< if not refreshed within its countdown, then it executes a callback function and if not refreshed
                                         it forces a reset of the system */
} hal_watchdog_t;

enum { hal_watchdogs_number = 2 };



/** @typedef    typedef struct hal_watchdog_cfg_t;
    @brief      contains configuration data of watchdog peripheral.
 **/
typedef struct
{
    hal_reltime_t               countdown;                  /**< the countdown of the watchdog in microseconds      */
    hal_interrupt_priority_t    priority;                   /**< priority of the ISR for the window watchdog */
    hal_callback_t              onwindowexpiry_cbk;         /**< callback called by the ISR when the window watchdog expires   */
    void*                       onwindowexpiry_arg;         /**< argument of the callback                           */    
} hal_watchdog_cfg_t;


// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------

extern const hal_watchdog_cfg_t hal_watchdog_cfg_default; //= { .countdown = 20000, .priority = hal_int_priorityNONE, .onwindowexpiry_cbk = NULL, .onwindowexpiry_arg = NULL}



// - declaration of extern public functions ---------------------------------------------------------------------------

/** @fn			extern hal_result_t hal_watchdog_init(hal_watchdog_t id, const hal_watchdog_cfg_t *cfg)
    @brief  	This function initializes a watchdog. 
    @details    c rfce.
    @param      id              The watchdog to initialise. 
    @param      cfg             The configuration. If NULL it uses its default.
                                The normal watchdog has a countdown range from 10 msec upto 10 seconds.
                                The window watchdog has a countdown range from 5 ms upto 50 ms. The callback function has 
                                up to 800 usec to be executed.
    @return 	hal_res_NOK_generic in case the watchdog cannot be initted, hal_res_NOK_unsupported if it is not
                supported, hal_res_OK if successful
    @warning    a given watchdog can be initted multiple times until it is started. After it has started it cannot be initted 
                anymore and an attempt to init it will give a hal_res_NOK_generic.
  */
extern hal_result_t hal_watchdog_init(hal_watchdog_t id, const hal_watchdog_cfg_t *cfg);


/**
    @fn         extern hal_result_t hal_watchdog_start(hal_watchdog_t id)
    @brief      starts the watchdog @e watchdog if it was already initted. if it was already started, the watchdog is refreshed.
    @param      id        The watchdog to start. It must be initted before.
    @return     hal_res_NOK_generic in case the watchdog wasn't initted, else hal_res_OK
 **/
extern hal_result_t hal_watchdog_start(hal_watchdog_t id);


/**
    @fn         extern hal_result_t hal_watchdog_refresh(hal_watchdog_t id)
    @brief      refreshes the watchdog @e watchdog
    @param      id        The watchdog to refresh. It must be initted and started.
    @return     hal_res_NOK_generic in case the watchdog wasn't initted or started, else hal_res_OK
 **/
extern hal_result_t hal_watchdog_refresh(hal_watchdog_t id);



/** @}            
    end of group hal_watchdog  
 **/

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------




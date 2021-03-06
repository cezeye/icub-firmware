/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
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

/* @file       eOcfg_nvsEP_as_any_con_sxxdefault.c
    @brief      This file keeps constant configuration for ...
    @author     marco.accame@iit.it
    @date       04/06/2012
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h" 
#include "string.h"
#include "stdio.h"

#include "EoAnalogSensors.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "eOcfg_nvsEP_as_any_con_sxxdefault.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------





// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------


const eOas_strain_t eo_cfg_nvsEP_as_any_con_sxxdefault_defaultvalue =
{
    EO_INIT(.config)
    {
        EO_INIT(.mode)                          eoas_strainmode_acquirebutdonttx,
        EO_INIT(.datarate)                      1,    
        EO_INIT(.signaloncefullscale)           eobool_true,  
        EO_INIT(.filler01)                      {0xf1}
    },
    EO_INIT(.inputs)
    {
        EO_INIT(.filler04)                      {0xf1, 0xf2, 0xf3, 0xf4}
    },
    EO_INIT(.status)                       
    {
        EO_INIT(.fullscale)                     
        {
            EO_INIT(.head)
            {
                EO_INIT(.capacity)              6,
                EO_INIT(.itemsize)              2,
                EO_INIT(.size)                  0,
                EO_INIT(.dummy)                 0
            },
            EO_INIT(.data)                      {0}
        },
        EO_INIT(.calibratedvalues)
        {
            EO_INIT(.head)
            {
                EO_INIT(.capacity)              6,
                EO_INIT(.itemsize)              2,
                EO_INIT(.size)                  6,
                EO_INIT(.dummy)                 0
            },
            EO_INIT(.data)                      {0}
        },
        EO_INIT(.uncalibratedvalues)            
        {
            EO_INIT(.head)
            {
                EO_INIT(.capacity)              6,
                EO_INIT(.itemsize)              2,
                EO_INIT(.size)                  6,
                EO_INIT(.dummy)                 0
            },
            EO_INIT(.data)                      {0}
        }
    }
}; 


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables
// --------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------




// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------





// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------

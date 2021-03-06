/*
 * Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
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


// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _HL_CFG_PLUS_MODULES_H_
#define _HL_CFG_PLUS_MODULES_H_

// - doxy begin -------------------------------------------------------------------------------------------------------

/** @file       hl_cfg_plus_modules.h
    @brief      This header file keeps which plus modules to build in hl library
    @author     marco.accame@iit.it
    @date       10/21/2013
**/

// - external dependencies --------------------------------------------------------------------------------------------

#include "hl_common.h"

#if     defined(HL_CFG_TARGET_BY_FILE )
    #include "hl_cfg_plus_target.h"
#endif



// - public #define  --------------------------------------------------------------------------------------------------


#define HL_USE_CORE_CMSIS

#define HL_USE_CORE_STM32

// HL_BEH_* contain behaviours to be applied to every module

//#define HL_BEH_REMOVE_RUNTIME_PARAMETER_CHECK
//#define HL_BEH_REMOVE_RUNTIME_VALIDITY_CHECK



#if     defined(HL_USE_BRD_MCBSTM32)


// HL_USE_UTIL_* allow compilation of a given module (peripheral or sw utility)

//#define HL_USE_UTIL_SYS
//#define HL_USE_UTIL_GPIO
//#define HL_USE_UTIL_BITS
//#define HL_USE_UTIL_FIFO
//#define HL_USE_UTIL_I2C


// HL_USE_CHIP_* allow compilation of the driver of a given chip

//#define HL_USE_CHIP_XX_EEPROM
//#define HL_USE_CHIP_ST_L3G4200D
//#define HL_USE_CHIP_ST_LIS3X
 
#elif   defined(HL_USE_BRD_MCBSTM32C) 


// HL_USE_UTIL_* allow compilation of a given module (peripheral or sw utility)

//#define HL_USE_UTIL_SYS
//#define HL_USE_UTIL_GPIO
//#define HL_USE_UTIL_BITS
//#define HL_USE_UTIL_FIFO
//#define HL_USE_UTIL_I2C


// HL_USE_CHIP_* allow compilation of the driver of a given chip

//#define HL_USE_CHIP_XX_EEPROM
//#define HL_USE_CHIP_ST_L3G4200D
//#define HL_USE_CHIP_ST_LIS3X

#elif   defined(HL_USE_BRD_EMS001) 


// HL_USE_UTIL_* allow compilation of a given module (peripheral or sw utility)

//#define HL_USE_UTIL_SYS
//#define HL_USE_UTIL_GPIO
//#define HL_USE_UTIL_BITS
//#define HL_USE_UTIL_FIFO
//#define HL_USE_UTIL_I2C


// HL_USE_CHIP_* allow compilation of the driver of a given chip

//#define HL_USE_CHIP_XX_EEPROM
//#define HL_USE_CHIP_ST_L3G4200D
//#define HL_USE_CHIP_ST_LIS3X

#else
    #error HL: specify the modules
#endif



// - declaration of public user-defined types ------------------------------------------------------------------------- 


// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------



// - declaration of extern public functions ---------------------------------------------------------------------------




#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------




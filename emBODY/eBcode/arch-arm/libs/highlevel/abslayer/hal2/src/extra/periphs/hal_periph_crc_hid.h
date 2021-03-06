/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero, Marco Accame
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
#ifndef _HAL_PERIPH_CRC_HID_H_
#define _HAL_PERIPH_CRC_HID_H_


/* @file       hal_periph_crc_hid.h
    @brief      This header file implements hidden interface to crc
    @author     marco.accame@iit.it
    @date       09/16/2011
 **/


// - external dependencies --------------------------------------------------------------------------------------------

#include "hal_base.h"



// - declaration of extern public interface ---------------------------------------------------------------------------
 
#include "hal_crc.h"



// - #define used with hidden struct ----------------------------------------------------------------------------------
// empty-section


// - definition of the hidden struct implementing the object ----------------------------------------------------------

typedef struct
{
    uint8_t             supported_mask;                 /**< bit in position hal_crcx must be 1 if crcx is supported */
} hal_crc_hid_brdcfg_t;

// - declaration of extern hidden variables ---------------------------------------------------------------------------

extern const hal_crc_hid_brdcfg_t hal_brdcfg_crc__theconfig;

// - declaration of extern hidden functions ---------------------------------------------------------------------------




#endif  // include guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------





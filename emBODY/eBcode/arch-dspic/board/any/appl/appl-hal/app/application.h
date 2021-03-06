/* 
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Valentina Gaggero
 * email:   valentina.gaggero@iit.it
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

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

// - doxy begin -------------------------------------------------------------------------------------------------------

/** @file       application.h
    @brief      this file ...
    @author     marco.accame@iit.it
    @date       06/17/2011
**/


// - external dependencies --------------------------------------------------------------------------------------------


// - public #define  --------------------------------------------------------------------------------------------------
// empty-section
 
// - declaration of public user-defined types ------------------------------------------------------------------------- 
// empty-section

    
// - declaration of extern public variables, ...deprecated: better using use _get/_set instead ------------------------


// - declaration of extern public functions ---------------------------------------------------------------------------

/** @fn         extern void ap_init(void);
    @brief      initialises the application application 
 **/
extern void ap_init(void);

extern void ap_run(void);


// - doxy end ---------------------------------------------------------------------------------------------------------
// empty-section

#endif  // include-guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



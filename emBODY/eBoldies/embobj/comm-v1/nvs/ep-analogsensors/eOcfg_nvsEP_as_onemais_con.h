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

// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _EOCFG_NVSEP_AS_ONEMAIS_CON_H_
#define _EOCFG_NVSEP_AS_ONEMAIS_CON_H_




/** @file       eOcfg_nvsEP_as_onemais_con.h
	@brief      This header file gives the constant configuration for the NVs in the endpoint bodysensors
	@author     marco.accame@iit.it
	@date       04/05/2012
**/

/** @defgroup eo_werasfdgr234 Configuation of the NVs ...
    Tcecece 
    
    @{		
 **/



// - external dependencies --------------------------------------------------------------------------------------------

#include "EOconstvector.h"
#include "EOarray.h"
#include "EOnv.h"

#include "EoAnalogSensors.h"

#include "eOcfg_nvsEP_as.h"

 

// - public #define  --------------------------------------------------------------------------------------------------
// empty-section



// - declaration of public user-defined types ------------------------------------------------------------------------- 




typedef uint16_t eo_cfg_nvsEP_as_onemais_con_strainNumber_t; // just to have it

enum { strainOneMais_TOTALnumber = 0}; 


/** @typedef    typedef eOcfg_nvsEP_as_strainNVindex_t eo_cfg_nvsEP_as_onemais_con_strainNVindex_t
    @brief      It contains an index for all the network variables in a strain of the upper arm.
 **/
typedef eOcfg_nvsEP_as_strainNVindex_t eo_cfg_nvsEP_as_onemais_con_strainNVindex_t;


/** @typedef    typedef eOcfg_nvsEP_as_maisNVindex_t eo_cfg_nvsEP_as_onemais_con_maisNVindex_t
    @brief      It contains an index for all the network variables in a mais of the upper arm: none
 **/
typedef eOcfg_nvsEP_as_maisNVindex_t eo_cfg_nvsEP_as_onemais_con_maisNVindex_t;




/** @typedef    typedef enum eo_cfg_nvsEP_as_onemais_con_strainNumber_t
    @brief      It contains an index for all the strain in the body.
 **/
typedef enum
{
    maisOneMais_00 = 0
} eo_cfg_nvsEP_as_onemais_con_maisNumber_t;

enum { maisOneMais_TOTALnumber = 1}; 



/** @typedef    enum varsASonemais_TOTALnumber;
    @brief      It contains the total number of network variables managed by the endpoint leftleg
 **/
enum {varsASonemais_TOTALnumber = strainOneMais_TOTALnumber*strainNVindex_TOTALnumber + maisOneMais_TOTALnumber*maisNVindex_TOTALnumber };


/** @typedef    typedef struct eo_cfg_nvsEP_as_onemais_t;
    @brief      contains all the variables in the onemais of kind analog sensors
 **/
typedef struct                      // size is 48*1 = 48                
{
    eOas_mais_t                   maises[1];
} eo_cfg_nvsEP_as_onemais_t;        EO_VERIFYsizeof(eo_cfg_nvsEP_as_onemais_t, 48);

    
// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------


extern const eOas_strain_t* eo_cfg_nvsEP_as_strain_defaultvalue;
extern const eOas_mais_t* eo_cfg_nvsEP_as_mais_defaultvalue;

// - declaration of extern public functions ---------------------------------------------------------------------------



/** @fn         extern eOnvID_t eo_cfg_nvsEP_as_onemais_strain_NVID_Get(eo_cfg_nvsEP_as_onemais_con_strainNumber_t s, eo_cfg_nvsEP_as_onemais_con_strainNVindex_t snvindex)
    @brief      This function retrieves the eOnvID_t of a network variable with index @e snvindex for the strain number @e s
    @param      s               the strain number 
    @param      snvinxed        the index of the nv inside the strain
    @return     the nvid or EOK_uint16dummy in case of failure.
  */
extern eOnvID_t eo_cfg_nvsEP_as_onemais_strain_NVID_Get(eo_cfg_nvsEP_as_onemais_con_strainNumber_t s, eo_cfg_nvsEP_as_onemais_con_strainNVindex_t snvindex);


/** @fn         extern eOnvID_t eo_cfg_nvsEP_as_onemais_mais_NVID_Get(eo_cfg_nvsEP_as_onemais_con_maisNumber_t m, eo_cfg_nvsEP_as_onemais_con_maisNVindex_t mnvindex)
    @brief      This function retrieves the eOnvID_t of a network variable with index @e snvindex for the mais number @e m
    @param      m               the mais number 
    @param      mnvinxed        the index of the nv inside the mais
    @return     the nvid or EOK_uint16dummy in case of failure.
  */
extern eOnvID_t eo_cfg_nvsEP_as_onemais_mais_NVID_Get(eo_cfg_nvsEP_as_onemais_con_maisNumber_t m, eo_cfg_nvsEP_as_onemais_con_maisNVindex_t mnvindex);




// - declarations used to load the endpoint into a EOnvsCfg object ----------------------------------------------------


// it is a EOconstvector where each element is a EOtreenode whose data field is a EOnv_con_t object (id, capacity, valuedef, offset)
extern const EOconstvector* const eo_cfg_nvsEP_as_onemais_constvector_of_treenodes_EOnv_con;

// if not NULL it contains a mapping from IDs to index inside eo_cfg_nvsEP_as_onemais_constvector_of_treenodes_EOnv_con
extern const eOuint16_fp_uint16_t eo_cfg_nvsEP_as_onemais_fptr_hashfunction_id2index;

// the size of the ram to be used
#define EOK_cfg_nvsEP_as_onemais_RAMSIZE                                            (sizeof(eo_cfg_nvsEP_as_onemais_t));

// the hash function
extern uint16_t eo_cfg_nvsEP_as_onemais_hashfunction_id2index(uint16_t id);

/** @}            
    end of group eo_werasfdgr234  
 **/

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------








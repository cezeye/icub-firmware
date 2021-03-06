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

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "EoCommon.h"
#include "string.h"
#include "EOtheMemoryPool.h"
#include "EOtheErrorManager.h"
#include "EOnv.h"
#include "EOrop_hid.h"



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "EOtheFormer.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "EOtheFormer_hid.h" 


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------
// empty-section



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

static const char s_eobj_ownname[] = "EOtheFormer";
 
static EOtheFormer eo_theformer = 
{
    EO_INIT(.initted)       0
};




// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

 
extern EOtheFormer * eo_former_Initialise(void) 
{
    if(1 == eo_theformer.initted)
    {
        return(&eo_theformer);
    }
    
    eo_theformer.initted = 1;
  
    return(&eo_theformer);        
}    


extern EOtheFormer * eo_former_GetHandle(void) 
{
    return( (1 == eo_theformer.initted) ? (&eo_theformer) : (eo_former_Initialise()) );
}

extern uint16_t eo_former_GetSizeOfStream(EOtheFormer *p, const EOrop *rop)
{
    uint16_t size = sizeof(eOrophead_t);

    if((NULL == p) || (NULL == rop))
    {    
        return(0);
    }

    if( (eo_ropcode_none == rop->stream.head.ropc) || (eo_ropcode_usr == rop->stream.head.ropc) )
    {
        return(0);      
    }

    if(eobool_true == eo_rop_hid_DataField_is_Present(&rop->stream.head))
    {
         size += eo_rop_hid_DataField_EffectiveSize(rop->stream.head.dsiz);
    }

    if(1 == rop->stream.head.ctrl.plussign)
    {
        size += 4;
    }

    if(1 == rop->stream.head.ctrl.plustime)
    {
        size+= 8;
    }

    return(size);
}

extern eOresult_t eo_former_GetStream(EOtheFormer *p, const EOrop *rop, const uint16_t streamcapacity, uint8_t *streamdata, uint16_t *streamsize) //, eOipv4addr_t *ipaddr)
{
    uint16_t dataeffectivesize = 0;
    uint8_t  signsize = 0;
    uint8_t  timesize = 0;

    if((NULL == p) || (NULL == rop) || (NULL == streamdata) || (NULL == streamsize))// || (NULL == ipaddr))
    {    
        return(eores_NOK_nullpointer);
    }

    // we assume that the rop is legal as it was formed by the EOtheAgent or extracted from a stream by EOtheParser
    // thus we dont check too much. we just dont process eo_ropcode_none and eo_ropcode_usr
    if( (eo_ropcode_none == rop->stream.head.ropc) || (eo_ropcode_usr == rop->stream.head.ropc) )
    {
        if(eo_ropcode_usr == rop->stream.head.ropc)
        {
            eo_errman_Error(eo_errman_GetHandle(), eo_errortype_warning, s_eobj_ownname, "doesnt support user defined ropc");
        }
        return(eores_NOK_generic); 
    }


    if(eobool_true == eo_rop_hid_DataField_is_Present(&rop->stream.head))
    {
        dataeffectivesize = eo_rop_hid_DataField_EffectiveSize(rop->stream.head.dsiz);
    }

    if(1 == rop->stream.head.ctrl.plussign)
    {
        signsize = 4;
    }

    if(1 == rop->stream.head.ctrl.plustime)
    {
        timesize = 8;
    }

    // we cannot put inside tha stream what is contained inside the rop
    if(streamcapacity < (sizeof(eOrophead_t) + dataeffectivesize + signsize + timesize))
    {
        return(eores_NOK_generic);
    } 
    

    // copy head into stream
    memcpy(&streamdata[0], &rop->stream.head, sizeof(eOrophead_t));
    (*streamsize) = sizeof(eOrophead_t);

    // copy data into stream if any
    if(0 != dataeffectivesize)
    {
        memcpy(&streamdata[*streamsize], rop->stream.data, dataeffectivesize);
        (*streamsize) += dataeffectivesize;
    }
    
    
    // copy sign if any
    if(0 != signsize)
    {
        *((uint32_t*) (&streamdata[*streamsize]) ) = rop->stream.sign;
        (*streamsize) += 4; // or signsize 
    }
 
    // copy time if any
    if(0 != timesize)
    {
        *((uint64_t*) (&streamdata[*streamsize]) ) = rop->stream.time;
        (*streamsize) += 8; // or timesize
    }

    // ipaddr
//    *ipaddr = rop->aboutip.ipaddr;

    // to be called only once just before transmission
//    #warning --> i have removed the eo_agent_hid_OutROPonTransmission() from inside the former .... verify it
    //eo_agent_hid_OutROPonTransmission(eo_agent_GetHandle(), rop);
    
    return(eores_OK);
}


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





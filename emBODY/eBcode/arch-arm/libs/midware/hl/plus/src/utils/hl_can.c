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

/* @file       hl_can.c
	@brief      This file contains internals for hl can utilities.
	@author     marco.accame@iit.it
    @date       11/11/2013
**/

// - modules to be built: contains the HL_USE_* macros ---------------------------------------------------------------

#include "hl_cfg_plus_modules.h"


#if     defined(HL_USE_UTIL_CAN)

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------


#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "hl_core.h"        // contains the required stm32f10x_*.h or stm32f4xx*.h header files

#include "hl_gpio.h"
#include "hl_sys.h"

#include "hl_bits.h" 

#include "hl_arch.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hl_can.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hl_can_hid.h" 


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define HL_can_id2index(p)              ((uint8_t)((p)))

#define HL_can_port2peripheral(p)      ( ( hl_can1 == (p) ) ? (CAN1) : (CAN2) )



#warning WIP --> in HAL2 it does not trigger the rx handler... WE MUST FIX IT ...

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

const hl_can_cfg_t hl_can_cfg_default =
{
    .baudrate   = hl_can_baudrate_1mbps
};

//__weak extern const hl_can_advcfg_t *hl_can_advcfg = NULL;


// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------



typedef struct
{
    hl_can_cfg_t                config;
} hl_can_internal_item_t;


typedef struct
{
    uint8_t                     initted;
    hl_can_internal_item_t*     items[hl_cans_number];   
} hl_can_theinternals_t;



// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hl_boolval_t s_hl_can_supported_is(hl_can_t id);
static void s_hl_can_initted_set(hl_can_t id);
static hl_boolval_t s_hl_can_initted_is(hl_can_t id);


// hw related initialisation which may change with different versions of stm32fx mpus
static void s_hl_can_hw_gpio_init(hl_can_t id);
static void s_hl_can_hw_clock_init(hl_can_t id);
static hl_result_t s_hl_can_hw_registers_init(hl_can_t id);

static void s_hl_can_fill_gpio_init_altf(hl_can_t id, hl_gpio_init_t* rxinit, hl_gpio_altf_t* rxaltf, hl_gpio_init_t* txinit, hl_gpio_altf_t* txaltf);





// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static const variables
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static hl_can_theinternals_t s_hl_can_theinternals =
{
    .initted            = 0,
    .items              = { NULL }   
};



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern hl_result_t hl_can_init(hl_can_t id, const hl_can_cfg_t *cfg)
{
    hl_can_internal_item_t* intitem = s_hl_can_theinternals.items[HL_can_id2index(id)];

    if(hl_false == s_hl_can_supported_is(id))
    {
        return(hl_res_NOK_generic);
    }
    
    if(hl_true == s_hl_can_initted_is(id))
    {
        return(hl_res_NOK_generic);
    }    

    if(NULL == cfg)
    {
        cfg  = &hl_can_cfg_default;
    }
    
    
    // give memory to can internal item for this id ...   
    if(NULL == intitem)
    {
        // the internal item
        intitem = s_hl_can_theinternals.items[HL_can_id2index(id)] = hl_sys_heap_new(sizeof(hl_can_internal_item_t));   
    }
    
    // set config
    memcpy(&intitem->config, cfg, sizeof(hl_can_cfg_t));
     
            
    // init gpios
    s_hl_can_hw_gpio_init(id);
    
    // init clock
    s_hl_can_hw_clock_init(id);
    
    s_hl_can_initted_set(id);

    return(hl_res_OK);
}


extern hl_result_t hl_can_enable(hl_can_t id)
{
	hl_result_t res;
//    hl_can_internal_item_t* intitem = s_hl_can_theinternals.items[HL_can_id2index(id)];

    if(hl_false == s_hl_can_initted_is(id))
    {
        return(hl_res_NOK_generic);
    }

    res = s_hl_can_hw_registers_init(id); // hardware setup

    if(res != hl_res_OK)
    {
    	return(hl_res_NOK_generic);
    }
    
    // i dont init any isr in here ...


	return(hl_res_OK);
}


extern hl_result_t hl_can_disable(hl_can_t id) 
{
//    hl_can_internal_item_t* intitem = s_hl_can_theinternals.items[HL_can_id2index(id)];
    CAN_TypeDef* CANx = HL_can_port2peripheral(id); 

    if(hl_false == s_hl_can_initted_is(id))
    {
        return(hl_res_NOK_generic);
    }

   

    CAN_DeInit(CANx);

    return(hl_res_OK);
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------


// ---- isr of the module: begin ----


// ---- isr of the module: end ------



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static hl_boolval_t s_hl_can_supported_is(hl_can_t id)
{
    return(hl_bits_byte_bitcheck(hl_can_mapping.supported_mask, HL_can_id2index(id)) );
}

static void s_hl_can_initted_set(hl_can_t id)
{
    hl_bits_byte_bitset(&s_hl_can_theinternals.initted, HL_can_id2index(id));
}

static hl_boolval_t s_hl_can_initted_is(hl_can_t id)
{
    return(hl_bits_byte_bitcheck(s_hl_can_theinternals.initted, HL_can_id2index(id)));
}


static void s_hl_can_hw_gpio_init(hl_can_t id)
{

    hl_gpio_init_t gpio_init_rx;    
    hl_gpio_altf_t gpio_altf_rx;    
    hl_gpio_init_t gpio_init_tx;
    hl_gpio_altf_t gpio_altf_tx;   
    

    s_hl_can_fill_gpio_init_altf(id, &gpio_init_rx, &gpio_altf_rx, &gpio_init_tx, &gpio_altf_tx);
    
    hl_gpio_init(&gpio_init_rx);
    hl_gpio_altf(&gpio_altf_rx);
    
    hl_gpio_init(&gpio_init_tx);
    hl_gpio_altf(&gpio_altf_tx);
}


static void s_hl_can_hw_clock_init(hl_can_t id)
{
    RCC_APB1PeriphClockCmd((hl_can1 == id) ? (RCC_APB1Periph_CAN1) : (RCC_APB1Periph_CAN2), ENABLE);
}




#if 0
#if     defined(HL_USE_MPU_ARCH_STM32F1)
    // the clock APB1 is 36mhz, half the max frequency of 72 mhz. (see page 49/1096 of stm32f1x reference manual Doc ID 13902 Rev 14) 
    // the clock APB2 operates at full speed of 72 mhz. (see page 49/1096 of stm32f1x reference manual Doc ID 13902 Rev 14)
    // can is on APB1, aka the slow bus
    // we give a total of HL_CAN_TQ_TOTAL = 9 time quanta for the duration of a can bit. 
    // we split the 9 time quanta in 5+1+3 = HL_CAN_TQ_BS1+1+HL_CAN_TQ_BS2
    // we allow to stretch the bit duration in order to re-synch by maximum HL_CAN_TQ_SJW = 3 time quanta 
    //#define HL_CAN_CLK	  		      (hl_brdcfg_cpu__theconfig.speeds.slowbus)
    #define HL_CAN_CLK                  (SystemCoreClock/2)
    #define HL_CAN_TQ_TOTAL             9
    #define HL_CAN_TQ_SJW               CAN_SJW_3tq
    #define HL_CAN_TQ_BS1               CAN_BS1_5tq
    #define HL_CAN_TQ_BS2               CAN_BS2_3tq
#elif    defined(HL_USE_MPU_ARCH_STM32F4)
    // the clock APB1 is 42mhz, a fourth the max frequency of 168 mhz. (see page 23/180 of stm32f4x datasheet Doc ID 022152 Rev 3)
    // the clock APB2 is 84mhz, a half the max frequency of 168 mhz. (see page 23/180 of stm32f4x datasheet Doc ID 022152 Rev 3)
    // can is on APB1, aka the slow bus
    // we give a total of HL_CAN_TQ_TOTAL = 7 time quanta for the duration of a can bit. 
    // we split the 7 time quanta in 4+1+2 = HL_CAN_TQ_BS1+1+HL_CAN_TQ_BS2
    // we allow to stretch the bit duration in order to re-synch by maximum HL_CAN_TQ_SJW = 3 time quanta     
    //#define HL_CAN_CLK                  (hl_brdcfg_cpu__theconfig.speeds.slowbus)
    #define HL_CAN_TQ_TOTAL             7
    #define HL_CAN_TQ_SJW               CAN_SJW_3tq
    #define HL_CAN_TQ_BS1               CAN_BS1_4tq
    #define HL_CAN_TQ_BS2               CAN_BS2_2tq    
#else //defined(HL_USE_CPU_ARCH_*)
    #error ERR --> choose a HL_USE_CPU_ARCH_*
#endif 
#endif

static hl_result_t s_hl_can_hw_registers_init(hl_can_t id)
{
    static const  hl_can_advcfg_bitsampling_t s_hl_can_bitsampling_default = 
    {
    #if     defined(HL_USE_MPU_ARCH_STM32F1)    
        // for 36mhz
        .bs1        = CAN_BS1_5tq,
        .bs2        = CAN_BS2_3tq,        
        .sjw        = CAN_SJW_3tq
    #elif    defined(HL_USE_MPU_ARCH_STM32F4)
        // for 42mhz
        .bs1        = CAN_BS1_4tq,
        .bs2        = CAN_BS2_2tq,        
        .sjw        = CAN_SJW_3tq        
    #else //defined(HL_USE_CPU_ARC_*)
        #error ERR --> choose a HL_USE_CPU_ARCH_*
    #endif     
    };
    
    hl_can_internal_item_t* intitem = s_hl_can_theinternals.items[HL_can_id2index(id)];
    CAN_TypeDef* CANx = HL_can_port2peripheral(id);
    
    CAN_InitTypeDef* init2use = NULL;
    const hl_can_advcfg_bitsampling_t* bits2use = NULL;
    
    
    CAN_InitTypeDef CAN_InitStructure;
    CAN_StructInit(&CAN_InitStructure);       
    

    if(NULL != hl_can_mapping.advconf[HL_can_id2index(id)].full)
    {
        // use: the full as specified
        init2use = (CAN_InitTypeDef*) hl_can_mapping.advconf[HL_can_id2index(id)].full;    
        bits2use = NULL;       
    }
    else
    {
        // use: the standard initstructure
        init2use = &CAN_InitStructure;
        
        if(NULL != hl_can_mapping.advconf[HL_can_id2index(id)].bitsampling)
        {
            // use: the bitsampling as specified. 
            bits2use = hl_can_mapping.advconf[HL_can_id2index(id)].bitsampling;             
        }
        else
        {
            // use: standard bitsampling.
            bits2use = &s_hl_can_bitsampling_default; 
        }             
    }
    

    // in case we use the standard initstructure, we complete init2use with the chosen bits2use
    
    if(init2use == (&CAN_InitStructure))
    {
        RCC_ClocksTypeDef clocks;
        RCC_GetClocksFreq(&clocks);  
        
        uint32_t baudrate = 0;              
        switch(intitem->config.baudrate)
        {
            case hl_can_baudrate_1mbps:     baudrate = 1000000;     break;
            case hl_can_baudrate_500kbps:   baudrate =  500000;     break;
            default:                        baudrate =  500000;     break;
        }
        
        uint8_t tqtotal = bits2use->bs1 + 1 + bits2use->bs2;
        
        // CAN_Prescaler is the prescaler to apply to PCLK1_Frequency so that the bit has tqtotal time quanta (or ticks of the clock)
        // where tqtotal = CAN_BS1 + 1 + CAN_BS2        
        init2use->CAN_Prescaler = (clocks.PCLK1_Frequency / tqtotal) / baudrate;      
        init2use->CAN_Mode      = CAN_Mode_Normal;  // operating mode
        init2use->CAN_SJW       = bits2use->sjw;    // max num of time quanta the hw is allowed to stretch a bit in order to re-synch
        init2use->CAN_BS1       = bits2use->bs1;    // number of time quanta in bit-segment-1 (the one before the sampling time quantum)
        init2use->CAN_BS2       = bits2use->bs2;    // number of time quanta in bit-segment-2 (the one after the sampling time quantum)
        init2use->CAN_TTCM      = DISABLE;          // time-triggered communication mode
        init2use->CAN_ABOM      = DISABLE;          // automatic bus-off
        init2use->CAN_AWUM      = DISABLE;          // automatic wake-up
        init2use->CAN_NART      = DISABLE;          // no-automatic retransmission mode 
        init2use->CAN_RFLM      = DISABLE;          // receive fifo locked mode
        init2use->CAN_TXFP      = ENABLE;           // transmit fifo priority (if ENABLEd, priority amongst the pending mailboxes is driven by the request order)    
    }
    
    
    // ok, we init teh can
	 
    CAN_DeInit(CANx);

	if(CAN_InitStatus_Failed == CAN_Init(CANx, init2use))
	{
		return(hl_res_NOK_generic);
	}
    
    // we not init the filters: just a basic init

	// TODO: rendere configurabile i filtri
    // #warning VALE->filter doesn't work!!!
    // acemor on 19-oct-2012: FILTERNUM_CAN2 era 14 che e' un valore non valido ...
    //                        quindi lascio i filtri 0->6 per il can1 ed i filtri 7->13 per il can2
    #define FILTERNUM_CAN1                              0
    #define FILTERNUM_CAN2                              7

    // NOTE VALE: in order to receive msg, i had to init filter for receive all.
	// CAN filter init
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterMode              = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale             = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh            = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow             = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh        = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow         = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment    = 0;
	CAN_FilterInitStructure.CAN_FilterActivation        = ENABLE;
    CAN_FilterInitStructure.CAN_FilterNumber            = (hl_can1 == id) ? (FILTERNUM_CAN1) : (FILTERNUM_CAN2);

    CAN_FilterInit(&CAN_FilterInitStructure);
   
	return(hl_res_OK);
}



static void s_hl_can_fill_gpio_init_altf(hl_can_t id, hl_gpio_init_t* rxinit, hl_gpio_altf_t* rxaltf, hl_gpio_init_t* txinit, hl_gpio_altf_t* txaltf)
{
    static const hl_gpio_init_t s_hl_can_rx_gpio_init = 
    {
#if     defined(HL_USE_MPU_ARCH_STM32F1)
        .port           = hl_gpio_portNONE,
        .mode           =
        {
            .gpio_pins      = hl_gpio_pinNONE,
            .gpio_speed     = GPIO_Speed_50MHz, // both GPIO_Speed_2MHz and GPIO_Speed_50MHz are the same as it is configured in input mode
            .gpio_mode      = GPIO_Mode_IPU        
        }
#elif   defined(HL_USE_MPU_ARCH_STM32F4)  
        .port        = hl_gpio_portNONE,
        .mode        =
        {
            .gpio_pins      = hl_gpio_pinNONE,
            .gpio_mode      = GPIO_Mode_AF,
            .gpio_speed     = GPIO_Speed_50MHz,
            .gpio_otype     = GPIO_OType_PP,
            .gpio_pupd      = GPIO_PuPd_UP
        }
#else //defined(HL_USE_MPU_ARCH_*)
    #error ERR --> choose a HL_USE_MPU_ARCH_*
#endif 
    };

    
    static const hl_gpio_init_t s_hl_can_tx_gpio_init         = 
    {
#if     defined(HL_USE_MPU_ARCH_STM32F1)
        .port           = hl_gpio_portNONE,
        .mode           =
        {
            .gpio_pins      = hl_gpio_pinNONE,
            .gpio_speed     = GPIO_Speed_50MHz, 
            .gpio_mode      = GPIO_Mode_AF_PP        
        }
#elif   defined(HL_USE_MPU_ARCH_STM32F4)  
        .port        = hl_gpio_portNONE,
        .mode        =
        {
            .gpio_pins      = hl_gpio_pinNONE,
            .gpio_mode      = GPIO_Mode_AF,
            .gpio_speed     = GPIO_Speed_50MHz,
            .gpio_otype     = GPIO_OType_PP,
            .gpio_pupd      = GPIO_PuPd_UP
        }
#else //defined(HL_USE_MPU_ARCH_*)
    #error ERR --> choose a HL_USE_MPU_ARCH_*
#endif 
    };

   // at first we copy the default configuration of pin rx and tx
    
    memcpy(rxinit, &s_hl_can_rx_gpio_init, sizeof(hl_gpio_init_t));   
    memcpy(txinit, &s_hl_can_tx_gpio_init, sizeof(hl_gpio_init_t));
    
    // then we verify the pin mapping and the altfun ... ok don't do it.
    // but you could put it in here. maybe by calling an external function which depends on the mpu
    
    // then we set the port and pin of rx and tx
    hl_gpio_fill_init(rxinit, &hl_can_mapping.gpiomap[HL_can_id2index(id)].rx);
    hl_gpio_fill_init(txinit, &hl_can_mapping.gpiomap[HL_can_id2index(id)].tx);
    
    // then we set altfun of rx and tx
    hl_gpio_fill_altf(rxaltf, &hl_can_mapping.gpiomap[HL_can_id2index(id)].rx);
    hl_gpio_fill_altf(txaltf, &hl_can_mapping.gpiomap[HL_can_id2index(id)].tx);  

}

#endif//defined(HL_USE_UTIL_CAN)

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------






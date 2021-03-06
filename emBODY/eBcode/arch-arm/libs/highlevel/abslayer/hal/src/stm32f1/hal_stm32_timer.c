/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
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

/* @file       hal_timer.c
	@brief      This file implements internal implementation of the hal timer module.
	@author     marco.accame@iit.it
    @date       09/12/2011
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"

#ifdef HAL_USE_TIMER

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "string.h"
#include "stdlib.h"
#include "hal_stm32_base_hid.h" 
#include "hal_stm32_sys_hid.h"
#include "hal_brdcfg.h"


#include "stm32f1.h" 


 
// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_timer.h"



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hal_stm32_timer_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define HAL_timer_t2index(t)           ((uint8_t)((t)))




// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------
// emty-section

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

typedef struct
{
    TIM_TypeDef*        TIMx;
    uint32_t            RCC_APB1Periph_TIMx;
    IRQn_Type           TIMx_IRQn;
} hal_timer_stm32_regs_t;

typedef struct
{
    hal_timer_cfg_t             cfg;        // fixed for every architecture
    hal_timer_status_t          status;     // fixed for every architecture
    uint16_t                    period;     // fixed for every architecture
    uint16_t                    prescaler;  // fixed for every architecture
    uint32_t                    tick_ns;    
} hal_timer_datastructure_t;


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_timer_supported_is(hal_timer_t timer);
static void s_hal_timer_initted_set(hal_timer_t timer);
static hal_boolval_t s_hal_timer_initted_is(hal_timer_t timer);

static void s_hal_timer_status_set(hal_timer_t timer, hal_timer_status_t status);
static hal_timer_status_t s_hal_timer_status_get(hal_timer_t timer);

static void s_hal_timer_prepare(hal_timer_t timer, const hal_timer_cfg_t *cfg);

static void s_hal_timer_stm32_start(hal_timer_t timer);
static void s_hal_timer_stm32_stop(hal_timer_t timer);



static hal_time_t s_hal_timer_get_period(hal_timer_t timer);

static void s_hal_timer_callback(hal_timer_t timer);


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static const hal_timer_stm32_regs_t s_hal_timer_stm32regs[hal_timers_num] =
{
    {   // timer1   
        .TIMx                   = TIM1,
        .RCC_APB1Periph_TIMx    = RCC_APB2Periph_TIM1, 
        .TIMx_IRQn              = TIM1_UP_IRQn 
    },
    {   // timer2   
        .TIMx                   = TIM2,
        .RCC_APB1Periph_TIMx    = RCC_APB1Periph_TIM2, 
        .TIMx_IRQn              = TIM2_IRQn 
    },
    {   // timer3   
        .TIMx                   = TIM3,
        .RCC_APB1Periph_TIMx    = RCC_APB1Periph_TIM3, 
        .TIMx_IRQn              = TIM3_IRQn 
    },
    {   // timer4   
        .TIMx                   = TIM4,
        .RCC_APB1Periph_TIMx    = RCC_APB1Periph_TIM4, 
        .TIMx_IRQn              = TIM4_IRQn 
    },
    {   // timer5   
        .TIMx                   = TIM5,
        .RCC_APB1Periph_TIMx    = RCC_APB1Periph_TIM5, 
        .TIMx_IRQn              = TIM5_IRQn 
    },
    {   // timer6   
        .TIMx                   = TIM6,
        .RCC_APB1Periph_TIMx    = RCC_APB1Periph_TIM6, 
        .TIMx_IRQn              = TIM6_IRQn 
    },
    {   // timer7   
        .TIMx                   = TIM7,
        .RCC_APB1Periph_TIMx    = RCC_APB1Periph_TIM7, 
        .TIMx_IRQn              = TIM7_IRQn 
    },
    {   // timer8   
        .TIMx                   = NULL,
        .RCC_APB1Periph_TIMx    = 0, 
        .TIMx_IRQn              = TIM7_IRQn // we use this value just to remove a warning raised by the compiler 
    }
};

static hal_timer_datastructure_t * s_hal_timer_info[hal_timers_num] = { NULL };


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern hal_result_t hal_timer_init(hal_timer_t timer, const hal_timer_cfg_t *cfg, hal_time_t *error)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];    

    if(hal_false == s_hal_timer_supported_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    if(NULL == info)
    {
        return(hal_res_NOK_generic);
    }

    if(NULL == cfg)
    {
        return(hal_res_NOK_nullpointer);
    }

    if(0 == cfg->countdown)
    {
        return(hal_res_NOK_wrongparam);
    }

    if((hal_timer_mode_oneshot == cfg->mode) && (hal_int_priorityNONE == cfg->priority))
    {
        return(hal_res_NOK_wrongparam);
    }

    if(hal_timer_prescalerAUTO != cfg->prescaler)
    {
        return(hal_res_NOK_wrongparam);
    }
     
    // if running stop.
    if(hal_timer_status_running == hal_timer_status_get(timer))
    {
        hal_timer_stop(timer);
    }

    // marker: save the cfg and init registers

    // computes the values to be put in registers and compute the error in microsec
    s_hal_timer_prepare(timer, cfg);

    // marker: calculate error
    if(NULL != error)
    {
        hal_time_t period = s_hal_timer_get_period(timer);
        if(period > cfg->countdown)
        {
            *error = period - cfg->countdown; 
        }
        else
        {
            *error = cfg->countdown - period;
        }
    }
    
    s_hal_timer_initted_set(timer);

    return(hal_res_OK);
}


extern hal_result_t hal_timer_start(hal_timer_t timer)
{
    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    if(hal_timer_status_running == s_hal_timer_status_get(timer))
    {
        hal_timer_stop(timer);
    }

    s_hal_timer_status_set(timer, hal_timer_status_running);
    s_hal_timer_stm32_start(timer); 

    return(hal_res_OK);
}



extern hal_result_t hal_timer_stop(hal_timer_t timer)
{
    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    if(hal_timer_status_idle == s_hal_timer_status_get(timer))
    {
        return(hal_res_OK);
    }
    // marker: stop and set status

    s_hal_timer_stm32_stop(timer);
    s_hal_timer_status_set(timer, hal_timer_status_idle);

    return(hal_res_OK);
}



extern hal_result_t hal_timer_countdown_set(hal_timer_t timer, hal_time_t countdown, hal_time_t *error)
{                                                                            
    hal_timer_cfg_t *curcfg = NULL;
    hal_timer_cfg_t newcfg;
    uint8_t wasrunning = 0;

    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    // if running stop.
    if(hal_timer_status_running == hal_timer_status_get(timer))
    {
        wasrunning = 1;
    }

    // computes the values to be put in registers
    curcfg = &s_hal_timer_info[HAL_timer_t2index(timer)]->cfg;
    memcpy(&newcfg, curcfg, sizeof(hal_timer_cfg_t));
    newcfg.countdown = countdown;

    hal_timer_init(timer, &newcfg, error);

    if(1 == wasrunning)
    {
        hal_timer_start(timer);
    }

    return(hal_res_OK);
}


extern hal_result_t hal_timer_priority_set(hal_timer_t timer, hal_interrupt_priority_t prio)
{
    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    // do something 

    return(hal_res_NOK_unsupported);
}


extern hal_result_t hal_timer_interrupt_enable(hal_timer_t timer)
{
    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    // do something 

    return(hal_res_NOK_unsupported);
}



extern hal_result_t hal_timer_interrupt_disable(hal_timer_t timer)
{
    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    // do something 

    return(hal_res_NOK_unsupported);
}



extern hal_result_t hal_timer_remainingtime_get(hal_timer_t timer, hal_time_t *remaining_time)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];
    TIM_TypeDef* TIMx               = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].TIMx;

    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    *remaining_time = info->tick_ns * TIM_GetCounter(TIMx) / 1000;

    return(hal_res_OK);
}


extern hal_result_t hal_timer_offset_write(hal_timer_t timer, hal_nanotime_t offset)
{
    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_res_NOK_generic);
    }

    // do something 

    // not supported
    return(hal_res_NOK_unsupported);
}


extern hal_timer_status_t hal_timer_status_get(hal_timer_t timer)
{

    if(hal_false == s_hal_timer_initted_is(timer))
    {
        return(hal_timer_status_none);
    }

    return(s_hal_timer_status_get(timer));
}




// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// ---- isr of the module: begin ----

void TIM1_UP_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    s_hal_timer_callback(hal_timer1);
}

void TIM2_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    s_hal_timer_callback(hal_timer2);
}


void TIM3_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    s_hal_timer_callback(hal_timer3);
}


void TIM4_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    s_hal_timer_callback(hal_timer4);
}

void TIM5_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    s_hal_timer_callback(hal_timer5);
}

void TIM6_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    s_hal_timer_callback(hal_timer6);
}

void TIM7_IRQHandler(void)
{
    // Clear TIMx update interrupt 
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    s_hal_timer_callback(hal_timer7);
}

// ---- isr of the module: end ------


extern uint32_t hal_timer_hid_getsize(const hal_cfg_t *cfg)
{
    uint32_t size = 0;
    
    size += (cfg->timers_num * sizeof(hal_timer_datastructure_t));

    return(size);
}

extern hal_result_t hal_timer_hid_setmem(const hal_cfg_t *cfg, uint32_t *memory)
{
    uint8_t i = 0;
    uint8_t *ram08 = (uint8_t*)memory;

    // removed dependancy from NZI ram
    for(i=0; i<hal_timers_num; i++)
    {
        s_hal_timer_info[i] = NULL;
    }

    // in case we have any timers
    if(0 != cfg->timers_num) 
    {
        if(NULL == memory)
        {
            hal_base_hid_on_fatalerror(hal_fatalerror_missingmemory, "hal_timer_hid_setmem(): memory missing");
            return(hal_res_NOK_generic);
        }

        for(i=0; i<cfg->timers_num; i++)
        {
            s_hal_timer_info[i] = (hal_timer_datastructure_t*)ram08;
            ram08 += sizeof(hal_timer_datastructure_t);
        }
    }

    return(hal_res_OK);  
}

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_timer_supported_is(hal_timer_t timer)
{
    return(hal_base_hid_byte_bitcheck(hal_brdcfg_timer__supported_mask, HAL_timer_t2index(timer)) );
}

static void s_hal_timer_initted_set(hal_timer_t timer)
{
    s_hal_timer_status_set(timer, hal_timer_status_idle);
}

static hal_boolval_t s_hal_timer_initted_is(hal_timer_t timer)
{
    return((hal_timer_status_none != s_hal_timer_status_get(timer)) ? (hal_true) : (hal_false));
}

static void s_hal_timer_status_set(hal_timer_t timer, hal_timer_status_t status)
{
    s_hal_timer_info[HAL_timer_t2index(timer)]->status = status;
}

static hal_timer_status_t s_hal_timer_status_get(hal_timer_t timer)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];
    return( (NULL == info) ? (hal_timer_status_none) : (info->status) );
}

static void s_hal_timer_prepare(hal_timer_t timer, const hal_timer_cfg_t *cfg)
{

    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];

    memcpy(&info->cfg, cfg, sizeof(hal_timer_cfg_t));

    // use prescaler = ((SystemCoreClock/a/1000) )

    if(0 == (info->cfg.countdown % 1000))
    {   // multiple of 1 ms: use 10 khz, thus a = 10. 1 tick is 100us, max countdown is 6400 msec = 6.4 s

        if(info->cfg.countdown > 64000*100) // tick is 100
        {
            info->cfg.countdown = 64000*100; // tick is 100
        }

        info->prescaler   = ((SystemCoreClock/10/1000) );  // a is 10. the value is 7200: ok, lower than 65k
        info->period      = info->cfg.countdown / 100; // tick is 100
        info->tick_ns     = 100*1000; // tick is 100

    }
    else if(0 == (info->cfg.countdown % 100))
    {   // multiple of 100 us: use 100 khz, thus a = 100. 1 tick is 10us, max countdown is 640 msec
        
        if(info->cfg.countdown > 64000*10) // tick is 10
        {
            info->cfg.countdown = 64000*10; // tick is 10
        }

        info->prescaler   = ((SystemCoreClock/100/1000) );  // a is 100. the value is 720: ok, lower than 65k
        info->period      = info->cfg.countdown / 10; // tick is 10
        info->tick_ns     = 10*1000; // tick is 10
    }
    else if(0 == (info->cfg.countdown % 10))
    {   // multiple of 10 us: use 1000 khz, thus a = 1000. 1 tick is 1us, max countdown is 64 msec
        
        if(info->cfg.countdown > 64000*1) // tick is 1
        {
            info->cfg.countdown = 64000*1; // tick is 1
        }

        info->prescaler   = ((SystemCoreClock/1000/1000) );  // a is 1000. the value is 72: ok, lower than 65k
        info->period      = info->cfg.countdown / 1; // tick is 1
        info->tick_ns     = 1*1000; // tick is 1
    }
    else
    {   // multiple of 1 us: use 8000 khz, thus a = 8000. 1 tick is 0.125us, max countdown is 8 msec
        
        if(info->cfg.countdown > 8000) // tick is 0.125
        {
            info->cfg.countdown = 8000; // tick is 0.125
        }

        info->prescaler   = ((SystemCoreClock/8000/1000) );  // a is 8000. the value is 9: ok, lower than 65k
        info->period      = info->cfg.countdown * 8; // tick is 0.125
        info->tick_ns     = 125; // tick is 0.125 micro
    }


}

static void s_hal_timer_stm32_start(hal_timer_t timer)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];
    TIM_TypeDef* TIMx               = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].TIMx;
    uint32_t RCC_APB1Periph_TIMx    = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].RCC_APB1Periph_TIMx;
    IRQn_Type TIMx_IRQn             = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].TIMx_IRQn;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   // NVIC_InitTypeDef NVIC_InitStructure;
   


    // Enable TIMx clock 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, ENABLE);

    // registers of TIMx

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = info->period - 1;          
    TIM_TimeBaseStructure.TIM_Prescaler = info->prescaler - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;  
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);


    //* enable counter 
    TIM_Cmd(TIMx, ENABLE);

    // Immediate load of  Precaler value */
    TIM_PrescalerConfig(TIMx, info->prescaler - 1, TIM_PSCReloadMode_Immediate);
    //TIM_PrescalerConfig(TIMx, info->prescaler - 1, TIM_PSCReloadMode_Update);
    

    // Clear  update pending flag */
    TIM_ClearFlag(TIMx, TIM_FLAG_Update);

    if(hal_int_priorityNONE != info->cfg.priority)
    {
        // Enable TIM2 Update interrupt */
        TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
    
        // enable irqs in nvic
        hal_sys_irqn_priority_set(TIMx_IRQn, info->cfg.priority);
        hal_sys_irqn_enable(TIMx_IRQn);
    }
}

static void s_hal_timer_stm32_stop(hal_timer_t timer)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];
//    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TypeDef* TIMx               = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].TIMx;
//    uint32_t RCC_APB1Periph_TIMx    = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].RCC_APB1Periph_TIMx;
    IRQn_Type TIMx_IRQn             = s_hal_timer_stm32regs[HAL_timer_t2index(timer)].TIMx_IRQn;
   

#if 1

    TIM_DeInit(TIMx);
    TIM_Cmd(TIMx, DISABLE);

    if(hal_int_priorityNONE != info->cfg.priority)
    {
        TIM_ITConfig(TIMx, TIM_IT_Update, DISABLE);
        hal_sys_irqn_disable(TIMx_IRQn);
    }

#else

    // Enable TIMx clock 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIMx, DISABLE);

    // register of TIMx
 
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);


    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);


    //* disable counter 
    TIM_Cmd(TIMx, DISABLE);

    // Immediate load of  Precaler value */
    TIM_PrescalerConfig(TIMx, info->prescaler - 1, TIM_PSCReloadMode_Immediate);

    // Clear  update pending flag */
    TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
//    TIM_ClearFlag(TIMx, TIM_FLAG_Update);

    // Enable TIM2 Update interrupt */
    TIM_ITConfig(TIMx, TIM_IT_Update, DISABLE);

    // nvic
    #warning --> acemor removed it to maintain 16 priorities 
    /* Configure two bits for preemption priority */
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
    /* Enable the TIM2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIMx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

#endif
}

//
//
//
//static void s_TIM_Configuration(void)
//{ 
//  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
////  TIM_OCInitTypeDef  TIM_OCInitStructure;
//
//    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
//
////    prescaler = ((SystemCoreClock/a/1000) - 1)
////    per granualrita' 1000 us -> vado a 2khz quindi uso a = 2   max 32000 msec
////    per               100 us ->       20khz            a = 20  max  3200 ms
////                       10 us ->      200khz            a = 200 max   320 ms
////                          1 us         2000khz           a = 2000 max 32 ms
////
////                       oppure 1000 us -> 10khz quindi max 6.4 sec
//
//  /* TIM2 configuration */
//  TIM_TimeBaseStructure.TIM_Period = 2000 - 1;          
////  TIM_TimeBaseStructure.TIM_Prescaler = 36000 - 1;//((SystemCoreClock/1200) - 1);  
////  TIM_TimeBaseStructure.TIM_Prescaler = TIM_TimeBaseStructure.TIM_Prescaler;     
//  TIM_TimeBaseStructure.TIM_Prescaler = ((SystemCoreClock/2/1000) - 1); 
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;  
//  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
//  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
//
//  //TIM_SetClockDivision(TIM2, TIM_CKD_DIV1);
//  
////  TIM_OCStructInit(&TIM_OCInitStructure);
////  /* Output Compare Timing Mode configuration: Channel1 */
////  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
////  TIM_OCInitStructure.TIM_Pulse = 0x0;  
////  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
////  
////  /* TIM3 configuration */
////  TIM_TimeBaseStructure.TIM_Period = 0x95F;
////          
////  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
////  /* Output Compare Timing Mode configuration: Channel1 */
////  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
////  
////  /* TIM4 configuration */
////  TIM_TimeBaseStructure.TIM_Period = 0xE0F;  
////        
////  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
////  /* Output Compare Timing Mode configuration: Channel1 */
////  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
//
//  /* TIM2 enable counter */
//  TIM_Cmd(TIM2, ENABLE);
////  /* TIM3 enable counter */
////  TIM_Cmd(TIM3, ENABLE);
////  /* TIM4 enable counter */
////  TIM_Cmd(TIM4, ENABLE);
//
//  /* Immediate load of TIM2 Precaler value */
////  TIM_PrescalerConfig(TIM2, ((SystemCoreClock/2/1000) - 1), TIM_PSCReloadMode_Immediate);
////  /* Immediate load of TIM3 Precaler value */  
////  TIM_PrescalerConfig(TIM3, ((SystemCoreClock/1200) - 1), TIM_PSCReloadMode_Immediate);
////  /* Immediate load of TIM4 Precaler value */
////  TIM_PrescalerConfig(TIM4, ((SystemCoreClock/1200) - 1), TIM_PSCReloadMode_Immediate);
//
//  /* Clear TIM2 update pending flag */
//  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
////  /* Clear TIM3 update pending flag */
////  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
////  /* Clear TIM4 update pending flag */
////  TIM_ClearFlag(TIM4, TIM_FLAG_Update);
//
//  /* Enable TIM2 Update interrupt */
//  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
////  /* Enable TIM3 Update interrupt */
////  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
////  /* Enable TIM4 Update interrupt */
////  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
//}

static hal_time_t s_hal_timer_get_period(hal_timer_t timer)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];

    return(info->cfg.countdown);
}


static void s_hal_timer_callback(hal_timer_t timer)
{
    hal_timer_datastructure_t *info = s_hal_timer_info[HAL_timer_t2index(timer)];
    
    if(hal_timer_mode_oneshot == info->cfg.mode)
    {
        // stop timer 
        s_hal_timer_stm32_stop(timer);
              
        s_hal_timer_status_set(timer, hal_timer_status_expired);
    }

    if(NULL != info->cfg.callback_on_exp)
    {
        info->cfg.callback_on_exp(info->cfg.arg);
    }
}


#endif//HAL_USE_TIMER

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------




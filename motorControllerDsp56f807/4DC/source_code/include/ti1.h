/*
 * ti1.h
 *	the interface and definitions to the timer interrupt handler.
 *
 */

#ifndef __ti1h__
#define __ti1h__

/**
 * \file ti1.h
 * the definitions of the timer interrupt module.
 */

#include "dsp56f807.h"

extern volatile bool _wait ;
extern volatile byte _count;
/*
 * prototypes.
 */
void TI1_init (void);
void TI1_interrupt (void);
Int16 TI1_getCounter();

// The function get the counter value of Timer1
Int16 TI1_getCounter();
#endif /* ifndef __TI1 */
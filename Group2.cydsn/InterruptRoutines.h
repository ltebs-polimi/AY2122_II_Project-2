/*******************************************************************************
* File Name: InterruptRoutines.h
*
* Description:
*  Functions prototypes and some varibles used in InterruptRoutines.c
*
*********************************************************************************/
#ifndef __INTERRUPT_ROUTINES_H
    #define __INTERRUPT_ROUTINES_H
    
    #include "cytypes.h"
    #include "stdio.h"
    
    #define TRANMSIT_BUFFER_SIZE 16
    
    CY_ISR_PROTO (Custom_ISR_RX);
    CY_ISR_PROTO (dacInterrupt);
    CY_ISR_PROTO (adcInterrupt);
    CY_ISR_PROTO (adcAmpInterrupt); 
    
    
#endif

/* [] END OF FILE */

/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * -------------------- INTERRUPT ROUTINES (header) -----------------------
 * Functions prototypes and some varibles used in InterruptRoutines.c
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/
#ifndef __INTERRUPT_ROUTINES_H
    #define __INTERRUPT_ROUTINES_H
    
    #include "cytypes.h"
    #include "stdio.h"
    

    
    CY_ISR_PROTO (Custom_ISR_RX);
    CY_ISR_PROTO (dacInterrupt);
    CY_ISR_PROTO (adcInterrupt);
    CY_ISR_PROTO (adcAmpInterrupt); 
      
#endif
/* [] END OF FILE */

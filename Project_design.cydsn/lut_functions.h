/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ----------------------- LUT FUNCTIONS (header) -------------------------
 * This file contains the function prototypes and constants used for
 *  the protocols to create look up tables.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#ifndef _LUT_PROTOCOLS_H
    #define _LUT_PROTOCOLS_H

    #include <project.h>
    #include "stdlib.h"  // remove after testing
    #include "stdio.h"  // remove after testing

//Function Prototypes    
    uint16 LUT_MakeCVStartZero(uint16 start_value, uint16 end_value);
    uint16 LUT_MakeTriangle_Wave(uint16 start_value, uint16 end_value);
    void LUT_MakePulse(uint16 base, uint16 pulse);
    uint16 LUT_make_line(uint16 start, uint16 end, uint16 index);

//Global variables external identifier
    extern uint16 waveform_CV_lut[];
    extern uint16 waveform_amp_lut[];
    
#endif

/* [] END OF FILE */

/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ----------------------- LUT FUNCTIONS (source) -------------------------
 * This file contains the protocols to create look up tables.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#include "lut_functions.h"
#include "Globals.h"
extern char LCD_str[];  // for debug


/*  LUT MAKE TRIANGLE WAVE
*   \brief: Fills in the look up table (waveform_CV_lut) for the DACs to perform a cyclic voltammetry experiment.
*   \Parameters:
*       @param start_value: first value to put in the dac
*       @param end_value: to peak dac value
*   \Return: _lut_index: how long the look up table is.
*/
uint16 LUT_MakeTriangle_Wave(uint16 start_value, uint16 end_value) {
    
    uint16 _lut_index = 0;  // start at the beginning of the lut
    
    _lut_index = LUT_make_line(start_value, end_value, 0);
    
    _lut_index = LUT_make_line(end_value, start_value, _lut_index-1);
    waveform_CV_lut[_lut_index] = start_value;  // the DAC is changed before the value is checked in the isr so it will go 1 over so make it stay at last voltage
    _lut_index++;

    
    return _lut_index;  
}


/*  LUT MAKE LINE
*   \brief: Makes a ramp from start to end in waveform_CV_lut starting at index
*   \Parameters:
*       @param start: first value to put in the look up table
*       @param end: end value to put in the look up table
*       @param index: the place to start putting in numbers in the look up table
*   \Return: index: first place after the filled in area of the look up table.
*/
uint16 LUT_make_line(uint16 start, uint16 end, uint16 index) {
    
    if (start < end) {
        for (uint16 value = start; value <= end; value = value+2) {  //increasing part of the triangular wave
            waveform_CV_lut[index] = value;
            index ++;
        }
    }
    else {
        for (uint16 value = start; value >= end; value = value-2) { //decreasing part of the triangular wave
            waveform_CV_lut[index] = value;
            index ++;
        }
    }
    return index;
}


/*  LUT MAKE PULSE
*   \brief: Make a look up table that stores a square pulse sequence
*   \Parameters:
*       @param base: value to be placed in the DAC to maintain the baseline potential
*       @param pulse: value to put in the DAC for the voltage pulse
*   \Return: NONE
*/
void LUT_MakePulse(uint16 base, uint16 pulse) {
    int _lut_index = 0;
    while (_lut_index < 10) {  //set the base value for the first 100 values (@10 ms (i.e. 100Hz), it means that for 1 sec we are imposing the base value)
        waveform_amp_lut[_lut_index] = base;
        _lut_index++;
    }
    while (_lut_index < 200) { //set the pulse value for the following 1900 values (@10 ms (i.e. 100Hz), it means that for 19 secs we are imposing the pulse value)
        waveform_amp_lut[_lut_index] = pulse;
        _lut_index++;
    }
}
/* [] END OF FILE */

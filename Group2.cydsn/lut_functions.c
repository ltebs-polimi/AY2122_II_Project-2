/*******************************************************************************
* File Name: lut_functions.c
*
* Description:
*  This file contains the protocols to create look up tables
*
*********************************************************************************/

#include "lut_functions.h"
#include "Global_variables.h"
extern char LCD_str[];  // for debug

/******************************************************************************
* Function Name: LUT_MakeTriangleWave
*******************************************************************************
*
* Summary:
*  Fill in the look up table (waveform_CV_lut) for the DACs to perform a cyclic voltammetry experiment.
*
* Parameters:
*  uint16 start_value: first value to put in the dac
*  uint16 end_value: to peak dac value
*
* Return:
*  uint16: how long the look up table is
*
* Global variables:
*  waveform_CV_lut: Array the look up table is stored in
*
*******************************************************************************/

uint16 LUT_MakeTriangle_Wave(uint16 start_value, uint16 end_value) {
    uint16 _lut_index = 0;  // start at the beginning of the lut
    
    
    _lut_index = LUT_make_line(start_value, end_value, 0);
    
    _lut_index = LUT_make_line(end_value, start_value, _lut_index-1);
    waveform_CV_lut[_lut_index] = start_value;  // the DAC is changed before the value is checked in the isr so it will go 1 over so make it stay at last voltage
    _lut_index++;

    
    return _lut_index;  
}



/******************************************************************************
* Function Name: LUT_make_line
*******************************************************************************
*
* Summary:
*  Make a ramp from start to end in waveform_CV_lut starting at index
*
* Parameters:
*  uint16 start: first value to put in the look up table
*  uint16 end: end value to put in the look up table
*  uint16 index: the place to start putting in numbers in the look up table
*
* Return:
*  uint16: first place after the filled in area of the look up table
*
* Global variables:
*  waveform_CV_lut: Array the look up table is stored in
*
*******************************************************************************/

uint16 LUT_make_line(uint16 start, uint16 end, uint16 index) {
    if (start < end) {
        for (uint16 value = start; value <= end; value++) {  //increasing part of the triangular wave
            waveform_CV_lut[index] = value;
            index ++;
        }
    }
    else {
        for (uint16 value = start; value >= end; value--) { //decreasing part of the triangular wave
            waveform_CV_lut[index] = value;
            index ++;
        }
    }
    return index;
}

/******************************************************************************
* Function Name: LUT_MakePulse
*******************************************************************************
*
* Summary:
*  Make a look up table that stores a square pulse sequence
*  Quick hack for chronoamperomerty
*  TODO: THIS SHOULD BE REPLACED
*
* Parameters:
*  uint16 base: value to be placed in the DAC to maintain the baseline potential
*  uint16 pulse: value to put in the DAC for the voltage pulse
*
* Global variables:
*  waveform_amp_lut: Array the look up table is stored in
*
*******************************************************************************/

void LUT_MakePulse(uint16 base, uint16 pulse) {
    int _lut_index = 0;
    while (_lut_index < 100) {  //set the base value for the first 100 values (@10 ms (i.e. 100Hz), it means that for 1 sec we are imposing the base value)
        waveform_amp_lut[_lut_index] = base;
        _lut_index++;
    }
    while (_lut_index < 2000) { //set the pulse value for the following 1900 values (@10 ms (i.e. 100Hz), it means that for 19 secs we are imposing the pulse value)
        waveform_amp_lut[_lut_index] = pulse;
        _lut_index++;
    }

}
/* [] END OF FILE */
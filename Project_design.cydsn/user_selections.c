/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ---------------------- USER SELECTIONS (source) ------------------------
 * This file contains the protocols to create look up tables
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#include <project.h>

#include "user_selections.h"
#include "InterruptRoutines.h"
#include "lut_functions.h"


/* START CYCLIC VOLTAMMETRY
*   \brief: Start a cyclic voltammetry experiment.  The look up table in waveform_CV_lut should
*           already be created.  If the dac isr is already running this will not start and throws
*           and error through the USB.
*   \Parameters: NONE
*   \Return: NONE (Starts the isr's used to perform an experiment else if the dac is already running,
*                  possibly because another experiment is already running, return an error through the USB)
*/
void user_start_cv_run(void){  // It is used to setup everything that is needed for a CV scan, then the look up table is actually scanned in the dac isr (see main.c)
    if (!isr_dac_GetState()){  // enable the dac isr if it isn't already enabled
        if (isr_adcAmp_GetState()) {  // User has started cyclic voltammetry while amp is already running, so disable amperometry
            isr_adcAmp_Disable();
            isr_dac_AMP_Disable();
            
        }
        lut_index = 0;  // start at the beginning of the look up table
        lut_value = waveform_CV_lut[0];
        helper_HardwareWakeup();  // start the hardware
        DVDAC_SetValue(lut_value);  // let the electrode equilibriate
        CyDelay(1);  // let the electrode voltage settle
        ADC_SigDel_StartConvert();  // start the converstion process of the delta sigma adc so it will be ready to read when needed
        CyDelay(20);
        
        int16 adc_reading = ADC_SigDel_GetResult16();  // Hack, get first adc reading, timing element doesn't reverse for some reason
        isr_dac_Enable();  // enable the interrupts to start the dac
        isr_adc_Enable();  // and the adc
    }

}


/*  RESET DEVICE
*   \brief: Stop all operations by disabling all the isrs and reset the look up table index to 0
*   \Parameters: NONE
*   \Return: NONE
*/
void user_reset_device(void) {
    isr_dac_Disable();
    isr_adc_Disable();
    isr_adcAmp_Disable();
    helper_HardwareSleep();
    
    lut_index = 0;  
}


/*  LUT MAKE LINE
*   \brief: Sets the PWM that is used as the isr timer
*   \Parameters:
*       @param scan_rate: slope of the triangle waveform
*   \Return: NONE
*/
void user_set_isr_timer(uint16 scan_rate) {
    
    PWM_isr_Wakeup();
    uint16 new_period = 1000/scan_rate;
    PWM_isr_WritePeriod((new_period*PWM_PERIOD_10_ms)/10); //that's needed because a period value of 2399 for the PMW corresponds to 10 msec
    PWM_isr_WriteCompare(new_period/2+1); //keep a DC value of 50%
    PWM_isr_Sleep();
}


/*  CHRONO LUT MAKER
*   \brief: Makes a look up table that will run a chronoamperometry experiment.
*   \Parameters:
*       @param potential_max: max pulse voltage
*   \Return: NONE
*/
void user_chrono_lut_maker(int16 potential_max) {
    PWM_isr_Wakeup();
    
    PWM_isr_WritePeriod(PWM_PERIOD_10_ms*10);  //period of the PWM fixed at 10 ms for a chronoamperometry experiment
    PWM_isr_WriteCompare((PWM_PERIOD_10_ms*10)/2);
    LUT_MakePulse(0, potential_max); //it creates a look up table for chronoamperometry based on these values of baseline and pulse voltage
    lut_value = waveform_amp_lut[0];  // setup the dac so when it starts it will be at the correct voltage
                
    PWM_isr_Sleep();

}


/*  CHRONO LUT MAKER
*   \brief: Makes a look up table for CV.
*   \Parameters: NONE
*   \Return: lut_length: length of the CV look up table
*/
uint16 user_lookup_table_maker(void) {
    PWM_isr_Wakeup();

    uint16 lut_length;

    lut_length = LUT_MakeTriangle_Wave(start_dac_value, end_dac_value); // these values are loaded from the EEPROM or updated by the clinician
     
    lut_value = waveform_CV_lut[0];  // Initialize for the start of the experiment
    PWM_isr_Sleep();
    
    return lut_length;
}


/*  RUN AMPEROMETRY
*   \brief: runs the chronoamperometry test.
*   \Parameters: NONE
*   \Return: NONE
*/
void user_run_amperometry(void) {
    helper_HardwareWakeup();  
    if (!isr_adcAmp_GetState()) {  // enable isr if it is not already
        if (isr_dac_GetState()) {  // User selected to run amperometry but a CV is still running 
            isr_dac_Disable();
            isr_adc_Disable();
        }
    }
    uint16 dac_value = 0;  // Start by setting 0 to the DAC
    lut_index = 0;
    DVDAC_SetValue(dac_value);
    CyDelay(1);
    ADC_SigDel_StartConvert(); // looh at the isr_adcamp in the main.c for the actual operation 
    CyDelay(20);
    isr_adcAmp_Enable();
    isr_dac_AMP_Enable();
}

/* [] END OF FILE */

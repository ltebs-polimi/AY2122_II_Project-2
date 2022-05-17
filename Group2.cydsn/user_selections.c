/*******************************************************************************
* File Name: user_selections.c
*
* Description:
*  Make some of functions the user can call through the UART. 
*  Only for the longer functions to keep main clearer
*
*********************************************************************************/

#include <project.h>

#include "user_selections.h"
#include "InterruptRoutines.h"
#include "lut_functions.h"




/******************************************************************************
* Function Name: user_start_cv_run
*******************************************************************************
*
* Summary:
*  Start a cyclic voltammetry experiment.  The look up table in waveform_CV_lut should
*  already be created.  If the dac isr is already running this will not start and throws
*  and error through the USB.  
*  
* Global variables:
*  uint16 lut_value: value gotten from the look up table that is to be applied to the DAC
*  uint16 lut_index: current index of the look up table
*  uint16 waveform_CV_lut[]:  look up table of the waveform to apply to the DAC
*
* Parameters:
*  None
*
* Return:
*  Starts the isr's used to perform an experiment else if the dac is already running,
*  possibly because another experiment is already running, return an error through the USB
*
*******************************************************************************/

// It is used to setup everything that is needed for a CV scan, then the look up table is actually scanned in the dac isr (see main.c)
void user_start_cv_run(void){
    if (!isr_dac_GetState()){  // enable the dac isr if it isn't already enabled
        if (isr_adcAmp_GetState()) {  // User has started cyclic voltammetry while amp is already running, so disable amperometry
            isr_adcAmp_Disable();
        }
        lut_index = 0;  // start at the beginning of the look up table
        lut_value = waveform_CV_lut[0];
        helper_HardwareWakeup();  // start the hardware
        DVDAC_SetValue(lut_value);  // let the electrode equilibriate
        CyDelay(1);  // let the electrode voltage settle
        ADC_SigDel_StartConvert();  // start the converstion process of the delta sigma adc so it will be ready to read when needed
        CyDelay(20);
        //ADC_array[0].data[lut_index] = ADC_SigDel_GetResult16();  // Hack, get first adc reading, timing element doesn't reverse for some reason
        isr_dac_Enable();  // enable the interrupts to start the dac
        isr_adc_Enable();  // and the adc
    }

}

/******************************************************************************
* Function Name: user_reset_device
*******************************************************************************
*
* Summary:
*  Stop all operations by disabling all the isrs and reset the look up table index to 0
*  
* Global variables:
*  uint16 lut_index: current index of the look up table
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void user_reset_device(void) {
    isr_dac_Disable();
    isr_adc_Disable();
    isr_adcAmp_Disable();
    helper_HardwareSleep();
    
    lut_index = 0;  
}



/******************************************************************************
* Function Name: user_set_isr_timer
*******************************************************************************
*
* Summary:
*  Set the PWM that is used as the isr timer
* 
* Parameters:
*  uint8 command
*
* Explanation:
* the look up table for CV is created with all values in mV between the start and the end value
* Then this look up table is scanned in the interrupt of the DAC a value at a time based on the isr triggered by the PWM
* By setting a proper period for PWM, the look up table will be scanned with different speeds
* ex. scan rate=100 mV/sec --> PWM period = 1000 (i.e. 1 sec) / 100 = 10 ms (i.e. 100Hz) --> look up table is scanned 100 times every second
*  
* Return:
*  Set the period register of the pwm 
*
*******************************************************************************/


void user_set_isr_timer(uint16 scan_rate) {
    
    PWM_isr_Wakeup();
    uint16 new_period = 1000/scan_rate;
    PWM_isr_WritePeriod((new_period*PWM_PERIOD_10_ms)/10); //that's needed because a period value of 2399 for the PMW corresponds to 10 msec
    PWM_isr_WriteCompare(new_period/2+1); //keep a DC value of 50%
    PWM_isr_Sleep();
}

/******************************************************************************
* Function Name: user_chrono_lut_maker
*******************************************************************************
*
* Summary:
*  Make a look up table that will run a chronoamperometry experiment.  Hackish now
* 
*
*  
* Global variables:
*  uint16 lut_value: value gotten from the look up table that is to be applied to the DAC
*  uint16 waveform_amp_lut[]:  look up table of the waveform to apply to the DAC
*  
* Return:
*  4000 - how long the look up table will be
*
*******************************************************************************/

void user_chrono_lut_maker() {
    PWM_isr_Wakeup();
    
    PWM_isr_WritePeriod(PWM_PERIOD_10_ms);  //period of the PWM fixed at 10 ms for a chronoamperometry experiment
    LUT_MakePulse(0, potential_max_current); //it creates a look up table for chronoamperometry based on these values of baseline and pulse voltage
    lut_value = waveform_amp_lut[0];  // setup the dac so when it starts it will be at the correct voltage
                
    PWM_isr_Sleep();

}

/******************************************************************************
* Function Name: user_lookup_table_maker
*******************************************************************************
*
* Summary:
*  Make a look up table for CV
* 
* Global variables:
*  uint16 lut_value: value to get from the look up table and apply to the DAC
*  uint16 waveform_CV_lut[]:  look up table of the waveform to apply to the DAC
*  
* Return:
*  uint16 lut_length - how many look up table elements there are
*
*******************************************************************************/

uint16 user_lookup_table_maker(void) {
    PWM_isr_Wakeup();

    uint16 lut_length;

    lut_length = LUT_MakeTriangle_Wave(start_dac_value, end_dac_value); // these values are loaded from the EEPROM or updated by the clinician
     
    lut_value = waveform_CV_lut[0];  // Initialize for the start of the experiment
    PWM_isr_Sleep();
    
    return lut_length;
}

/******************************************************************************
* Function Name: user_run_amperometry

*******************************************************************************/

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
    
    ADC_SigDel_StartConvert(); // looh at the isr_adcamp in the main.c for the actual operation 
    CyDelay(15);
    isr_adcAmp_Enable();

}


/* [] END OF FILE */
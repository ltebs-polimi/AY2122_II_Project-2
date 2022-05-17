/*********************************************************************************
* File Name: InterruptRoutines.c
*
* Description:
* Interrupt service routines definition
*
*********************************************************************************/

#include "InterruptRoutines.h"
#include "project.h"
#include "stdio.h"
#include "stdlib.h"
// local files
#include "Calibration.h"
#include "DVDAC.h"
#include "Global_variables.h"
#include "Helper_functions.h"

char UART_str[64];


uint8 AMux_channel_select = 0;  // Let the user choose to use the two electrode configuration (set to 0) or a three
// electrode configuration (set to 1) by choosing the correct AMux channel

uint8 adc_recording_channel = 0;  //  A COSA SERVE???


uint8 adc_hold;  // value to hold what adc buffer was just filled  --> SERVE ???


int16 uA_CV_scan[MAX_CV_LUT_SIZE];
int16 uA_amp[MAX_amp_LUT_SIZE];
float32 R_analog_route = 0;

int16 potential_max_current=0;


// ISR used to run a CV scan 
CY_ISR(dacInterrupt)
{
    
    DVDAC_SetValue(lut_value); // this function sets the DVDAC value 
    lut_index++; //incremented in order to scan the look up table
    if (lut_index >= lut_length) { // all the data points have been given
        isr_adc_Disable();
        isr_dac_Disable();

        helper_HardwareSleep();
        lut_index = 0; 
        
        potential_max_current= helper_search_max(uA_CV_scan, lut_length); //we look at what has been saved in the uA_CV_scan and we search for the max current
        
        AMP_ready_Flag=true; //CV has ended and we have calculated a potential value to be applied for chronoamperometry
    }
    
    lut_value = waveform_CV_lut[lut_index]; // take value from the CV look up table 
}


// ISR used to verify the error between the potential of the working electrode and the VIRTUAL GROUND at 2.048 V
CY_ISR(adcInterrupt){
    

    ADC_CV_array[lut_index] = ADC_SigDel_GetResult16(); 

    uA_CV_scan[lut_index] = - ADC_CV_array[lut_index] * uA_per_adc_count; // uA read while performing CV scan, check if the - sign is correct
    
    //send out values with the UART for the CV graph --> TO BE ADDED
}


// ISR used to get the data from the ADC in case of amperometry measurement
CY_ISR(adcAmpInterrupt){
    
    DVDAC_SetValue(lut_value); // this function sets the DVDAC value 
    uA_amp[lut_index] = ADC_SigDel_GetResult16() * uA_per_adc_count; //Read the current 
    
    //send out values with the UART for the AMP graph --> TO BE ADDED
    
    lut_index++;  
    if (lut_index >= MAX_amp_LUT_SIZE) { // all the data points have been given
        isr_adcAmp_Disable();

        helper_HardwareSleep();
        lut_index = 0;
        
        // Set a flag to indicate that the amperometry has ended
        // Call a function to convert (based on a calibration curve) the current at a certain time instant into a glucose concentration
       
    }
    lut_value = waveform_amp_lut[lut_index]; // take value from the AMP look up table
}


/* [] END OF FILE */

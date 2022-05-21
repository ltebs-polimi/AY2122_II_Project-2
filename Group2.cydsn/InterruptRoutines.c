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


uint8 AMux_channel_select = 0;  // Let the user choose to use the two electrode configuration (set to 0) or a three
// electrode configuration (set to 1) by choosing the correct AMux channel

uint8 adc_recording_channel = 0;  //  A COSA SERVE???


uint8 adc_hold;  // value to hold what adc buffer was just filled  --> SERVE ???


float uA_CV_scan[MAX_CV_LUT_SIZE];
float uA_amp;
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

        len= snprintf(str, sizeof(str), "Potential max current: %d\r\n", potential_max_current);
        UART_Debug_PutString(str);
        
        AMP_ready_Flag=true; //CV has ended and we have calculated a potential value to be applied for chronoamperometry
    }
    
    lut_value = waveform_CV_lut[lut_index]; // take value from the CV look up table 
    
    //len= snprintf(str, sizeof(str), "applied potential: %d\r\n", lut_value);
    //UART_Debug_PutString(str);
        
    
}


// ISR used to verify the error between the potential of the working electrode and the VIRTUAL GROUND at 2.048 V
CY_ISR(adcInterrupt){
    
    
    int16 valore_adc_mv_CV= ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
    float current_CV= (float)(-1)*(valore_adc_mv_CV)/20.0;
   
    uint16_t current_int_CV = current_CV;
    uint8_t current_int_CV_H = current_int_CV >>8 ;
    uint8_t current_int_CV_L = current_int_CV &(0xFF);
    
    uint8_t potential_CV_H = lut_value >> 8 ;
    uint8_t potential_CV_L = lut_value & (0xFF);
    
    
    len= snprintf(str, sizeof(str), "uA ADC read: %.8f\r\n", current_CV);
    UART_Debug_PutString(str);
    
    len= snprintf(str, sizeof(str), "lut_value: %d\r\n", lut_value);
    UART_Debug_PutString(str);

    
    //send out values with the UART for the CV graph --> TO BE ADDED
 
//    UART_buffer_CV[0]= 'A';
//    UART_buffer_CV[1]= current_int_CV_H;
//    UART_buffer_CV[2]= current_int_CV_L;
//    UART_buffer_CV[3]= potential_CV_H;
//    UART_buffer_CV[4]= potential_CV_L;
//    UART_buffer_CV[5]= 'z';
//
//    UART_Debug_PutArray(UART_buffer_CV, TRANSMIT_CV_SIZE);

    len= snprintf(str, sizeof(str), "A%ub%uc%ud%uz", current_int_CV_H, current_int_CV_L, potential_CV_H, potential_CV_L);
    UART_PutString(str);
    UART_Debug_PutString(str);
    
    
}


// ISR used to get the data from the ADC in case of amperometry measurement
CY_ISR(adcAmpInterrupt){
    
    DVDAC_SetValue(lut_value); // this function sets the DVDAC value 
    
    int16 valore_adc_mv_AMP= ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
    uA_amp= (float)(-1)*(valore_adc_mv_AMP)/20.0;
    
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

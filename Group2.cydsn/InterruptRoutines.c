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
#include "DVDAC.h"
#include "Global_variables.h"
#include "Helper_functions.h"



float uA_CV_scan[MAX_CV_LUT_SIZE];
float uA_amp;
float32 R_analog_route = 0;

float uA_CV_scan[MAX_CV_LUT_SIZE];
float uA_amp = 0.0;
float current_CV = 0.0;
float array_current_CV[10];
float average_MA=0.0;
float average_MA_first=0.0;
int16 potential_max_current=0;
int counter_amperometry = 0;
int first_time=1;
int16 valore_adc_mv_CV=0;

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
    
    len= snprintf(str, sizeof(str), "applied potential: %d\r\n", lut_value);
    UART_Debug_PutString(str);
        
    
}

// ISR used to verify the error between the potential of the working electrode and the VIRTUAL GROUND at 2.048 V
CY_ISR(adcInterrupt){
   
    if(lut_index<=9){
        
        valore_adc_mv_CV = ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
        current_CV = (float)(-1)*(valore_adc_mv_CV)/20.0;
        array_current_CV[lut_index]=current_CV;
        average_MA_first += current_CV;
    
    }else{
    
        if(first_time){
            
            average_MA_first/=10.0; 
            
            first_time=0;
        
        }else{
        
            valore_adc_mv_CV= ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
            current_CV= (float)(-1)*(valore_adc_mv_CV)/20.0;
        
            for(int k=0; k<9; k++){
                array_current_CV[k]=array_current_CV[k+1];
                average_MA += array_current_CV[k+1];
            }
            array_current_CV[9]=current_CV;
            average_MA+=current_CV;
            average_MA/=10.0;
            
            
            len= snprintf(str, sizeof(str), "uA ADC read: %.8f\r\n", average_MA);
            UART_Debug_PutString(str);
            
            len= snprintf(str, sizeof(str), "lut_value: %d\r\n", lut_value);
            UART_Debug_PutString(str);

            
            //send out values with the UART for the CV graph --> int values
            len= snprintf(str, sizeof(str), "A%.2fb%uz", average_MA, lut_value);
            UART_PutString(str);
            UART_Debug_PutString(str);
            
            
        }
        
    
    }

    
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

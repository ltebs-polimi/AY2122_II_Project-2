/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * -------------------- INTERRUPT ROUTINES (source) -----------------------
 * Interrupt service routines definition
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#include "InterruptRoutines.h"
#include "project.h"
#include "stdio.h"
#include "stdlib.h"
// local files
#include "DVDAC.h"
#include "Globals.h"
#include "Helper_functions.h"
#include "OLED_Driver.h"



// ISR used to run a CV scan 
CY_ISR(dacInterrupt)
{

    DVDAC_SetValue(lut_value); // this function sets the DVDAC value 
    lut_index++; //incremented in order to scan the look up table
    
    len= snprintf(str, sizeof(str), "Max_relativo: %.2f\r\n", max_rel);
    UART_DEBUG_PutString(str);    
    
    if (lut_index >= lut_length) { // all the data points have been given
        isr_adc_Disable();
        isr_dac_Disable();

        helper_HardwareSleep();
        lut_index = 0; 
        

        len= snprintf(str, sizeof(str), "Potential max current: %d\r\n", potential_max_current);
        UART_DEBUG_PutString(str);
        
        AMP_ready_Flag=true; //CV has ended and we have calculated a potential value to be applied for chronoamperometry
        CV_ready_Flag=false;
        CV_finished_flag=true;
        lut_index = 0; 
    }
    
    lut_value = waveform_CV_lut[lut_index]; // take value from the CV look up table 
    
    //len= snprintf(str, sizeof(str), "applied potential: %d\r\n", lut_value);
    //UART_DEBUG_PutString(str);
        
    
}


// ISR used to verify the error between the potential of the working electrode and the VIRTUAL GROUND at 2.048 V
CY_ISR(adcInterrupt){
   
    average_MA_old=average_MA;
    
    if(lut_index<=9){
        
        valore_adc_mv_CV = ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
        current_CV = (float)(-1)*(valore_adc_mv_CV)/20.0;
        
        
        //len= snprintf(str, sizeof(str), "adc_mv: %d    current_CV:%f\r\n", valore_adc_mv_CV, current_CV);
        //UART_DEBUG_PutString(str);
        
        array_current_CV[lut_index]=current_CV;
        average_MA_first += current_CV;
    
    }else{
    
        if(first_time){
            
            average_MA_first/=10.0; 
            len= snprintf(str, sizeof(str), "A%.2fb%uz", average_MA_first, lut_value);
            UART_BLT_PutString(str);
            
            first_time=0;
        
        }else{
        
            valore_adc_mv_CV= ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
            current_CV= (float)(-1)*(valore_adc_mv_CV)/20.0;
            
            if(average_MA>=average_MA_old && average_MA>=max_rel){
             
                max_rel=average_MA;
                potential_max_current=lut_value;
                
            }
        
            for(int k=0; k<9; k++){
                array_current_CV[k]=array_current_CV[k+1];
                average_MA += array_current_CV[k+1];
            }
            array_current_CV[9]=current_CV;
            average_MA+=current_CV;
            average_MA/=10.0;
            
   
            
            //send out values with the UART for the CV graph --> int values
            len= snprintf(str, sizeof(str), "A%.2fb%uz", average_MA, lut_value);
            UART_BLT_PutString(str);
            //UART_DEBUG_PutString(str);
            
            
        }
        
    
    }

    
}


// ISR used to get the data from the ADC in case of amperometry measurement
CY_ISR(adcAmpInterrupt){
    
    valore_adc_mv_AMP= ADC_SigDel_CountsTo_mVolts(ADC_SigDel_GetResult32());
    uA_amp= (float)(-1)*(valore_adc_mv_AMP)/20.0;
    
    //send out values with the UART for the AMP graph
    if(flag_user_measurement==false){
        len= snprintf(str, sizeof(str), "B%.2fc%.2fz", uA_amp, (float)(lut_index/10.0));
        UART_BLT_PutString(str);
        UART_DEBUG_PutString(str);
    }
}

CY_ISR(adcDacInterrupt){
    
    lut_index_old=lut_index;
            
    DVDAC_SetValue(lut_value); // this function sets the DVDAC value     
    lut_index++;  
    
    if(lut_index_old!=lut_index && flag_GUI_running==false){
     
        OLED_loading();
        
    }
    
    if(lut_index == 100)
    {
        uAMP_10=uA_amp;
        char chronoamp_content[64];
        len = snprintf(chronoamp_content, sizeof(chronoamp_content), "Chronoamp current @10s: %.2f\r\n", uAMP_10);
        UART_DEBUG_PutString(chronoamp_content);
    }
        
    
    if (lut_index >= MAX_amp_LUT_SIZE) { // all the data points have been given
                
        isr_adcAmp_Disable();
        isr_dac_AMP_Disable();

        helper_HardwareSleep();
        lut_index = 0;
        
        // Set a flag to indicate that the amperometry has ended
        // Call a function to convert (based on a calibration curve) the current at a certain time instant into a glucose concentration
       
        finished_chronoAmp=1;
        
        if(flag_GUI_running){
            
            glucose_GUI=(int)((uAMP_10-intercept_calibration)/slope_calibration);
            len= snprintf(str, sizeof(str), "D%dF", glucose_GUI);
            UART_BLT_PutString(str);
            
            
        }else{
            
            glucose_GUI=(int)((uAMP_10-intercept_calibration)/slope_calibration);
            
        }
        
                
    }
    lut_value = waveform_amp_lut[lut_index]; // take value from the AMP look up table
    
}

/*CY_ISR(ISR_battery)
{
    Timer_ReadStatusRegister(); //To reset the timer
    
    btlvl_count++;
    
    //At the power on or every 5 min the battery indicator is updated
    if(btlvl_count == 0 || btlvl_count == 301)
    {
        btlvl_count = 1;
        ADC_SAR_IRQ_Enable();
        ADC_SAR_StartConvert();
        
        while(flag_btlvl_ready == 0);
        bt_level = ADC_SAR_CountsTo_mVolts(bt_level);
        
        //Conversion in percentage
        battery_level_OLED = (bt_level*100)/5;
        flag_btlvl_ready = 0;
    }
}
*/

/* [] END OF FILE */

/*******************************************************************************
* File Name: Helper_functions.c
*
* Description:
*  Functions used by main.
*
*********************************************************************************/

#include "Helper_functions.h"



/******************************************************************************
* Function Name: helper_HardwareSetup
*******************************************************************************
*
* Summary:
*    Setup all the hardware needed for an experiment.  This will start all the hardware
*    and then put them to sleep so they can be awoke for an experiment.  Connect all the 
*    defualt analog muxes
*
*******************************************************************************/


void helper_HardwareSetup(void) {
    helper_HardwareStart();
    helper_HardwareSleep();

    AMux_TIA_input_Init();
    AMux_TIA_resistor_bypass_Init();
    DVDAC_Start(); 
    
    // iniatalize the analog muxes connections 
    AMux_TIA_input_Select(AMux_TIA_working_electrode_ch);  // Connect the working electrode
    AMux_TIA_resistor_bypass_Select(0);  // Start with no extra TIA resistor

}

/******************************************************************************
* Function Name: helper_HardwareStart
*******************************************************************************
*
* Summary:
*    Start all the hardware needed for an experiment.
*
*******************************************************************************/

void helper_HardwareStart(void){  // start all the components that have to be on for a reading
    ADC_SigDel_Start();  
    TIA_Start(); 
    VDAC_TIA_Start();  
    Opamp_Aux_Start();  
    PWM_isr_Start();
    
    //DVDAC is already started
    
}

/******************************************************************************
* Function Name: helper_HardwareWakeup
*******************************************************************************
*
* Summary:
*    Start all the hardware needed for an experiment.
*
*******************************************************************************/

void helper_HardwareWakeup(void){  // wakeup all the components that have to be on for a reading
    ADC_SigDel_Wakeup();
    TIA_Wakeup();
    VDAC_TIA_Wakeup();
    DVDAC_Wakeup();
    CyDelay(1);
    DVDAC_SetValue(lut_value);
    CyDelay(10);
    Opamp_Aux_Wakeup();
    
    PWM_isr_Wakeup();
    
}

/******************************************************************************
* Function Name: helper_HardwareWakeup
*******************************************************************************
*
* Summary:
*    Put to sleep all the hardware needed for an experiment.
*
*******************************************************************************/

void helper_HardwareSleep(void){  // put to sleep all the components that have to be on for a reading
    ADC_SigDel_Sleep();
    DVDAC_Sleep();
    TIA_Sleep();
    VDAC_TIA_Sleep();
    Opamp_Aux_Sleep();
    PWM_isr_Sleep();
    
}

uint16 helper_Convert2Dec(uint8 array[], uint8 len){
    uint16 num = 0;
    for (int i = 0; i < len; i++){
        num = num * 10 + (array[i] - '0'); // to be checked
    }
    return num;
}


/******************************************************************************
* Function Name: helper_search_max
*******************************************************************************
*
* Summary:
*   Search for a max current value (after a CV scan) and find the corresponding potential value
*
*   To be checked:
*   Is it enough to search for the maximum in this way?
*******************************************************************************/

int16 helper_search_max(int16 current_array[], uint16 dimensione_array){
    
    int max_rel=0;
    int potential_max_current=0;
    
    for(int i=1; i< dimensione_array-1; i++){
        
        if(current_array[i] >= current_array[i-1] && current_array[i]>= current_array[i+1]){
            
            if(current_array[i]>=max_rel){
                max_rel=current_array[i];
                potential_max_current= waveform_CV_lut[i]; //in theory waveform_CV_lut is global, but consider to pass it inside the function as an argument
                
            }
            
        }
        
        
    }
    
    return potential_max_current;
    
    
}

/* [] END OF FILE */


/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ---------------------- HELPER FUNCTIONS (source) -----------------------
 * In this file, functions in HELPER FUNCTIONS (header) are implemented.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#include "Helper_functions.h" 

/*  HELPER HARDWARE SETUP
*   \brief: Setup all the hardware needed for an experiment.  This will start all the hardware
*           and then put them to sleep so they can be awoke for an experiment.  Connect all the 
*           defualt analog muxes
*   \Parameters: NONE
*   \Return: NONE
*/
void helper_HardwareSetup(void) {
    helper_HardwareStart();
    helper_HardwareSleep();

    AMux_TIA_resistor_bypass_Init();
    DVDAC_Start(); 
    
    // iniatalize the analog muxes connections 
    AMux_TIA_resistor_bypass_Select(0);  // Start with no extra TIA resistor

}

/*  HELPER HARDWARE START
*   \brief: Start all the hardware needed for an experiment.
*   \Parameters: NONE
*   \Return: NONE
*/
void helper_HardwareStart(void){  // start all the components that have to be on for a reading
    ADC_SigDel_Start();  
    TIA_Start(); 
    VDAC_TIA_Start();  
    Opamp_Aux_Start();  
    PWM_isr_Start();
    //ADC_SAR_Start();
    
    //DVDAC is already started    
}

/*  HELPER HARDWARE WAKEUP
*   \brief: Start/wakes up all the hardware needed for an experiment.
*   \Parameters: NONE
*   \Return: NONE
*/
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

/*  HELPER HARDWARE SLEEP
*   \brief: Put to sleep all the hardware needed for an experiment.
*   \Parameters: NONE
*   \Return: NONE
*/
void helper_HardwareSleep(void){  // put to sleep all the components that have to be on for a reading
    ADC_SigDel_Sleep();
    DVDAC_Sleep();
    TIA_Sleep();
    VDAC_TIA_Sleep();
    Opamp_Aux_Sleep();
    PWM_isr_Sleep();
    
}


/*  HELPER CONVERT TO DECIMAL
*   \brief: Function that converts a 8-bit array into a decimal number.
*   \Parameters:
*       @param array[]: 8-bit array
*       @param len: length of the array
*   \Return: NONE
*/
uint16 helper_Convert2Dec(uint8 array[], uint8 len){
    uint16 num = 0;
    for (int i = 0; i < len; i++){
        num = num * 10 + (array[i] - '0'); // to be checked
    }
    return num;
}

/* [] END OF FILE */

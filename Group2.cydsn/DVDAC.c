/*********************************************************************************
* File Name: DVDAC.c
*
* Description:
*  This file contains the source code for the 12-bit DVDAC
*
*********************************************************************************/

#include "DVDAC.h"
#include "Global_variables.h"

/******************************************************************************
* Function Name: DAC_Start
*******************************************************************************
*
* Summary:
*  Start the correct voltage source.  
*  Figure what source is being used, set the  correct AMux settings and start 
*  the correct source
*
* Parameters:
*
*
*  Global variables:
*  selected_voltage_source:  voltage source that is set to run, 
*      [VDAC_IS_VDAC or VDAC_IS_DVDAC]
*
*******************************************************************************/

void DAC_Start(void) {

    DVDAC_Start();
    dac_ground_value = VIRTUAL_GROUND;  //VIRTUAL_GROUND / 1 mV the value of the DAC that will make 0 V across the working and aux electrodes 

}

/******************************************************************************
* Function Name: DAC_Sleep
*******************************************************************************
*
* Summary:
*  Put to sleep the correct voltage source
*
* Parameters:
*
*
* Global variables:
*  selected_voltage_source:  voltage source that is set to run, 
*      [VDAC_IS_VDAC or VDAC_IS_DVDAC]
*
*******************************************************************************/

void DAC_Sleep(void) {

    DVDAC_Sleep();

}


/******************************************************************************
* Function Name: DAC_Wakeup
*******************************************************************************
*
* Summary:
*  Wake up the correct voltage source
*
* Parameters:
*
*
* Global variables:
*  selected_voltage_source:  voltage source that is set to run, 
*      [VDAC_IS_VDAC or VDAC_IS_DVDAC]
*
*******************************************************************************/

void DAC_Wakeup(void) {

    DVDAC_Wakeup();
}


/******************************************************************************
* Function Name: DAC_SetValue
*******************************************************************************
*
* Summary:
*  Set the value of the correct voltage source
*
* Parameters:
*  uint16 value: number to place in the appropriate DAC
*
* Global variables:
*  selected_voltage_source:  voltage source that is set to run, 
*      [VDAC_IS_VDAC or VDAC_IS_DVDAC]
*
*******************************************************************************/

void DAC_SetValue(uint16 value) {
    

    DVDAC_SetValue(value);

}



/* [] END OF FILE */
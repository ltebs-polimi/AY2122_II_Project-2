/*******************************************************************************
* File Name: calibrate.c
*
* Description:
*  Protocols to calibrate the current measuring circuitry i.e. TIA / delta sigma ADC with an IDAC
*
*********************************************************************************/

#include <project.h>
#include "math.h"
#include <stdio.h>
#include "stdlib.h"

#include "Calibration.h"
#include "Global_variables.h"


const uint16 calibrate_TIA_resistor_list[] = {20, 30, 40, 80, 120, 250, 500, 1000}; //8 possible levels of resistance [in kOhm]
uint16 static ADC_value;
uint16 max_idac_value;
uint16 min_idac_value;
uint16 max_adc_value;
uint16 min_adc_value;

/***************************************
* Forward function references
***************************************/
static void Calibrate_Hardware_Wakeup(void);
static void calibrate_step(uint16 IDAC_value, uint8 IDAC_index);
static void Calibrate_Hardware_Sleep(void);

/******************************************************************************
* Function Name: calibrate_TIA
*******************************************************************************
*
* Summary:
*  Calibrate the TIA circuit each time the current gain settings (for the Opamp in input of the ADC) are changed
*
* Global variables:
*  uint8 TIA_resistor_value_index: index of whick TIA resistor to use, Supplied by USB input
*
*
*
*
*******************************************************************************/

//void calibrate_TIA(uint8 TIA_resistor_value_index, uint8 ADC_buffer_index) {
void calibrate_TIA(void) {
    // The IDAC only 
    
    IDAC_calibrate_Start();
    IDAC_calibrate_SetValue(0);  //set IDAC value to 0 uA
    // start the hardware required
    Calibrate_Hardware_Wakeup();
    CyDelay(100);
    // decide what currents to use based on TIA resistor and ADC buffer settings
    uint16 resistor_value = calibrate_TIA_resistor_list[TIA_resistor_value_index]; // select a resistor value among the list of 8 possible resistors of the TIA

    uint16 IDAC_setting = 0;
    
    // set input current to zero and read ADC
    ADC_SigDel_StartConvert();
    calibrate_step(IDAC_setting, 2); // user defined function to set the IDAC value and then read with the ADC --> in this case 0 current is set
    
    // calculate the IDAC value needed to get a 1 Volt in the ADC
    // the 8000 is because the IDAC has a 1/8 uA per bit and 8000=1000mV/(1/8 uA per bit)
    float32 transfer = 8000./(resistor_value); //it's correct (resistor value is in kohm)
    int transfer_int = (int) transfer;
    if (transfer_int > 250) {  // the TIA needs too much current, reduce needs by half.  Is needed for the 20k resistor setting
        transfer_int /= 2;
    }

    // We set different values for the IDAC: highest, 2nd highest, 2nd loweest and lowest
    IDAC_calibrate_SetPolarity(IDAC_calibrate_SINK); // positive polarity
    calibrate_step(transfer_int, 0);
    calibrate_step(transfer_int/2, 1);
    IDAC_calibrate_SetPolarity(IDAC_calibrate_SOURCE); //negative polarity
    calibrate_step(transfer_int/2, 3);
    calibrate_step(transfer_int, 4);
    IDAC_calibrate_SetValue(0);
    Calibrate_Hardware_Sleep();
    
    max_idac_value= CalibrationBuffer[0]; //look at the calibrate_step function to understand the indexes
    min_idac_value= CalibrationBuffer[4];;
    max_adc_value= CalibrationBuffer[5];
    min_adc_value= CalibrationBuffer[9];
    
    //Linear regression by using the max and min values in order to extract the uA per ADC count
    
    float32 uAs = (float)(max_idac_value + min_idac_value) / 8.0; //sum of IDAC values (in bit) based on the calibrate step function 
    uA_per_adc_count = uAs / (max_adc_value - min_adc_value); // calculate the uA per ADC count value
    
    TIA_Calibration_ended_Flag = true;
    
}

/******************************************************************************
* Function Name: calibrate_step
*******************************************************************************
*
* Summary:
*  Gets a single calibration data point by setting the calibration IDAC and reading
*  the ADC count and saving them in the calibration_array
*
* Parameters:
*  uint16 IDAC_value: value to set the calibration  IDAC to before measuring with the ADC
*  uint8 IDAC_index: index of where in calibration_array to save the IDAC and ADC data
*
* Global variables:
*  calibration_array: array of saved IDAC and ADC values
*
*******************************************************************************/

static void calibrate_step(uint16 IDAC_value, uint8 IDAC_index) {
    
    IDAC_calibrate_SetValue(IDAC_value);
    CyDelay(100);  // allow the ADC to settle
    ADC_value = ADC_SigDel_GetResult16();
    CalibrationBuffer[IDAC_index] = IDAC_value; 
    CalibrationBuffer[IDAC_index+5] = ADC_value;    
        
}

/******************************************************************************
* Function Name: Calibrate_Hardware_Wakeup
*******************************************************************************
*
* Summary:
*  Wakeup all the hardware needed for the calibration routine and set the AMux to 
*  the correct channel
*
*******************************************************************************/

static void Calibrate_Hardware_Wakeup(void) {
    AMux_TIA_input_Select(AMux_TIA_calibrat_ch);
    TIA_Wakeup();
    VDAC_TIA_Wakeup();
    ADC_SigDel_Wakeup();
}

/******************************************************************************
* Function Name: Calibrate_Hardware_Sleep
*******************************************************************************
*
* Summary:
*  Put to sleep all the hardware needed for the calibration routine, stop  
*  the IDAC, and set the AMux to the correct channel
*
*******************************************************************************/

static void Calibrate_Hardware_Sleep(void) {
    AMux_TIA_input_Select(AMux_TIA_measure_ch);
    TIA_Sleep();
    VDAC_TIA_Sleep();
    ADC_SigDel_Sleep();
    IDAC_calibrate_Stop();
}

/* [] END OF FILE */

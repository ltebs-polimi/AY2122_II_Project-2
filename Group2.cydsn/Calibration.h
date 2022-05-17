/*******************************************************************************
* File Name: calibrate.h
*
* Description:
*  This file contains the function prototypes and constants used for
*  the protocols to calibrate a TIA / delta sigma ADC with an IDAC
*
*********************************************************************************/

#if !defined(CALIBRATE_H)
#define CALIBRATE_H

#include "cytypes.h"
#include "math.h"
    
/**************************************
*      Constants
**************************************/

#define AMux_TIA_calibrat_ch 0
#define AMux_TIA_measure_ch 1  
#define Number_calibration_points 5
#define CALIBRATION_BUFFER_SIZE 10 

      
uint16 CalibrationBuffer[CALIBRATION_BUFFER_SIZE];


/***************************************
* Global variables identifier 
***************************************/

uint8 TIA_resistor_value_index;
uint8 ADC_buffer_index;
extern float32 R_analog_route;


/***************************************
*        Function Prototypes
***************************************/  
void calibrate_TIA(void);

#endif
/* [] END OF FILE */

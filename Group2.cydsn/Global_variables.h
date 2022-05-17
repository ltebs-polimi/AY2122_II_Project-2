/*******************************************************************************
* File Name: Global_variables.h
*
* Description:
*  Global variables and structures to use in the project
*
**********************************************************************************/


#if !defined(GLOBAL_VARIABLES_H)
#define GLOBAL_VARIABLES_H
    
#include "cytypes.h"

/**************************************
*        UART INPUT OPTIONS
**************************************/ 

#define CONNECT_TO_COM_PORT             'A'
#define SET_SCAN_RATE                   'B'                      
#define SET_CV_START_VALUE              'C'
#define SET_CV_END_VALUE                'D' 
#define SET_CV_TIME                     'E' 
#define START_CYCLIC_VOLTAMMETRY        'F'
#define RUN_AMPEROMETRY                 'G'

/**************************************
*           FLAGS
**************************************/  
    
uint8 Input_Flag;
uint8 Command_ready_Flag;
uint8_t TIA_Calibration_ended_Flag;
uint8_t CV_ready_Flag;
uint8_t AMP_ready_Flag;
uint8_t Load_EEPROM_Flag;
uint8_t Update_scanrate_Flag;
uint8_t Update_startvalue_Flag;
uint8_t Update_endvalue_Flag;
uint8_t Update_timevalue_Flag;
    
/**************************************
*           LUT Constants
**************************************/  
    
// define how big to make the arrays for the lut     
#define MAX_CV_LUT_SIZE 5000
#define MAX_amp_LUT_SIZE 2000

    
/**************************************
*           API Constants
**************************************/
#define true                        1
#define false                       0
    
#define VIRTUAL_GROUND              2048  
    
#define PWM_PERIOD_10_ms            2399
    
#define AMux_TIA_working_electrode_ch 1
        
/**************************************
*        Global Variables
**************************************/   
    
uint16 dac_ground_value;  // value to load in the DAC 
    
    
/* Make global variables needed for the DAC/ADC interrupt service routines */
uint16 lut_value;  // value need to load DAC
uint16 waveform_CV_lut[MAX_CV_LUT_SIZE];  // look up table to store CV waveform
uint16 waveform_amp_lut[MAX_amp_LUT_SIZE];  // look up table to store chronoamperometry waveform
uint16 lut_index;  // look up table index
uint16_t lut_length; // look up table length
uint16 ADC_CV_array[MAX_CV_LUT_SIZE]; //array to store the ADC readings when performing a CV scan
float32 uA_per_adc_count;

/* Variables for CV */
uint16 start_dac_value;
uint16 end_dac_value;
int16 potential_max_current; //value of potential at which the maximum current is found
uint16 selected_scan_rate;

/* Variables for the received commands via UART */
#define MAX_COMMAND_LENGTH 40

uint8_t command[MAX_COMMAND_LENGTH];   
uint8_t command_lenght;


#endif    
/* [] END OF FILE */

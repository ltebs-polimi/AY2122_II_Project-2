/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * -------------------------- GLOBALS (header) ----------------------------
 * This files contains global variables.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/
#include <project.h>
#include "cytypes.h"

#ifndef _GLOBALS_H
    #define _GLOBALS_H
    
// UART INPUT OPTIONS
    #define CONNECT_TO_COM_PORT             'A'
    #define SET_SCAN_RATE                   'B'                      
    #define SET_CV_START_VALUE              'C'
    #define SET_CV_END_VALUE                'D' 
    #define SET_CV_TIME                     'E' 
    #define CLINICIAN_FETCH                 'F'
    #define START_CYCLIC_VOLTAMMETRY        'G'
    #define RUN_AMPEROMETRY                 'H'
    #define USER_START                      'I'
    #define GUI_START                       'L'
    
// API CONSTANTS
    #define true                             1
    #define false                            0
        
    #define VIRTUAL_GROUND                   2048  
        
    #define PWM_PERIOD_10_ms                 2399
        
    #define AMux_TIA_working_electrode_ch    1
    
// LUT CONSTANTS
    //Define how big to make the arrays for the lut     
    #define MAX_CV_LUT_SIZE 5000
    #define MAX_amp_LUT_SIZE 200
        
// I2C TRANSFER RESULT STATUS
    #define TRANSFER_CMPLT                  0x00u
    #define TRANSFER_ERROR                  0xFFu
    
// FLAGS
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
    uint8_t Button_Flag;
    uint8_t flag_first_chrono;
    
// MAIN GLOBALS
    char state;
    uint8_t button_state;                           //User button state
    uint8_t glucose_concentration;                  //Current glucose concentration value
    uint8_t glucose_concentration_old;              //Previous glucose concentration value
    uint8_t finished_chronoAmp;                     //Flag that signals if the chronoamperometry is finished
    uint8_t chronoAmp_progress;                     //Progress of the chronoamperometry procedure
    uint8_t chronoAmp_progress_old;                 //Previous progress of the chronoamperometry procedure
 
// OLED GLOBALS
    uint8_t power_on;                               //Flag that signals the power on of the device
    uint8_t flag_bluetooth;                         //Flag that monitors bluetooth connection status
    uint8_t flag_bluetooth_old;                     //Previous bluetooth connection status

// RTC USER-CHANGING GLOBALS
    char rtc_content[64];                           //String that contains the time stamp from rtc
    char rtc_content_history[64];                   //String that contains the time stamp from rtc to display the history in Python GUI
    char glucose_content[64];                       //String that contains the measured glucose concentration value
    uint8_t current_seconds;
    uint8_t current_minutes;
    uint8_t current_hours;
    uint8_t current_date;
    uint8_t current_month;
    uint16_t current_year;
    
    uint8_t current_seconds_old;
    uint8_t current_minutes_old;
    uint8_t current_hours_old;
    uint8_t current_date_old;
    uint8_t current_month_old;
    uint16_t current_year_old;

// EEPROM GLOBALS
    uint16_t eeprom_current_address;
    char spacer;                                    //Spacer code for EEPROM memory
    uint8_t n_measures;                             //Number of done measures

// BATTERY GLOBALS
    uint16_t btlvl_count;
    uint16_t bt_level;
    uint8_t flag_btlvl_ready;
    uint8_t battery_level_OLED;

// GENERAL GLOBALS
    uint16 dac_ground_value;  // value to load in the DAc
        
        
    /* Make global variables needed for the DAC/ADC interrupt service routines */
    uint16 lut_value;                               // value need to load DAC
    uint16 waveform_CV_lut[MAX_CV_LUT_SIZE];        // look up table to store CV waveform
    uint16 waveform_amp_lut[MAX_amp_LUT_SIZE];      // look up table to store chronoamperometry waveform
    uint16 lut_index;                               // look up table index
    uint16 lut_index_old;
    uint16_t lut_length;                            // look up table length
    uint16 ADC_CV_array[MAX_CV_LUT_SIZE];           //array to store the ADC readings when performing a CV scan
    float uA_per_adc_count;

    /* Variables for CV */
    uint16 start_dac_value;
    uint16 end_dac_value;
    int16 potential_max_current;                    //value of potential at which the maximum current is found
    uint16 selected_scan_rate;

    /* Variables for the received commands via UART */
    #define MAX_COMMAND_LENGTH 40
    #define TRANMSIT_BUFFER_SIZE 16
    #define PACKET_DIMENSION 4
    #define TRANSMIT_CV_SIZE 6

    uint8_t command[MAX_COMMAND_LENGTH];   
    uint8_t command_lenght;
    char str[64];
    int len;
    char DataBuffer[TRANMSIT_BUFFER_SIZE];
    uint8_t data_packet[PACKET_DIMENSION];
    uint8_t UART_buffer_CV[TRANSMIT_CV_SIZE];
    
    
#endif

/* [] END OF FILE */

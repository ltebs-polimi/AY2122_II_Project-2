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

#ifndef _GLOBALS_H
    #define _GLOBALS_H
    
// I2C TRANSFER RESULT STATUS
    #define TRANSFER_CMPLT                  0x00u
    #define TRANSFER_ERROR                  0xFFu
    
// MAIN GLOBALS
    char state;
    uint8_t button_state;
    uint8_t glucose_concentration;
    uint8_t glucose_concentration_old;

// OLED GLOBALS
    uint8_t power_on;
    uint8_t battery_level;
    uint8_t flag_bluetooth;
    uint8_t flag_bluetooth_old;

// RTC USER-CHANGING GLOBALS
    char rtc_content[64];
    char glucose_content[64];
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
    
#endif

/* [] END OF FILE */

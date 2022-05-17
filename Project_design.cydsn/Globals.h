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

// OLED GLOBALS
    uint8_t power_on;
    uint8_t battery_level;

// RTC USER-CHANGING GLOBALS
    uint8_t current_seconds;
    uint8_t current_minutes;
    uint8_t current_hours;
    uint8_t current_date;
    uint8_t current_month;
    uint16_t current_year;

// EEPROM GLOBALS
    uint16_t eeprom_current_address;
    
#endif

/* [] END OF FILE */

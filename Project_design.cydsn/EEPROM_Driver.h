/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ----------------------- EEPROM Driver (header) -------------------------
 * 512Kbit programmable EEPROM with 8-bit registers. Compatibility from
 * 100KHz to 400KHz. Need of HW setup of pins A0, A1, A2 to determine Chip
 * Select (CS).
 * 64000 Possible register addresses (16-bit addresses).
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/
#include <project.h>
#include <stdio.h>
#include <math.h>

#include "ErrorCodes.h"
#include "Globals.h"

#ifndef _EPPROM_DRIVER_H
    #define _EPPROM_DRIVER_H
    
    #define EEPROM_ADDRESS                      0x50
    #define EEPROM_ADDRESS_WRITE                0xA0        //0b10100000 (bits 1-3 are CS)
    #define EEPROM_ADDRESS_READ                 0xA1        //0b10100001 (bits 1-3 are CS)
        
    #define EEPROM_START_ADDRESS                0x0000      //16-bits addresses
        
// FUNCTIONS DEFINITION
    ErrorCode EEPROM_WriteRegister(uint8_t device_address, uint16_t register_address, uint8_t data);
    ErrorCode EEPROM_ReadRegister(uint8_t device_address, uint16_t register_address, uint8_t* data);
    void save_current_measurement(uint8_t glucose_concentration);
    uint8_t get_measurement_from_memory(uint16_t eeprom_address);
    
#endif

/* [] END OF FILE */

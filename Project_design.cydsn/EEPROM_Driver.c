/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ----------------------- EEPROM Driver (source) -------------------------
 * 
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#include <project.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "EEPROM_Driver.h"

/*------------------------------------------------------------------------
*                       FUNCTIONS IMPLEMENTATION
-------------------------------------------------------------------------*/
/*  EEPROM READ REGISTER
*   \brief: Function that reads slave register content
*   \Parameters:
*       @param device_address: 8-bit slave device address
*       @param register_address: 16-bit address of the register where content is stored
*       @param data: pointer to array where to store read value
*   \Return: error (if reading is successfull or not)
*/
ErrorCode EEPROM_ReadRegister(uint8_t device_address, uint16_t register_address, uint8_t* data){
     
    //The function returns an ACK or NACK.
    uint8_t error = I2CMASTER_MasterSendStart(device_address, I2CMASTER_WRITE_XFER_MODE);                                        
    if (error == I2CMASTER_MSTR_NO_ERROR)
    {
        //Write the register address to be read. HIGH byte first, LOW byte then
        uint8_t MSB = (register_address & 0xFF00)>>8;
        uint8_t LSB = register_address & 0x00FF;
        error = I2CMASTER_MasterWriteByte(MSB);
        error = I2CMASTER_MasterWriteByte(LSB);
        if (error == I2CMASTER_MSTR_NO_ERROR)
        {
            
            error = I2CMASTER_MasterSendRestart(device_address, I2CMASTER_READ_XFER_MODE);
            if (error == I2CMASTER_MSTR_NO_ERROR)
            {
                *data = I2CMASTER_MasterReadByte(I2CMASTER_NAK_DATA); //The NACK is needed otherwise the I2C keeps reading
            }
        }
    }

    I2CMASTER_MasterSendStop();

    return error ? ERROR : NO_ERROR;
}

/*  EEPROM WRITE REGISTER
*   \brief: Function that sets slave register content
*   \Parameters:
*       @param device_address: 8-bit slave device address
*       @param register_address: 16-bit address of the register where content has to be written
*       @param data: pointer to array containing values to be written
*   \Return: error (if writing is successfull or not)
*/
ErrorCode EEPROM_WriteRegister(uint8_t device_address, uint16_t register_address, uint8_t data){   
    
    uint8_t error = I2CMASTER_MasterSendStart(device_address, I2CMASTER_WRITE_XFER_MODE);
    
    if (error == I2CMASTER_MSTR_NO_ERROR)
    {
        // Write Register Address HIGH byte first, LOW byte then
        uint8_t MSB = (register_address & 0xFF00)>>8;
        uint8_t LSB = register_address & 0x00FF;
        error = I2CMASTER_MasterWriteByte(MSB);
        error = I2CMASTER_MasterWriteByte(LSB);
        if (error == I2CMASTER_MSTR_NO_ERROR)
        {
            // Write data byte content
            error = I2CMASTER_MasterWriteByte(data);
        }
    }

    I2CMASTER_MasterSendStop();

    return error ? ERROR : NO_ERROR;
}

/*  EEPROM SAVE CURRENT MEASUREMENT
*   \brief: Function that saves the timestamp of the measurement and 
*           the measurement itself
*   \Parameters:
*       @param glucose concentration: glucose concentration value
*   \Return: NONE
*/
void save_current_measurement(uint8_t glucose_concentration) {
    
    ErrorCode error;
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address, current_seconds);
    if(error == ERROR) {
        UART_1_PutString("\nError in saving seconds in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 1, current_minutes);
    if(error == ERROR) {
        UART_1_PutString("\nError in saving minutes in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 2, current_hours);
    if(error == ERROR) {
        UART_1_PutString("\nError in saving hours in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 3, current_date);
    if(error == ERROR) {
        UART_1_PutString("\nError in saving date in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 4, current_month);
    if(error == ERROR) {
        UART_1_PutString("\nError in saving month in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 5, current_year);
    if(error == ERROR) {
        UART_1_PutString("\nError in saving year in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 6, glucose_concentration); 
    if(error == ERROR) {
        UART_1_PutString("\nError in saving glucose data in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    eeprom_current_address += 7;
}

/*  EEPROM GET MEASUREMENT FROM MEMORY
*   \brief: Function that recovers previous measured values from
*           EEPROM memory
*   \Parameters:
*       @param eeprom_address: 16-bit address where the data is saved
*   \Return: data: value stored in memory
*/
uint8_t get_measurement_from_memory(uint16_t eeprom_address) {
    
    ErrorCode error;
    uint8_t data;
    int len = 0;
    char str[64];
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, eeprom_address, &data);
    
    if(error == ERROR) {
        UART_1_PutString("\nError in reading data from EEPROM memory\r\n");
    }
    
    len = snprintf(str, sizeof(str), "\nFrom memory: %d\r\n\n", data);
    UART_1_PutString(str);
    
    return data;
}

/* [] END OF FILE */

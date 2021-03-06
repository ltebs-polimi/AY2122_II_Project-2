/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ----------------------- EEPROM Driver (source) -------------------------
 * Implementation of the function defined in the EEPROM Driver header
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
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address, current_minutes);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving minutes in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 1, current_hours);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving hours in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 2, current_date);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving date in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 3, current_month);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving month in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 4, current_year8);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving year in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 5, glucose_concentration); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving glucose data in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    /*error = EEPROM_WriteRegister(EEPROM_ADDRESS, eeprom_current_address + 6, spacer); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving spacer in EEPROM memory\r\n");
    }
    CyDelay(50);*/
    
    eeprom_current_address += 6;
    
    
    //EEPROM current address saving
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0000, eeprom_current_address >> 8); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving EEPROM_H address in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0001, eeprom_current_address & 0xFF); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving EEPROM_L address in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    //Saving number of measures
    n_measures++;
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0004, n_measures); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving number of measurements in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0005, 255); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving spacer of measurements in EEPROM memory\r\n");
    }
    CyDelay(50);
}

/*  EEPROM SAVE CV RESULT
*   \brief: Function that saves in the 3rd and 4th positions of the EEPROM the CV result.
*   \Parameters: NONE
*   \Return: NONE
*/
void save_CV_result(){
    ErrorCode error;
    
    len= snprintf(str, sizeof(str), "Potential max current in SAVE FUNCTION: %d\r\n", potential_max_current);
    UART_DEBUG_PutString(str);    
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0002, (potential_max_current >> 8));
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving CV_H result in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0003, (potential_max_current & 0xFF));
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving CV_L result in EEPROM memory\r\n");
    }
    CyDelay(50);
}

/*  EEPROM GET CV RESULT
*   \brief: Function that fetches in the first positions of the EEPROM the CV result.
*   \Parameters: NONE
*   \Return:  
*       @data: signed CV value fetched from memory
*/
int16 get_CV_result() {
    ErrorCode error;
    uint8 data_H;
    uint8 data_L;
    int16 data = 0;
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, 0x0002, &data_H);
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading CV_H result from EEPROM memory\r\n");
    }
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, 0x0003, &data_L);
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading CV_L result from EEPROM memory\r\n");
    }
    
    data = data_L;
    data |= data_H << 8;
    
    len= snprintf(str, sizeof(str), "value taken from EEPROM: %u\r\n", data);
    UART_DEBUG_PutString(str);
    
    return data;    
}

/*  EEPROM GET N_MEASURES
*   \brief: Function that fetches the number of measures done
*   \Parameters: NONE
*   \Return:  
*       @data: unsigned number of samples
*/
uint8 get_n_measures() {
    ErrorCode error;
    uint8 data;
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, 0x0004, &data);
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading n_measures from EEPROM memory\r\n");
    }
    
    len= snprintf(str, sizeof(str), "value taken from EEPROM: %u\r\n", data);
    UART_DEBUG_PutString(str);
    
    return data;    
}

/*  EEPROM GET CURRENT ADDRESS FROM MEMORY
*   \brief: Function that recovers last used address from
*           EEPROM memory
*   \Parameters: NONE
*   \Return: 
*       @data: register address stored in memory
*/
uint16_t get_eeprom_current_address(){
    ErrorCode error;
    uint8 data_H;
    uint8 data_L;
    uint16 data = 0;
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, 0x00, &data_H);
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading EEPROM_H address from EEPROM memory\r\n");
    }
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, 0x01, &data_L);
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading EEPROM_L address from EEPROM memory\r\n");
    }
    
    data = data_L;
    data |= data_H << 8;
    return data; 
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
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading data from EEPROM memory\r\n");
    }
    
    len = snprintf(str, sizeof(str), "\nFrom memory: %d\r\n\n", data);
    UART_DEBUG_PutString(str);
    
    return data;
}

/*  DISPLAY HISTORY
*   \brief: Function that recovers the history of all performed measures
*           and displays them
*   \Parameters: NONE
*   \Return: NONE
*/
void display_history() {
    ErrorCode error;
    uint16_t current_address = get_eeprom_current_address();
    uint8_t samples_number;
    uint8_t data[6];
    int len = 0;
    uint8_t j = 0;
    
    error = EEPROM_ReadRegister(EEPROM_ADDRESS, 0x0004, &samples_number);
    CyDelay(50);
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in reading samples number result from EEPROM memory\r\n");
    }
    
    for(int i=0; i<samples_number; i++)
    {
        for(j = 0; j<6; j++)
        {
            data[j] = get_measurement_from_memory(current_address-i*6-j-1);
            /*if(data[j] == 255)
            {
                exit(0); // it makes you exit from the function
            }*/
            
            if(j == 5)    //6 readings (glucose concentration value and time stamp)
            {
                len = snprintf(rtc_content_history, sizeof(rtc_content_history), "%d-%d-20%d %02d:%02d -- %dZ", data[3], 
                                   data[2], data[1], data[4], data[5], data[0]);
                UART_DEBUG_PutString(rtc_content_history);
                UART_BLT_PutString(rtc_content_history);
            }
            
            if(i==samples_number-1 && j==5){
                UART_DEBUG_PutString("F");
                UART_BLT_PutString("F");
            
            }

            
        }
    }
}

/* [] END OF FILE */

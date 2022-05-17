/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ------------------------- RTC Driver (header) --------------------------
 * This header file contains the functions' declarations for the Real Time
 * Clock driver. Also registers addresses and contents are defined.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/
#include <project.h>
#include <ErrorCodes.h>

#ifndef _RTC_DRIVER_H
    #define _RTC_DRIVER_H

/*------------------------------------------------------------------------
*                        RTC DISPLAY REGISTERS
-------------------------------------------------------------------------*/
    #define RTC_ADDRESS                     0x68        //Can also be 0x57

    #define RTC_SECONDS                     0x00        //Range 0-59. Bits 0-3 are units, bits 4-6 are dozens
    #define RTC_MINUTES                     0x01        //Range 0-59. Bits 0-3 are units, bits 4-6 are dozens
    #define RTC_HOURS                       0x02        //Range 1-12. Bits 0-3 are units, bit 4 is dozen, bit 6 is format hour (12 or 24)
    #define RTC_DAY                         0x03        //Range 1-7. Bits 0-2
    #define RTC_DATE                        0x04        //Range 1-31. Bits 0-3 are units, bits 4-5 are dozens
    #define RTC_MONTH                       0x05        //Range 1-12. Bits 0-3 are units, bit 4 is dozen
    #define RTC_YEAR                        0x06        //Range 0-99. Bits 0-3 are units, bits 4-7 are dozens

// 1 unit alarm (1 second, 1 minute, 1 hour etc.)
    #define RTC_ALARM1_SECONDS              0x07        //Range 0-59. Bits 0-3 are units, bits 4-6 are dozens
    #define RTC_ALARM1_MINUTES              0x08        //Range 0-59. Bits 0-3 are units, bits 4-6 are dozens
    #define RTC_ALARM1_HOURS                0x09        //Range 1-12. Bits 0-3 are units, bit 4 is dozen, bit 6 is format hour (12 or 24)
    #define RTC_ALARM1_DATE                 0x0A        //Range 1-7. Bits 0-2

// 2 unit alarm (2 minutes, 2 hours etc.)
    #define RTC_ALARM2_MINUTES              0x0B        //Range 0-59. Bits 0-3 are units, bits 4-6 are dozens
    #define RTC_ALARM2_HOURS                0x0C        //Range 1-12. Bits 0-3 are units, bit 4 is dozen, bit 6 is format hour (12 or 24)
    #define RTC_ALARM2_DATE                 0x0D        //Range 1-7. Bits 0-2

// SETUP REGISTERS
    #define RTC_CTRL_REG                    0x0E
    #define RTC_CTRL_STS_REG                0x0F
    #define RTC_TEMP_LSB                    0x11
    #define RTC_TEMP_MSB                    0x12

/*------------------------------------------------------------------------
*                        RTC SETTING DATA VALUES
-------------------------------------------------------------------------*/
// DAY DEFINITION
    #define MONDAY                          0x01
    #define TUESDAY                         0x02
    #define WEDNESDAY                       0x03
    #define THURSDAY                        0x04
    #define FRIDAY                          0x05
    #define SATURDAY                        0x06
    #define SUNDAY                          0x07
    
// MONTH DEFINITION
    #define JANUARY                         0x01
    #define FEBRUARY                        0x02
    #define MARCH                           0x03
    #define APRIL                           0x04
    #define MAY                             0x05
    #define JUNE                            0x06
    #define JULY                            0x07
    #define AUGUST                          0x08
    #define SEPTEMBER                       0x09
    #define OCTOBER                         0x10
    #define NOVEMBER                        0x11
    #define DECEMBER                        0x12
    
// YEAR DEFINITION
    #define Y_2022                          0x22        //2000 to be added to get 2022 (range 00-99)        
    #define Y_2023                          0x23
        
    uint8 Current_time[8];

// Main functions
    void set_RTC(uint8_t current_seconds, uint8_t current_minutes, uint8_t current_hours, uint8_t current_date, 
                 uint8_t current_month, uint8_t current_year);
    void rtc_init(uint8_t rtc_address);
    void rtc_read_time(uint8_t rtc_data_register);  
    ErrorCode RTC_ReadRegister(uint8_t device_address, uint8_t register_address, uint8_t* data);
    ErrorCode RTC_WriteRegister(uint8_t device_address, uint8_t register_address, uint8_t data);

// Auxiliary functions
    uint8_t RTC_convert_seconds(uint8_t buffer);
    uint8_t RTC_convert_minutes(uint8_t buffer);
    uint8_t RTC_convert_hours(uint8_t buffer);
    uint8_t RTC_convert_date(uint8_t buffer);
    uint8_t RTC_convert_month(uint8_t buffer);
    uint8_t RTC_convert_year(uint8_t buffer);
        
#endif
/* [] END OF FILE */

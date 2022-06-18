/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ---------------------- HELPER FUNCTIONS (header) -----------------------
 * 
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/
#ifndef _HELPER_FUNCTIONS_H
    #define _HELPER_FUNCTIONS_H

    #include <project.h>
    #include "cytypes.h"
    #include "Globals.h"
    #include "DVDAC.h"
        
//Variables
    extern uint8 selected_voltage_source;
        
//Function Prototypes
    uint8 helper_check_voltage_source(void);
    void helper_set_voltage_source(uint8 selected_voltage_source);
    void helper_Writebyte_EEPROM(uint8 data, uint16 address);
    uint8 helper_Readbyte_EEPROM(uint16 address);

    void helper_HardwareSetup(void);
    void helper_HardwareStart(void);
    void helper_HardwareSleep(void);
    void helper_HardwareWakeup(void);

    uint16 helper_Convert2Dec(uint8 array[], uint8 len);



#endif
/* [] END OF FILE */

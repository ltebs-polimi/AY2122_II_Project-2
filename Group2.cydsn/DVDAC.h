/*********************************************************************************
* File Name: DVDAC.h
*
* Description:
*  This file contains the function prototypes and constants used for
*  the custom DAC, an 8-bit VDAC or DVDAC selectable by the user
*
*********************************************************************************/

#if !defined(DAC_H)
#define DVDAC_H

#include <project.h>
#include "cytypes.h"
#include "Helper_functions.h"
#include "Global_variables.h"

/**************************************
*        AMuX API Constants
**************************************/
    
#define DVDAC_channel 1
#define VDAC_channel 0
    
    
/***************************************
*        Variables
***************************************/     
    
uint8 selected_voltage_source;
    
    
/***************************************
*        Function Prototypes
***************************************/ 
    
void DVDAC_Start(void);
void DVDAC_Sleep(void);
void DVDAC_Wakeup(void);
void DVDAC_SetValue(uint16 value);
    
#endif
/* [] END OF FILE */
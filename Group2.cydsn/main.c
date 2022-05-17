/*********************************************************************************
* File Name: main.c
*
* Description:
*  Main program to use PSoC 5LP as an electrochemcial device
*
* Necessary updates:
*   - Add everything about LCD, EEPROM, RTC
*   - Calibration curve to extract glucose measurement after chronoamperometry
*   - Add SET_TIME_DURATION case for a CV scan 
**********************************************************************************/

#include <project.h>
#include "stdio.h"
#include "stdlib.h"
// local files
#include "Calibration.h"
#include "DVDAC.h"
#include "Global_variables.h"
#include "Helper_functions.h"
#include "InterruptRoutines.h"
#include "user_selections.h"

char DataBuffer[TRANMSIT_BUFFER_SIZE];
uint8 ch_received;
char str[64];

int main(void)
{
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    //Initialize some variables values
    Input_Flag=false;
    Command_ready_Flag=false;
    TIA_Calibration_ended_Flag=false;
    CV_ready_Flag=false;
    AMP_ready_Flag=false;
    Load_EEPROM_Flag=false;
    Update_scanrate_Flag=false;
    Update_startvalue_Flag=false;
    Update_endvalue_Flag=false;
    Update_timevalue_Flag=false;
    
    lut_length=5000; // how long the look up table is,initialize large so when starting isr the ending doesn't get triggered
    command_lenght = 0;
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    helper_HardwareSetup();  //user-defined function to initialize the HW 
    
    
    ADC_SigDel_SelectConfiguration(2, DO_NOT_RESTART_ADC); // select the configuration to be used for the ADC (2 possible configurations --> SERVONO ???)
    
    
    //LOAD VALUES FROM THE EEPROM --> TO BE ADDED (set the Load_EEPROM_Flag=true when ended)
    
    
    //Start the UART (used by the BT) and the corresponding isr
    UART_Start();

    
    //Initialize other isr
    isr_dac_StartEx(dacInterrupt);
    isr_dac_Disable();  
    
    isr_adc_StartEx(adcInterrupt);
    isr_adc_Disable();
    
    isr_adcAmp_StartEx(adcAmpInterrupt);
    isr_adcAmp_Disable();
    
    //TIA calibration
    calibrate_TIA();  // manca gestione della scelta della resistenza da usare per calibrare
    
    
    //Start the watchdog timer --> SERVE DAVVERO??
    CyWdtStart(CYWDT_1024_TICKS, CYWDT_LPMODE_NOCHANGE); // it enables the watchdog timer (period between 2 and 3 secs)
    

    for(;;) {
        CyWdtClear(); //The watchdog must be cleared using the CyWdtClear() function before three ticks of the watchdog timer occur
        
        if (UART_GetRxBufferSize() > 0 && Command_ready_Flag==false) // Continuously check if something is received via UART
        {
            Input_Flag=true;
            uint8_t received_char = UART_GetChar();
            command[command_lenght] = received_char;
            command_lenght += 1;
            
        }
        
        if(Input_Flag==true && UART_GetRxBufferSize()==0){

            if (command[command_lenght-1]=='z') //This is necessary in order to understand if the message has ended (or use a tail as an alternative)
            {   

                Input_Flag=false;
                Command_ready_Flag=true;
                
            }
        }
        
        
        
        //Check if something has been received by the UART 
        if (Command_ready_Flag == true) {
            
            //Switch case based on the first character that is received
            switch (command[0]) { 
            
            case CONNECT_TO_COM_PORT: // 'A' Connect to the BT COM port
                
                sprintf(DataBuffer, "Glucose $$$");
                UART_PutString(DataBuffer);
                
                break;
           
            
                
            case SET_SCAN_RATE: ; // 'B' Set the scan rate (in mV per second) by properly setting the PWM period 
                
                selected_scan_rate = (command[1]<<8) | (command[2]&(0xFF));
                user_set_isr_timer(selected_scan_rate);
                Update_scanrate_Flag=true;
                
                if(Update_scanrate_Flag && Update_startvalue_Flag && Update_endvalue_Flag){
                    
                    
                    CV_ready_Flag = true;
                    
                    // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                    sprintf(DataBuffer, "CV ready");
                    UART_PutString(DataBuffer);
                    
                }
                
                break;
            

            case SET_CV_START_VALUE: // 'C' Set the initial value for the CV scan (in mV)
                
                start_dac_value = (command[1]<<8) | (command[2]&(0xFF));
                Update_startvalue_Flag=true;
                
                if((Update_scanrate_Flag||Update_timevalue_Flag) && Update_startvalue_Flag && Update_endvalue_Flag){
                    
                    CV_ready_Flag = true;
                    
                    // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label                      
                    sprintf(DataBuffer, "CV ready");
                    UART_PutString(DataBuffer);
                }
                
    
                break;
                
            case SET_CV_END_VALUE: // 'D' Set the initial value for the CV scan (in mV)
                
                end_dac_value = (command[1]<<8) | (command[2]&(0xFF));
                
                Update_endvalue_Flag=true;                

                if((Update_scanrate_Flag||Update_timevalue_Flag) && Update_startvalue_Flag && Update_endvalue_Flag){
                    
                    CV_ready_Flag = true;
                    // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                    sprintf(DataBuffer, "CV ready");
                    UART_PutString(DataBuffer);
                
                }
                
                break;             

            case SET_CV_TIME: // 'E' Set the time duration for the CV scan (in seconds)
                
                Update_timevalue_Flag=true;
                uint8 selected_time = command[1];
                
                if(Update_startvalue_Flag && Update_endvalue_Flag && Update_timevalue_Flag){
                    
                    selected_scan_rate = (end_dac_value - start_dac_value)/selected_time;
                    
                    CV_ready_Flag = true;
                    // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                    sprintf(DataBuffer, "CV ready");
                    UART_PutString(DataBuffer);
                
                }
                
                break;    
                
                
            case START_CYCLIC_VOLTAMMETRY: ;  // 'F' Start a cyclic voltammetry experiment
                
                if(CV_ready_Flag==true){
                
                    lut_length = user_lookup_table_maker(); //scan rate, start and initial values should be already set
                    
                    // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                    
                    user_start_cv_run();
                    
                }
                
                break;
            

            case RUN_AMPEROMETRY: ; // 'G' run an amperometric experiment --> set the dac to a certain value
                
                if(AMP_ready_Flag || Load_EEPROM_Flag){
                    
                    user_chrono_lut_maker();   // Create a look up table for chronoamperometry            
                    user_run_amperometry();
                
                }

                break;
                
                
            }  // end of switch statment
            Input_Flag = false;  // turn off input flag because it has been processed
            Command_ready_Flag=false;
            
            for (uint8_t i = 0; i < MAX_COMMAND_LENGTH; i++) //clear the command and wait for the next one
            {
                command[i] = 0;
            }
            command_lenght = 0;
        }
    }  // end of for loop in main
    
}  // end of main

/* [] END OF FILE */

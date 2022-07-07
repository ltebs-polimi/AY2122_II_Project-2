/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * --------------------------------- MAIN ---------------------------------
 * Main file.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

//INCLUDES
#include <project.h>
#include <string.h>
#include <stdio.h>
#include "stdlib.h"

#include "OLED_Driver.h"
#include "RTC_Driver.h"
#include "EEPROM_Driver.h"
#include "ErrorCodes.h"
#include "time.h"
#include "DVDAC.h"
#include "Globals.h"
#include "Helper_functions.h"
#include "InterruptRoutines.h"
#include "user_selections.h"

//STATES DEFINE
#define INITIALIZATION 0
#define IDLE 1
#define LOADING 2
#define DISPLAY_MEASUREMENT 3
#define GUI 4
#define AMPEROMETRY_USER 5
#define STALL 6

//GLOBAL READY TO ACCESS
uint8_t seconds = 0;
uint8_t rtc_data_register;
int16 potential_EEPROM=0;


int main(void)
{
    
    CyDelay(1000);
    
    //Global variables initialization
    state = 0;
    power_on = 1;
    battery_level_OLED = 0;
    flag_bluetooth = 0;
    flag_bluetooth_old = 1;
    flag_first_chrono = 1;
    
    current_seconds_old = 0;
    current_minutes_old = 0;
    current_hours_old = 0;
    current_date_old = 0;
    current_month_old = 0;
    current_year_old = 0;
    
    finished_chronoAmp = 0;
    chronoAmp_progress = 0;
    chronoAmp_progress_old = -1;
    
    glucose_concentration = 100;
    glucose_concentration_old = 0;
    
    btlvl_count = 0;
    flag_btlvl_ready = 0;
    lut_index_old = 1;
    
    spacer = 'A';
   
    CyGlobalIntEnable;
    
    //Initialize Interrupt routines variables
    uA_amp = 0.0;
    current_CV = 0.0;
    average_MA_old=0.0;
    average_MA=0.0;
    average_MA_first=0.0;
    potential_max_current=0;
    counter_amperometry = 0;
    first_time=1;
    valore_adc_mv_CV=0;
    valore_adc_mv_AMP=0;
    max_rel=0.0;
    slope_calibration=0.00929;
    intercept_calibration=0.90714;
    
    //Initialize flags values
    Input_Flag=false;
    Command_ready_Flag=false;
    TIA_Calibration_ended_Flag=false;
    CV_ready_Flag=false;
    CV_finished_flag=false;
    AMP_ready_Flag=false;
    Update_scanrate_Flag=false;
    Update_startvalue_Flag=false;
    Update_endvalue_Flag=false;
    Update_timevalue_Flag=false;
    Button_Flag=true;
    flag_GUI_running=false;
    flag_user_measurement=false;
        
    //Communication protocols start
    I2CMASTER_Start();
    UART_DEBUG_Start();
    
    CyDelay(5);
    
    //Peripherals initialization
    display_init(DISPLAY_ADDRESS);
    rtc_init(RTC_ADDRESS);
        
    int len = 0;
    
    /*If you want to setup the RTC, uncomment the following line and insert the needed informations, in order.
      Seconds, Minutes, Hours, Date, Month, Year
      Then run the program, re-comment the line and re-run the program
    */
    //set_RTC(0x40,0x32,0x09,0x06,JULY,Y_2022);
    
    //RTC debug time check via Terminal (if wrong date, see line 125)
    rtc_read_time(RTC_ADDRESS);
    len = snprintf(rtc_content, sizeof(rtc_content), "%d-%d-%d %02d:%02d\n", current_date, 
               current_month, current_year, current_hours, current_minutes);
    UART_DEBUG_PutString(rtc_content);
    
    uint8_t glucose_concentration_from_memory = 0;
    char flag = 0;
    
    //OLED graphics only for main
    const uint8_t GUI_activated[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x61, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x60, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x60, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x60, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x60, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x20, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xf8, 0x03, 0xf0, 0x00, 0xf8, 0x18, 0x1f, 0x08, 0x0f, 0x80, 0x03, 0xc0, 0xff, 0xff, 
        0xff, 0xff, 0xfc, 0x71, 0xf8, 0x7e, 0xe3, 0xce, 0x3f, 0xbe, 0x3f, 0x9c, 0x73, 0x1e, 0x7f, 0xff, 
        0xff, 0xff, 0xfc, 0x70, 0xf8, 0x7b, 0xc3, 0xce, 0x3f, 0xbe, 0x3f, 0xbc, 0x7a, 0x1e, 0x7f, 0xff, 
        0xff, 0xff, 0xfc, 0x71, 0xf8, 0x73, 0xe0, 0xfe, 0x3f, 0xbe, 0x3f, 0xfc, 0x7f, 0x07, 0xff, 0xff, 
        0xff, 0xff, 0xfc, 0x07, 0xf8, 0x7b, 0xf8, 0x1e, 0x3f, 0xbe, 0x3f, 0xfc, 0x7f, 0xc0, 0xff, 0xff, 
        0xff, 0xff, 0xfc, 0x63, 0xf8, 0x7f, 0xff, 0x0e, 0x3f, 0xbe, 0x3f, 0xfc, 0x7f, 0xf8, 0x7f, 0xff, 
        0xff, 0xff, 0xfc, 0x71, 0xf8, 0x7e, 0xcf, 0x8e, 0x3f, 0xbe, 0x3f, 0xfc, 0x7e, 0x7c, 0x7f, 0xff, 
        0xff, 0xff, 0xfc, 0x78, 0xf8, 0x7c, 0xc7, 0x1f, 0x1f, 0x7e, 0x3f, 0x7c, 0x7e, 0x38, 0xff, 0xff, 
        0xff, 0xff, 0xf0, 0x1c, 0x00, 0x01, 0xf9, 0xff, 0xe3, 0xf8, 0x00, 0xf0, 0x1f, 0xcf, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xfe, 0x38, 0xfc, 0x3f, 0xbf, 0xfc, 0x78, 0x78, 0x7f, 0x7c, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xf8, 0xfe, 0x3c, 0x1f, 0xbf, 0xf1, 0xfe, 0x78, 0x7f, 0x7c, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xf1, 0xfe, 0x3d, 0x87, 0xbf, 0xe3, 0xff, 0x78, 0x7f, 0x7c, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xf1, 0xfe, 0x1d, 0xe3, 0xbf, 0xe3, 0xff, 0xf8, 0x7f, 0x7c, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xf1, 0xfe, 0x3d, 0xf0, 0xbf, 0xe3, 0xf8, 0x78, 0x7f, 0x7c, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xf1, 0xfe, 0x3d, 0xfc, 0x3f, 0xe3, 0xfc, 0x78, 0x7f, 0x7c, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xf8, 0xfe, 0x7d, 0xfe, 0x3f, 0xf1, 0xfc, 0x7c, 0x7e, 0xfc, 0x7f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xfe, 0x01, 0xfd, 0xff, 0xbf, 0xfc, 0x00, 0xfe, 0x01, 0xfc, 0x3f, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0x8f, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0x8f, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    
    uint32_t rval;
    char message[100] = {'\0'};
    
    //display_clear();
    UART_DEBUG_PutString("\r\n**************\r\n");
    UART_DEBUG_PutString("** I2C Scan **\r\n");
    UART_DEBUG_PutString("**************\r\n");
    
    CyDelay(10);
    
    UART_DEBUG_PutString("\n\n   ");
	for(uint8_t i = 0; i<0x10; i++)
	{
        sprintf(message, "%02X ", i);
		UART_DEBUG_PutString(message);
	}
 
    
    // SCAN the I2C BUS for slaves
	for( uint8_t i2caddress = 0; i2caddress < 0x80; i2caddress++ ) {
        
		if(i2caddress % 0x10 == 0 ) {
            sprintf(message, "\n%02X ", i2caddress);
		    UART_DEBUG_PutString(message);
        }
 
		rval = I2CMASTER_MasterSendStart(i2caddress, I2CMASTER_WRITE_XFER_MODE);
        
        if( rval == I2CMASTER_MSTR_NO_ERROR ) // If you get ACK then print the address
		{
            sprintf(message, "%02X ", i2caddress);
		    UART_DEBUG_PutString(message);
		}
		else //  Otherwise print a --
		{
		    UART_DEBUG_PutString("-- ");
		}
        I2CMASTER_MasterSendStop();
	}
	UART_DEBUG_PutString("\n\n");

    
    // PSOC Variables initialization
    lut_length=5000; // how long the look up table is,initialize large so when starting isr the ending doesn't get triggered
    command_lenght = 0;
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    helper_HardwareSetup();  //user-defined function to initialize the HW 
    
    ADC_SigDel_SelectConfiguration(1, DO_NOT_RESTART_ADC); // select the configuration to be used for the ADC (2 possible configurations --> SERVONO ???)

    
    //Start the UART (used by the BT) and the corresponding isr
    UART_BLT_Start();

    
    //Initialize all interrupts
    isr_dac_StartEx(dacInterrupt);
    isr_dac_Disable();  
    
    isr_adc_StartEx(adcInterrupt);
    isr_adc_Disable();
    
    isr_adcAmp_StartEx(adcAmpInterrupt);
    isr_adcAmp_Disable();
    
    isr_dac_AMP_StartEx(adcDacInterrupt);
    isr_dac_AMP_Disable();
    
    //isr_timer_StartEx(ISR_battery);
    
    TIA_SetResFB(TIA_RES_FEEDBACK_20K); //A 20KOhm feedback resistor is chosen for the TIA
    
    //Fetch of previously saved EEPROM last saved value address
    eeprom_current_address = get_eeprom_current_address();
    
    //n_samples fetch
    n_measures = get_n_measures();
    
    //Terminal print debugs. If errors or to reset the memory, see line 294 and comment from line 282 to 285 included
    len = snprintf(rtc_content, sizeof(rtc_content), "N_MEASURES: %d\r\n\n", n_measures);
    UART_DEBUG_PutString(rtc_content);
    
    len = snprintf(rtc_content, sizeof(rtc_content), "EEPROM current address: %d\r\n\n", eeprom_current_address);
    UART_DEBUG_PutString(rtc_content);
    
    //TO RESET MEMORY, UNCOMMENT LINES BELOW
    /*
    n_measures = 0;
    ErrorCode error;
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0004, n_measures); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving number of measurements in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0000, 0x00); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving EEPROM initialization H in EEPROM memory\r\n");
    }
    CyDelay(50);
    
    error = EEPROM_WriteRegister(EEPROM_ADDRESS, 0x0001, 0x06); 
    if(error == ERROR) {
        UART_DEBUG_PutString("\nError in saving EEPROM initialization L in EEPROM memory\r\n");
    }
    
    CyDelay(50);*/

    while(1)
    {
        //CyWdtClear(); //The watchdog must be cleared using the CyWdtClear() function before three ticks of the watchdog timer occur
        
        //OUTER SWITCH CASE
        switch(state) {
            
            //STALL: Measurement done, history open. The PSoC does not do anything
            case STALL:    
            break;
            
            //INITIALIATION: OLED is started and also peripherals are checked
            case INITIALIZATION:
                OLED_welcome_screen();
                
                //Serial terminal debug
                rtc_read_time(RTC_ADDRESS);
                len = snprintf(rtc_content, sizeof(rtc_content), "Secondi: %d\r\n", current_seconds);
                UART_DEBUG_PutString(rtc_content);
                len = snprintf(rtc_content, sizeof(rtc_content), "Minuti: %d\r\n", current_minutes);
                UART_DEBUG_PutString(rtc_content);
                len = snprintf(rtc_content, sizeof(rtc_content), "Ore: %d\r\n", current_hours);
                UART_DEBUG_PutString(rtc_content);
                len = snprintf(rtc_content, sizeof(rtc_content), "Giorno: %d\r\n", current_date);
                UART_DEBUG_PutString(rtc_content);
                len = snprintf(rtc_content, sizeof(rtc_content), "Mese: %d\r\n", current_month);
                UART_DEBUG_PutString(rtc_content);
                len = snprintf(rtc_content, sizeof(rtc_content), "Anno: %d\r\n\n", current_year);
                UART_DEBUG_PutString(rtc_content);
                
                current_year8 = 0xFF & (current_year - 2000);
                
                len = snprintf(rtc_content, sizeof(rtc_content), "%d-%d-%d %02d:%02d:%02d\n", current_date, 
                               current_month, current_year, current_hours, current_minutes, current_seconds);
                UART_DEBUG_PutString(rtc_content);
                
                /*if(flag == 0) {
                    flag = 1;
                    save_current_measurement(glucose_concentration);
                    glucose_concentration_from_memory = get_measurement_from_memory(eeprom_current_address - 8);
                }*/
                
                state = IDLE;

            break;
            
            //IDLE: the microcontroller awaits if the user is connecting to a GUI or presses the 
            //button to use the OLED screen
            case IDLE:
                button_state = Pin_Button_Read();
                //len = snprintf(message, sizeof(message), "%d\n", button_state);
                //UART_DEBUG_PutString(message);
                if(button_state==0) {
                    state = LOADING;
                    Button_Flag = false;
                    display_clear();                    
                }
                
                
                if (UART_BLT_GetRxBufferSize() > 0 && Command_ready_Flag==false) // Continuously check if something is received via UART
                {
                    Input_Flag=true;
                    uint8_t received_char = UART_BLT_GetChar();
                    command[command_lenght] = received_char;                 
                    command_lenght += 1;
                    
                }
                
                if(Input_Flag==true && UART_BLT_GetRxBufferSize()==0){

                    if (command[command_lenght-1]=='z') //This is necessary in order to understand if the message has ended (or use a tail as an alternative)
                    {   

                        Input_Flag=false;
                        Command_ready_Flag=true;
                        display_clear();
                        display_update();
                        rtx_drawBitmap(0, 0, GUI_activated, 128, 64, BLACK, WHITE);
                        display_update();
                        state=GUI;
                        flag_GUI_running=true;
                        len= snprintf(str, sizeof(str), "comando ricevuto in case IDLE: %c\r\n", command[0]);
                        UART_DEBUG_PutString(str); 
                        
                        
                    }
                }
                
                
            
            break;
               
            //LOADING: if the user is not connected to a GUI, this state displays a loading bar while
            //chronoamperometry procedure is performed
            case LOADING:
                
                if(flag_first_chrono == 1)
                {
                    //fetch from EEPROM the needed data
                    potential_EEPROM = get_CV_result();                
                    
                    user_chrono_lut_maker(2000);   // Create a look up table for chronoamperometry            
                    user_run_amperometry();
                    flag_first_chrono = 0;
                }
                
                if(!finished_chronoAmp)
                {
                    /*if(lut_index_old!=lut_index){
                        OLED_loading();
                    }*/
                } else {
                    state = DISPLAY_MEASUREMENT;
                    flag_first_chrono = 1;
                }
                
            break;
            
            //DISPLAY_MEASUREMENT: on the OLED, glucose concentration value is displayed
            case DISPLAY_MEASUREMENT:
                finished_chronoAmp = 0;
                if(power_on){
                    power_on = 0;
                    display_battery_level(battery_level_OLED);

                }
                
                rtc_set_time();
                
                display_bluetooth_connection(flag_bluetooth);
                
                OLED_display_glucose();
                
                OLED_display_indicator();
            
                
                button_state = Pin_Button_Read();
                len = snprintf(message, sizeof(message), "%d\n", button_state);
                UART_DEBUG_PutString(message);
            
            break;
               
        
            //GUI: user connected to a GUI (OLED bypassed)
            case GUI:
                
                
                if (UART_BLT_GetRxBufferSize() > 0 && Command_ready_Flag==false) // Continuously check if something is received via UART
                {
                    Input_Flag=true;
                    uint8_t received_char = UART_BLT_GetChar();
                    command[command_lenght] = received_char;
                    command_lenght += 1;
                    
                }
                
                if(Input_Flag==true && UART_BLT_GetRxBufferSize()==0){

                    if (command[command_lenght-1]=='z') //This is necessary in order to understand if the message has ended (or use a tail as an alternative)
                    {   

                        Input_Flag=false;
                        Command_ready_Flag=true;
                        
                    }
                }
                
                if(CV_finished_flag==true){
                    command[0]='I';
                    CV_finished_flag=false;
                    Command_ready_Flag=true;
                }

                
                //Check if something has been received by the UART 
                if (Command_ready_Flag == true && Button_Flag==true) {  
                    
                    //INNER SWITCH CASE
                    //Switch case based on the first character that is received
                    switch (command[0]) { 
                        
                    //CONNECT_TO_COM_PORT: checks if device is connected to COM port via GUI
                    case CONNECT_TO_COM_PORT: // 'A' Connect to the BT COM port
                        
                        len= snprintf(str, sizeof(str), "comando ricevuto in case GUI: %c\r\n", command[0]);
                        UART_DEBUG_PutString(str);
                        
                        sprintf(DataBuffer, "Glucose $$$");
                        UART_BLT_PutString(DataBuffer);
                        UART_DEBUG_PutString(DataBuffer);

                        break;
                   
                    
                    //SET_SCAN_RATE: receives scan rate value from GUI  
                    case SET_SCAN_RATE:  // 'B' Set the scan rate (in mV per second) by properly setting the PWM period 
                        
                        selected_scan_rate = (command[1]<<8) | (command[2]&(0xFF));
                        user_set_isr_timer(selected_scan_rate);
                        Update_scanrate_Flag=true;
                        
                        UART_DEBUG_PutString("SET SCAN RATE\r\n");
                        
                        len= snprintf(str, sizeof(str), "scan rate: %d\r\n", selected_scan_rate);
                        UART_DEBUG_PutString(str);
                        
                        if(Update_scanrate_Flag && Update_startvalue_Flag && Update_endvalue_Flag){
                            
                            
                            CV_ready_Flag = true;
                            
                            // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                            sprintf(DataBuffer, "CV ready");
                            UART_BLT_PutString(DataBuffer);
                            
                        }
                        
                        break;
                    
                    //SET_CV_START_VALUE: receives CV start value from GUI
                    case SET_CV_START_VALUE: // 'C' Set the initial value for the CV scan (in mV)
                        
                        start_dac_value = (command[1]<<8) | (command[2]&(0xFF));
                        Update_startvalue_Flag=true;
                        
                        UART_DEBUG_PutString("SET INITIAL CV VALUE\r\n");
                        
                        len= snprintf(str, sizeof(str), "Initial CV value: %d\r\n", start_dac_value);
                        UART_DEBUG_PutString(str);
                        
                        if((Update_scanrate_Flag||Update_timevalue_Flag) && Update_startvalue_Flag && Update_endvalue_Flag){
                            
                            CV_ready_Flag = true;
                            
                            // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label                      
                            sprintf(DataBuffer, "CV ready");
                            UART_BLT_PutString(DataBuffer);
                        }
                        
            
                        break;
                     
                    //SET_CV_END_VALUE: receives CV end value from GUI
                    case SET_CV_END_VALUE: // 'D' Set the initial value for the CV scan (in mV)
                        
                        end_dac_value = (command[1]<<8) | (command[2]&(0xFF));
                        
                        Update_endvalue_Flag=true;    
                        
                        
                        UART_DEBUG_PutString("SET FINAL CV VALUE\r\n");
                        
                        len= snprintf(str, sizeof(str), "Final CV value: %d\r\n", end_dac_value);
                        UART_DEBUG_PutString(str);

                        if((Update_scanrate_Flag||Update_timevalue_Flag) && Update_startvalue_Flag && Update_endvalue_Flag){
                            
                            CV_ready_Flag = true;
                            // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                            sprintf(DataBuffer, "CV ready");
                            UART_BLT_PutString(DataBuffer);
                        
                        }
                        
                        break;
                        
                    //SET_CV_TIME: receives CV time value from GUI
                    case SET_CV_TIME: // 'E' Set the time duration for the CV scan (in seconds)
                        
                        Update_timevalue_Flag=true;
                        uint8 selected_time = command[1];
                        
                        if(Update_startvalue_Flag && Update_endvalue_Flag && Update_timevalue_Flag){
                            
                            selected_scan_rate = ((end_dac_value - start_dac_value)/selected_time)*2;
                            
                            CV_ready_Flag = true;
                            // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                            sprintf(DataBuffer, "CV ready");
                            UART_BLT_PutString(DataBuffer);
                        
                        }
                        
                        break;    
                        
                    //CLINICIAN_FETCH: fetches CV value from EEPROM (command received from GUI)
                    case CLINICIAN_FETCH: 
                        
                        //fetch potential value from EEPROM (save it in the correct global variable) 
                        potential_max_current = get_CV_result();
                        
                        len= snprintf(str, sizeof(str), "Potential max current: %d\r\n", potential_max_current);
                        UART_DEBUG_PutString(str);
                        
                        CyDelay(2000);
                        user_chrono_lut_maker(potential_max_current);   // Create a look up table for chronoamperometry            
                        user_run_amperometry();                        
                        
                        break;    
                        
                    //START_CYCLIC_VOLTAMMETRY: starts the CV procedure  
                    case START_CYCLIC_VOLTAMMETRY:   // 'G' Start a cyclic voltammetry experiment
                        
                        if(CV_ready_Flag==true){
                            
                            CyDelay(1000);
                        
                            lut_length = user_lookup_table_maker(); //scan rate, start and initial values should be already set
                            
                            // Send a UART message so that the red "NOT READY LABEL", turns into a green "READY" label
                            
                            user_start_cv_run();
                            
                        }
                        
                        break;
                    
                    //FINISHED_CV: things to be done when CV is finished
                    case FINISHED_CV:
                        
                        save_CV_result();
                        UART_DEBUG_PutString("Saved potential in EEPROM\r\n");
                        
                        potential_max_current = get_CV_result();
                        len= snprintf(str, sizeof(str), "Potential max current: %d\r\n", potential_max_current);
                        UART_DEBUG_PutString(str);
                        
                        break;
                    
                    //RUN_AMPEROMETRY: runs the chronoamperometry procedure
                    case RUN_AMPEROMETRY:  // 'H' run an amperometric experiment --> set the dac to a certain value
                        
                        CyDelay(2000);
                        
                        if(AMP_ready_Flag){
                            
                            user_chrono_lut_maker(potential_max_current);   // Create a look up table for chronoamperometry            
                            user_run_amperometry();
                        
                        }

                        break;
                    
                    //USER_GUI_MEASUREMENT: sends CA result to GUI to display the value
                    case USER_GUI_MEASUREMENT:
                        
                        flag_user_measurement=true;
                        
                        potential_max_current = get_CV_result();
                        CyDelay(2000);
                        user_chrono_lut_maker(potential_max_current);   // Create a look up table for chronoamperometry            
                        user_run_amperometry();
                        
                       
                        break;
                    
                    //HISTORY: sends EEPROM saved values to GUI to show the history of measurements
                    case HISTORY:
                        
                        CyDelay(2000);
                        display_history();
                        state = STALL;
                        
                        break;
                        
                        
                    }  // end of BLT switch statment
                    
                    Input_Flag = false;  // turn off input flag because it has been processed
                    Command_ready_Flag = false;
                    
                    for (uint8_t i = 0; i < MAX_COMMAND_LENGTH; i++) //clear the command and wait for the next one
                    {
                        command[i] = 0;
                    }
                    command_lenght = 0;
                    
                }// end of if for the commands
            
            break;
        } // end of LED switch case
        
        
    } // end of while(1)
} // end of main

/* [] END OF FILE */

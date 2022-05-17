/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ------------------------ OLED Driver (source) --------------------------
 * This file contains functions declared in the OLED header file 
 * implementation. They are divided into core functions to have a working
 * OLED and graphic functions to display things.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/

#include <project.h>
#include <stdlib.h>

#include "font.h"
#include "OLED_Driver.h"

/*------------------------------------------------------------------------
*                        OLED DISPLAY REGISTERS
-------------------------------------------------------------------------*/
// DEVICE ADDRESS
#define DISPLAY_ADDRESS                 0x3D    //It can be also 0x3C

// DISPLAY DIMENSIONS
#define DISPLAY_WIDTH                    128
#define DISPLAY_HEIGHT                   64

// FUNDAMENTAL COMMANDS
#define DISPLAY_ON                      0xAF
#define DISPLAY_RESET                   0x7F
#define DISPLAY_CONTRAST                0x81    //Followed by 0xNN for contrast level (value between 0 and 255)
#define DISPLAY_OFF                     0xAE

#define DISPLAY_ALL_ON_RESUME           0xA4    //RAM content displayed
#define DISPLAY_ALL_ON                  0xA5    //Display all white

#define DISPLAY_NORMAL                  0xA6
#define DISPLAY_INVERSE                 0xA7

// ADDRESSING SETTING COMMANDS
#define DISPLAY_LOWER_NIBBLE            0x00    //Followed by 0x0F
#define DISPLAY_UPPER_NIBBLE            0x10    //Followed by 0x1F
#define DISPLAY_SET_MEMORY              0x20    //Followed by: 00b Horizontal addressing, 01b Vertical addressing, 10b Page addressing
#define DISPLAY_SET_MEMORY_HORIZONTAL   0x00
#define DISPLAY_SET_MEMORY_VERTICAL     0x01
#define DISPLAY_SET_MEMORY_PAGE         0x02
#define DISPLAY_SET_COLUMN_ADDR         0x21    //2 bytes param 0-127 for start and 0-127 for end addresses
#define DISPLAY_SET_PAGE_ADDR           0x22    //2 bytes param 0-7 for start and 0-7 for end addresses

// HARDWARE SETTING COMMANDS
#define DISPLAY_START_LINE              0x40   
#define DISPLAY_SEGREMAP                0xA0    //Segment to column address 0
#define DISPLAY_MULTIPLEX               0xA8
#define DISPLAY_COM_OUTPUT              0xC0    //From COM0 to COM(N-1)
#define DISPLAY_COM_OUTPUT_INV          0xC8    //From COM(N-1) to COM0
#define DISPLAY_COM_PINS                0xDA
#define DISPLAY_OFFSET                  0xD3
#define DISPLAY_SET_COM                 0xDB

// TIMING SETTINGS
//Non so se serva mettere il refresh rate ecc
#define DISPLAY_PRECHARGE               0xD9
#define DISPLAY_CLOCKDIV                0xD5

// CHARGE PUMP SETTINGS
#define DISPLAY_CHARGEPUMP              0x8D
#define DISPLAY_CHARGEPUMP_ON           0x14
#define DISPLAY_CHARGEPUMP_OFF          0x10

// I2C TRANSFER RESULT STATUS
#define TRANSFER_CMPLT                  0x00u
#define TRANSFER_ERROR                  0xFFu

/*------------------------------------------------------------------------
*                         STATIC VARIABLES
-------------------------------------------------------------------------*/
static uint8_t oled_i2c_address;
static uint8_t DISPLAY_buffer[DISPLAY_HEIGHT * DISPLAY_WIDTH / 8 + 1] = {0x40};     //Dimension increased by 1 with respect to pixel number to include 0x40 to submit data in position [0]
static uint8_t* display_buf = DISPLAY_buffer+1;                                     //Pointer to next memory cell
static uint16_t display_buf_size = sizeof(DISPLAY_buffer)-1;                            //-1 because we have to subtract the starting command 0x40

// OLED GRAPHIC LOGOS
const uint8_t POLIMI_logo[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x40, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x08, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x80, 0x44, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x0c, 0xcc, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x07, 0xf4, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x61, 0x48, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x23, 0xc4, 0xb8, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x80, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x84, 0x82, 0x0f, 0x80, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x08, 0xd1, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x80, 0x00, 0x81, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x82, 0x01, 0x01, 0x10, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x89, 0x5c, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x01, 0x50, 0x80, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x80, 0x04, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0d, 0x00, 0x80, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x09, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x09, 0xc0, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x68, 0x42, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x05, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x1a, 0x9a, 0x01, 0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xe0, 0x48, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x61, 0x44, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x79, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x20, 0xc0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xc0, 0xf0, 0x81, 0x3f, 0x3e, 0x0e, 0x20, 0xcc, 0x3c, 0x1e, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xf3, 0xf8, 0x81, 0x3f, 0x3e, 0x3f, 0xb0, 0xcc, 0x7f, 0x3f, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x33, 0x0c, 0x81, 0x0c, 0x20, 0x71, 0x38, 0xcc, 0xe0, 0x71, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x36, 0x0c, 0x81, 0x0c, 0x30, 0x60, 0x3c, 0xcc, 0xc0, 0x60, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xf6, 0x06, 0x81, 0x0c, 0x3e, 0x60, 0x36, 0xcc, 0xc0, 0x40, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xe6, 0x04, 0x81, 0x0c, 0x3c, 0x60, 0x33, 0xcc, 0xc0, 0x60, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x06, 0x0c, 0x81, 0x0c, 0x20, 0x60, 0x31, 0xcc, 0xc0, 0x60, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x03, 0x9c, 0xc1, 0x0c, 0x30, 0x39, 0x30, 0xcc, 0x66, 0x33, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x01, 0xf8, 0xf9, 0x0c, 0x3e, 0x1f, 0x30, 0x4c, 0x3e, 0x1f, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x49, 0x02, 0x22, 0x7c, 0x11, 0xc4, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x49, 0x06, 0x32, 0x42, 0x12, 0x48, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x06, 0xe9, 0x05, 0x2a, 0x42, 0x11, 0xde, 0x70, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0b, 0xa9, 0x0d, 0x26, 0x42, 0x13, 0x52, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x29, 0x0f, 0xa6, 0x46, 0x12, 0x50, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x29, 0xf0, 0xa2, 0x3c, 0x39, 0xce, 0x70, 0x00, 0x00, 0x00, 0x00
};

// RAM SETTINGS
#define GDDRAM_ADDRESS(X,Y)             ((display_buf)+((Y)/8)*(DISPLAY_WIDTH)+(X))

#define GDDRAM_PIXMASK(Y)               (1 << ((Y)&0x07))

#define PIXEL_ON(X,Y)                   (*GDDRAM_ADDRESS(x,y) |= GDDRAM_PIXMASK(y))
#define PIXEL_OFF(X,Y)                  (*GDDRAM_ADDRESS(x,y) &= ~GDDRAM_PIXMASK(y))
#define PIXEL_TOGGLE(X,Y)               (*GDDRAM_ADDRESS(x,y) ^= GDDRAM_PIXMASK(y))

#define pgm_read_byte(addr) (*(const uint8_t *)(addr))


/*------------------------------------------------------------------------
*                       STATIC FUNCTIONS DECLARATION
-------------------------------------------------------------------------*/
static uint32 display_write_buffer(uint8_t* buffer, uint16_t size);
static void rtx_hvLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
static void rtx_rotation_adjust(int16_t* px, int16_t* py);
static void display_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/*------------------------------------------------------------------------
*                       FUNCTIONS IMPLEMENTATION
-------------------------------------------------------------------------*/

/*  DISPLAY INIT
*   \brief: Display initialization function.
            NB: if configuration commands has to be sent, the buffer must
                start with 0x00. Otherwise, if data has to be sent, with
                0x40.
*   \parameters:
*       @param oled_address: 8-bit address identifing the OLED screen
*/
void display_init(uint8_t oled_address) {
    
    oled_i2c_address = oled_address;
    
    uint8_t cmdbuffer[] = {
        0x00,                           //Every time a command sequence is sent the buffer has to start with 0x00
        DISPLAY_OFF,
        DISPLAY_CLOCKDIV,
        0x80,
        DISPLAY_MULTIPLEX,
        0x3F,                           //Reset value 0b00111111
        DISPLAY_OFFSET,
        0x00,                           //No offset
        DISPLAY_START_LINE | 0x0,
        DISPLAY_CHARGEPUMP,
        0x14,                           //Charge pump enabled. 0x10 to disable
        DISPLAY_SET_MEMORY,
        0x00,                           //Horizontal addressing
        DISPLAY_SEGREMAP | 0x1,
        DISPLAY_COM_OUTPUT_INV,
        DISPLAY_COM_PINS,
        0x12,
        DISPLAY_CONTRAST,
        0x7F,                           //Reset value
        DISPLAY_PRECHARGE,
        0xF1,
        DISPLAY_SET_COM,
        0x40,
        DISPLAY_ALL_ON_RESUME,
        DISPLAY_NORMAL,
        DISPLAY_ON
    };
    
    display_write_buffer(cmdbuffer, sizeof(cmdbuffer));
}

/*  DISPLAY WRITE BUFFER
*   \brief: Sends the addresses and content to OLED screen to set it up.
*   \Parameters:
*       @param buffer: data to be sent to the OLED screen
*       @param size: size of @param(buffer)
*   \Return: Transfer result status
*/
static uint32 display_write_buffer(uint8_t* buffer, uint16_t size) {
    
    uint32 status = TRANSFER_ERROR;
    I2CMASTER_MasterSendStart(oled_i2c_address, I2CMASTER_WRITE_XFER_MODE);
    
    for(int i=0; i<size; i++){  
        status=I2CMASTER_MasterWriteByte(buffer[i]);
    }
    I2CMASTER_MasterSendStop();
    return status;
}

/*  DISPLAY SET PIXEL
*   \brief: Function that changes the pixel status (ON/OFF).
*   \Parameters:
*       @param x: x coordinate of the pixel
*       @param y: y coordinate of the pixel
*       @param color: color to be set. Black, white or change with respect to the previous
*/
void display_setPixel(int16_t x, int16_t y, uint16_t color) {
    
    if((x < 0) || (x >= DISPLAY_WIDTH) || (y < 0) || (y >= DISPLAY_HEIGHT))
        return;

    switch(color) {
        case WHITE: 
            PIXEL_ON(x,y);
        break;
            
        case BLACK:
            PIXEL_OFF(x,y);
        break;
            
        case INVERSE: 
            PIXEL_TOGGLE(x,y);
        break;
    }
}

/*  DISPLAY RESET
*   \brief: Function that reset the display buffer content.
*   \Parameters: NONE
*/
void display_clear() {
    memset(display_buf, 0, display_buf_size);       
    DISPLAY_buffer[0] = 0x40;                       //Rewriting 0x40 to send data
}

/*  DISPLAY SET CONTRAST
*   \brief: Function that sets the display contrast.
*   \Parameters:
*       @param contrast: value comprised between 0 and 255 corresponding
*                        to screen contrast value
*/
void set_display_contrast(uint8_t contrast) {
        
    uint8 cmdbuffer[] = {
        0x00,  
        DISPLAY_CONTRAST,
        contrast
    };
    
    display_write_buffer(cmdbuffer, sizeof(cmdbuffer)); 
}

/*  DISPLAY SET STATUS
*   \brief: Function that inverts the pixel values.
*   \Parameters:
*       @param status: defines if the screen has to be normal (0)
*                      or inverted (1)
*/
void display_status(char status) {
    
    uint8_t cmdbuffer[2] = {0x00, 0x00};
    
    switch (status) {
    case 0:
        cmdbuffer[1] = DISPLAY_NORMAL;
    break;
        
    case 1:
        cmdbuffer[1] = DISPLAY_INVERSE;
    break;
    }
    
    display_write_buffer(cmdbuffer, sizeof(cmdbuffer));
}

/*  DISPLAY UPDATE
*   \brief: Function that reset the indexes for column and page
*           and rewrites memory value in the DISPLAY_buffer on
*           the display.
*   \Parameters: NONE
*/
void display_update() {
      
    uint8 cmdbuffer[] = {
        0x00,
        DISPLAY_SET_COLUMN_ADDR,
        0,                      
        DISPLAY_WIDTH-1,
        DISPLAY_SET_PAGE_ADDR,
        0,                      
        7                               //Total of 8 pages in the screen (end)                
    };
    display_write_buffer(cmdbuffer, sizeof(cmdbuffer)); 
    display_write_buffer(DISPLAY_buffer, sizeof(DISPLAY_buffer));
}

/*  SET PIXEL ON
*   \brief: Function that sets the value of pixels to 1 (ON)
*   \Parameters: NONE
*/
void set_pixel_on(uint16_t x, uint16_t y) {
    uint8_t page;
    
    page = y/8;
    *display_buf = *(display_buf + page*DISPLAY_WIDTH + x) | (1 << (0b00000111 & y));   
}

/*  SET PIXEL OFF
*   \brief: Function that sets the value of pixels to 0 (OFF)
*   \Parameters: NONE
*/
void set_pixel_off(uint16_t x, uint16_t y) {
    uint8_t page;
    
    page = y/8;
    *display_buf = *(display_buf + page*DISPLAY_WIDTH + x) & ~(1 << (0b00000111 & y));   
}

static void display_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){

    if(x1 == x2){
        // vertical
        uint8_t* pstart = GDDRAM_ADDRESS(x1,y1);
        uint8_t* pend = GDDRAM_ADDRESS(x2,y2);       
        uint8_t* ptr = pstart;             
        
        while(ptr <= pend){
            
            uint8_t mask;
            if(ptr == pstart){
                // top
                uint8_t lbit = y1 % 8;
                // bottom (line can be very short, all inside this one byte)
                uint8_t ubit = lbit + y2 - y1;
                if(ubit >= 7)
                    ubit = 7;
                mask = ((1 << (ubit-lbit+1)) - 1) << lbit;    
            }else if(ptr == pend){
                // top is always bit 0, that makes it easy
                // bottom
                mask = (1 << (y2 % 8)) - 1;    
            }

            if(ptr == pstart || ptr == pend){
                switch(color){
                    case WHITE:     *ptr |= mask; break;
                    case BLACK:     *ptr &= ~mask; break;
                    case INVERSE:   *ptr ^= mask; break;
                };  
            }else{
                switch(color){
                    case WHITE:     *ptr = 0xff; break;
                    case BLACK:     *ptr = 0x00; break;
                    case INVERSE:   *ptr ^= 0xff; break;
                };  
            }
            
            ptr += DISPLAY_WIDTH;
        }
    }else{
        // horizontal
        uint8_t* pstart = GDDRAM_ADDRESS(x1,y1);
        uint8_t* pend = pstart + x2 - x1;
        uint8_t pixmask = GDDRAM_PIXMASK(y1);    

        uint8_t* ptr = pstart;
        while(ptr <= pend){
            switch(color){
                case WHITE:     *ptr |= pixmask; break;
                case BLACK:     *ptr &= ~pixmask; break;
                case INVERSE:   *ptr ^= pixmask; break;
            };
            ptr++;
        }
    }
}


/*------------------------------------------------------------------------
*                           GRAPHIC FUNCTIONS
-------------------------------------------------------------------------*/
/*  GRAPHIC INITIALIZATION
*   \brief: Function that initializes the graphic settings
*   \Parameters: NONE
*/
void rtx_init(){
    rotation = 0;
    cursor_y = cursor_x = 0;
    textsize = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap = 1;
}

/*  SET CURSOR POSITION
*   \brief: Function that stores position from where begin to draw the
*           graphic
*   \Parameters: 
*       @param x: x starting position
*       @param y: y starting position
*/
void rtx_setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

void rtx_setTextColor(uint16_t color) {
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = color;
}

///////////////////
static void rtx_rotation_adjust(int16_t* px, int16_t* py){

    int16_t y0 = *py;
    
    switch(rotation){
        case 1:
            *py = *px;
            *px = DISPLAY_WIDTH - y0 - 1;
            break;
        case 2:
            *px = DISPLAY_WIDTH - *px - 1;
            *py = DISPLAY_HEIGHT - *py - 1;
            break;
        case 3:
            *py = DISPLAY_HEIGHT - *px - 1;
            *px = y0;
            break;
    }
}

////////////
static void rtx_hvLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
    
    if( x1 != x2 && y1 != y2 ){
        // neither vertical nor horizontal
        return;
    }    
    
    // bounds check
    if(rotation == 1 || rotation == 3){
        if( x1 < 0 || x1 >= DISPLAY_HEIGHT || x2 < 0 || x2 >= DISPLAY_HEIGHT)
            return;
        if( y1 < 0 || y1 >= DISPLAY_WIDTH || y2 < 0 || y2 >= DISPLAY_WIDTH)
            return;
    }else{
        if( y1 < 0 || y1 >= DISPLAY_HEIGHT || y2 < 0 || y2 >= DISPLAY_HEIGHT)
            return;
        if( x1 < 0 || x1 >= DISPLAY_WIDTH || x2 < 0 || x2 >= DISPLAY_WIDTH)
            return;
    }
    
    rtx_rotation_adjust( &x1, &y1 );
    rtx_rotation_adjust( &x2, &y2 );
    
    // ensure coords are from left to right and top to bottom
    if( (x1 == x2 && y2 < y1) || (y1 == y2 && x2 < x1) ){
        // swap as needed
        int16_t t = x1; x1 = x2; x2 = t;
        t = y1; y1 = y2; y2 = t;
    }
    
    display_line(x1, y1, x2, y2, color);
}

///////////////////////////
void rtx_drawBitmap(int16_t x, int16_t y,
            const uint8_t *bitmap, int16_t w, int16_t h,
            uint16_t color, uint16_t bg) {

  int16_t i, j, byteWidth = (w + 7) / 8;
  
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        rtx_drawPixel(x+i, y+j, color);
      }
      else {
      	rtx_drawPixel(x+i, y+j, bg);
      }
    }
  }
}

/*  SET TEXT SIZE
*   \brief: Function that sets text size
*   \Parameters: 
*       @param size: text size
*/
void rtx_setTextSize(uint8_t size) {
    textsize = (size > 0) ? size : 1;
}

/*  DRAW PIXEL
*   \brief: Function that draws a pixel on the OLED screen
*   \Parameters: 
*       @param x: x pixel position
*       @param y: y pixel position
*       @param color: pixel color
*/
void rtx_drawPixel(int16_t x, int16_t y, uint16_t color) {
    
    if((x < 0) || (x >= DISPLAY_WIDTH) || (y < 0) || (y >= DISPLAY_HEIGHT))
        return;
    
    rtx_rotation_adjust(&x, &y);

    display_setPixel(x,y,color);
}

/*  DRAW LINE
*   \brief: Function that draws a line on the screen
*   \Parameters: 
*       @param x: x pixel starting position
*       @param y: y pixel starting position
*       @param w: rectangle width
*       @param h: rectangle height
*       @param color: rectangle color
*/
void rtx_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
 
    if( x0 == x1 || y0 == y1 ){
        // vertical and horizontal lines can be drawn faster
        rtx_hvLine( x0, y0, x1, y1, color );
        return;
    }
    
    int16_t t;
    
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if( steep ){
        t = x0; x0 = y0; y0 = t;
        t = x1; x1 = y1; y1 = t;
    }
    if( x0 > x1 ){
        t = x0; x0 = x1; x1 = t;
        t = y0; y0 = y1; y1 = t;
    }
    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep;
    if( y0 < y1 ){
        ystep = 1;
    }else{
        ystep = -1;
    }
    for( ; x0<=x1; x0++ ){
        if( steep ){
            rtx_drawPixel( y0, x0, color );
        }else{
            rtx_drawPixel( x0, y0, color );
        }
        err -= dy;
        if( err < 0 ){
            y0 += ystep;
            err += dx;
        }
    }
}

/*  FILL RECTANGLE
*   \brief: Function that draws a rectangle on the OLED screen
*   \Parameters: 
*       @param x: x pixel starting position
*       @param y: y pixel starting position
*       @param w: rectangle width
*       @param h: rectangle height
*       @param color: rectangle color
*/
void rtx_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    int16_t i = 0;
    if(h > w){
        for(i = x ; i < x+w ; i++){
            rtx_drawLine(i, y, i, y+h-1, color);
        }
    }else{
        for(i = y ; i < y+h ; i++){
            rtx_drawLine( x, i, x+w-1, i, color);
        }
    }
}   

/*  DRAW CHARACTER
*   \brief: Function that draws characters on the screen
*   \Parameters: 
*       @param x: x char position
*       @param y: y char position
*       @param c: char to be drawn
*       @param color: text color
*       @param bg: background color
*       @param size: text size
*/
void rtx_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    if( (x >= DISPLAY_WIDTH) || // Clip right
        (y >= DISPLAY_HEIGHT) || // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0)) // Clip top
        return;

    int8_t i = 0;
    for(i = 0 ; i < 6 ; i++){
        uint8_t line;
        if(i == 5 )
            line = 0x0;
        else
           line = font[(c*5)+i];
        int8_t j = 0;
        for(j = 0; j < 8 ; j++){
            if(line & 0x1){
                if(size == 1) // default size
                    rtx_drawPixel(x+i, y+j, color);
                else { // big size
                    rtx_fillRect(x+(i*size), y+(j*size), size, size, color);
                }
            } else if(bg != color){
                if(size == 1) // default size
                    rtx_drawPixel(x+i, y+j, bg);
                else { // big size
                    rtx_fillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
}

/*  WRITE
*   \brief: Function prints a value
*   \Parameters: 
*       @param size: channel
*/
void rtx_write(uint8_t ch){
    if(ch == '\n'){
        cursor_y += textsize*8;
        cursor_x = 0;
    }else if(ch == '\r'){
        // skip em
    }else{
        rtx_drawChar(cursor_x, cursor_y, ch, textcolor, textbgcolor, textsize);
        cursor_x += textsize*6;
        if(wrap && (cursor_x > (DISPLAY_WIDTH - textsize*6))){
            cursor_y += textsize*8;
            cursor_x = 0;
        }
    }
}

/*  PRINT
*   \brief: Function prints a value
*   \Parameters: 
*       @param size: text
*/
void rtx_print(const char* s){
    
    unsigned int len = strlen(s);
    unsigned int i = 0; 
    for(i = 0 ; i < len ; i++){
        rtx_write(s[i]);
    }
}

/*  PRINT AND GO TO NEXT LINE
*   \brief: Function prints a value and moves to next line
*   \Parameters: 
*       @param size: text
*/
void rtx_println(const char* s){ 
    rtx_print(s); 
    rtx_write('\n');
}

/*------------------------------------------------------------------------
*                     GRAPHIC INITIALIZATION SCREENS
-------------------------------------------------------------------------*/
/*  OLED WELCOME SCREEN
*   \brief: Function that draws welcome graphics
*   \Parameters: NONE
*   \Return: NONE
*/
void OLED_welcome_screen() {
    
    int i=0, j=0;
    
    display_clear();
    display_update();
    rtx_drawBitmap(0, 0, POLIMI_logo, 128, 64, WHITE, BLACK);
    display_update();
    
    for(i=0; i<128; i++) {
        for(j=0; j<64; j++) {
            display_setPixel(i,j,INVERSE); 
        }
        display_update();
    }
    
    display_clear();
    for(i=0; i<=110; i++) {
        rtx_drawPixel(i, 5, WHITE);
        rtx_drawPixel(127-i, 59, WHITE);
        display_update();
    }
    
    rtx_setTextSize(2);
    rtx_setTextColor(WHITE);
    rtx_setCursor(5,24);
    rtx_println("GLUCOMETER");
    display_update();
    CyDelay(2000);
    
    display_clear();
    display_update();
    rtx_setTextSize(1);
    rtx_setTextColor(WHITE);
    rtx_setCursor(15,12);
    rtx_println("Press the button");
    rtx_setCursor(40,27);
    rtx_println("to START");
    rtx_setCursor(18,42);
    rtx_println("the measurement");
    display_update();
    CyDelay(1000);
}

void display_battery_level(uint8_t battery_level){
    int i = 0;
    
    display_clear();
    if(battery_level >= 75)
    {
        rtx_drawLine(0, 0, 18, 0, WHITE);
        rtx_drawLine(0, 7, 18, 7, WHITE);
        rtx_drawLine(0, 0, 0, 7, WHITE);
        rtx_drawLine(18, 0, 18, 2, WHITE);
        rtx_drawLine(18, 5, 18, 7, WHITE);
        rtx_drawLine(18, 5, 20, 5, WHITE);
        rtx_drawLine(18, 2, 20, 2, WHITE);
        rtx_drawLine(20, 2, 20, 5, WHITE);
        
        rtx_fillRect(2,2,3,4, WHITE);
        rtx_fillRect(6,2,3,4, WHITE);
        rtx_fillRect(10,2,3,4, WHITE);
        rtx_fillRect(14,2,3,4, WHITE);
        display_update();
    }
    
    if(battery_level>=50 && battery_level<75)
    {
        rtx_drawLine(0, 0, 18, 0, WHITE);
        rtx_drawLine(0, 7, 18, 7, WHITE);
        rtx_drawLine(0, 0, 0, 7, WHITE);
        rtx_drawLine(18, 0, 18, 2, WHITE);
        rtx_drawLine(18, 5, 18, 7, WHITE);
        rtx_drawLine(18, 5, 20, 5, WHITE);
        rtx_drawLine(18, 2, 20, 2, WHITE);
        rtx_drawLine(20, 2, 20, 5, WHITE);
        
        rtx_fillRect(2,2,3,4, WHITE);
        rtx_fillRect(6,2,3,4, WHITE);
        rtx_fillRect(10,2,3,4, WHITE);
        display_update();
    }
    
    if(battery_level>=25 && battery_level<50)
    {
        rtx_drawLine(0, 0, 18, 0, WHITE);
        rtx_drawLine(0, 7, 18, 7, WHITE);
        rtx_drawLine(0, 0, 0, 7, WHITE);
        rtx_drawLine(18, 0, 18, 2, WHITE);
        rtx_drawLine(18, 5, 18, 7, WHITE);
        rtx_drawLine(18, 5, 20, 5, WHITE);
        rtx_drawLine(18, 2, 20, 2, WHITE);
        rtx_drawLine(20, 2, 20, 5, WHITE);
        
        rtx_fillRect(2,2,3,4, WHITE);
        rtx_fillRect(6,2,3,4, WHITE);
        display_update();
    }
    
    if(battery_level>0 && battery_level<25)
    {
        rtx_drawLine(0, 0, 18, 0, WHITE);
        rtx_drawLine(0, 7, 18, 7, WHITE);
        rtx_drawLine(0, 0, 0, 7, WHITE);
        rtx_drawLine(18, 0, 18, 2, WHITE);
        rtx_drawLine(18, 5, 18, 7, WHITE);
        rtx_drawLine(18, 5, 20, 5, WHITE);
        rtx_drawLine(18, 2, 20, 2, WHITE);
        rtx_drawLine(20, 2, 20, 5, WHITE);
        
        rtx_fillRect(2,2,3,4, WHITE);
        display_update();
    }

}




/* [] END OF FILE */

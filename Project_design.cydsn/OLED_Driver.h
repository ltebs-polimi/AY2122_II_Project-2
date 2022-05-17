/* ========================================================================
 *
 * ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY
 * Academic year 2021/22, II Semester
 * Final Project
 *
 * Authors: Group 2
 *
 * ------------------------ OLED Driver (header) --------------------------
 * This file contains functions' declaration for the OLED screen.
 * ------------------------------------------------------------------------
 * 
 * ========================================================================
*/
#include <project.h>

#include "Globals.h"

#ifndef _OLED_DRIVER_H
    #define _OLED_DRIVER_H
        
    #define BLACK 0
    #define WHITE 1
    #define INVERSE 2 
        
    #define DISPLAY_ADDRESS                 0x3D
            
    int16_t cursor_x, cursor_y;
    uint8_t textsize;
    uint8_t wrap;                                           // If set, 'wrap' text at right edge of display
    uint8_t rotation;
    uint16_t textcolor, textbgcolor;

//OLED Setup
    void display_init(uint8_t oled_i2c_address);
    void display_clear(); 
    void display_status(char status);
    void display_update();
    void display_setPixel(int16_t x, int16_t y, uint16_t color);
    void set_pixel_on(uint16_t x, uint16_t y);
    void set_pixel_off(uint16_t x, uint16_t y);

//Graphic functions
    void rtx_init();
    void rtx_setCursor(int16_t x, int16_t y);
    void rtx_setTextColor(uint16_t color);
    void rtx_setTextSize(uint8_t size);
    void rtx_println(const char* s);
    void rtx_print(const char* s);
    void rtx_write(uint8_t ch);
    void rtx_drawChar(int16_t x, int16_t y, unsigned char c,uint16_t color, uint16_t bg, uint8_t size);
    void rtx_drawPixel(int16_t x, int16_t y, uint16_t color);
    void rtx_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void rtx_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void rtx_drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg);
    
    void OLED_welcome_screen(void);
    void display_battery_level(uint8_t battery_level);

#endif
/* [] END OF FILE */

#include <inttypes.h>

#ifndef LCD_H
#define LCD_H

//! lcd struct
/*!
*/
struct lcd {
    int fd;                 /**< filedescriptor */
    int cols;               /**< lcd column count */
    int rows;               /**< lcd row count */
    int on;                 /**< status of the lcd */
    uint8_t backlight;      /**< status of the lcd backlight */
    uint8_t cursor;         /**< cursor settings */
    uint8_t *buffer;
    int buf_pos;
    int buf_bsize;
    int direct;
};
//! initialize new lcd struct
/*! \param l pointer to the lcd struct
    \param path is path to the i2c device (/dev/i2c-0)
    \param addr is i2c address of the LCD
    \param r number of rows
    \param c number of columns */
int     lcd_setup(struct lcd* l, const char* path, uint8_t addr, int r, int c);
//! move cursor to the specified position
/*! \param r destination row
    \param c destination column
    \param l pointer to the lcd struct */
void    lcd_cursor_pos(struct lcd *l, int r, int c);
//! shift LCD memory one character to the right
/*! \param l pointer to the lcd struct */
void    lcd_shift_right(struct lcd *l);
//! shift LCD memory one character to the left
/*! \param l pointer to the lcd struct */
void    lcd_shift_left(struct lcd *l);
//! turn off the LCD
/*! \param l pointer to the lcd struct */
void    lcd_off(struct lcd *l);
//! turn on the LCD
/*! \param l pointer to the lcd struct */
void    lcd_on(struct lcd *l);
//! do not show cursor
/*! \param l pointer to the lcd struct */
void    lcd_cursor_no(struct lcd *l);
//! show solid cursor
/*! \param l pointer to the lcd struct */
void    lcd_cursor_solid(struct lcd *l);
//! show blinking cursor
/*! \param l pointer to the lcd struct */
void    lcd_cursor_blink(struct lcd *l);
//! turn on the lcd backlight
/*! \param l pointer to the lcd struct */
void    lcd_backlight_on(struct lcd *l);
//! turn off the lcd backlight
/*! \param l pointer to the lcd struct */
void    lcd_backlight_off(struct lcd *l);
//! clear LCD memory (screen)
/*! \param l pointer to the lcd struct */
void    lcd_clear(struct lcd *l);
//! print string at current position
/*! \param str string to display
    \param l pointer to the lcd struct */
void    lcd_print(struct lcd *l, const char *str);
//! define custom character pattern
/*! \param position index in pattern (CGRAM) memory
    \param rows byte array of matrix (each byte represents one row)
    \param l pointer to the lcd struct */
void    lcd_cgram_define(struct lcd *l, int position, uint8_t *rows);
//! define CGRAM characters for horizontal bar
/*! \param l pointer to the lcd struct */
void    lcd_bar_horizontal_cgram(struct lcd *l);
//! display bar at current position
/*! \param width current lenght of the bar (in dots not columns)
    \param width_max max lenght of the bar (in dots not columns)
    \param l pointer to the lcd struct */
void    lcd_bar_display(struct lcd *l, int width, int width_max);

void    lcd_buffer_flush(struct lcd *l);
void    lcd_buffer_on(struct lcd *l);
void    lcd_buffer_off(struct lcd *l);

#endif

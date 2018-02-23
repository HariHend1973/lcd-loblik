#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "lcd.h"

/* I2C expander and LCD wiring (HD44780)
    LCD     I2C expander    meaning
    -------------------------------
    RS      P(0)            command=0, data=1
    RW      P(1)            read=1, write=0 - we don't use this
    EN      P(2)            enable lines, if set to 1 LCD reads input
    BL      P(3)            backlight, on=1, off=1
    D4      P(4)            data, 4-bit interface
    D5      P(5)
    D6      P(6)
    D7      P(7)
*/
#define PCF8574_RS 0b00000001
#define PCF8574_RW 0b00000010
#define PCF8574_EN 0b00000100
#define PCF8574_BL 0b00001000

#define CGRAM_1 0x10
#define CGRAM_2 0x08
#define CGRAM_3 0x04
#define CGRAM_4 0x02
#define CGRAM_5 0x01

#define CMD_CLEAR           0x01
#define CMD_OFF             0x08
#define CMD_ON_NO_CURSOR    0x0C
#define CMD_ON_BLINK_CURSOR 0x0F
#define CMD_ON_SOLID_CURSOR 0x0E

#define CMD_SHIFT_LEFT      0x18
#define CMD_SHIFT_RIGHT     0x1C

#define CMD_FIRST_LINE      0x80
#define CMD_SECOND_LINE     0xC0
#define CMD_THIRD_LINE      0x94
#define CMD_FOURTH_LINE     0xD4

#define CMD_CGRAM_OFFSET    0x40
#define CMD_DDRAM_OFFSET    0x80

enum LCD_Input { DATA, COMMAND };

static uint8_t line_offsets[] = { CMD_FIRST_LINE, CMD_SECOND_LINE,
                                  CMD_THIRD_LINE, CMD_FOURTH_LINE };

static void lcd_init(struct lcd *l);
static void lcd_write(struct lcd *l, uint8_t data, enum LCD_Input type);
static void i2c_write(struct lcd *l, uint8_t byte);

int lcd_setup(struct lcd *l, const char *device, uint8_t i2c_addr, int c, int r) {
    l->rows = r;
    l->cols = c;
    l->buf_bsize = sizeof(uint8_t)*r*c;
    l->buffer = (uint8_t*)malloc(l->buf_bsize);
    memset(l->buffer, ' ', l->buf_bsize);
    l->buf_pos = 0;
    l->direct = 1;
    l->on = 1;
    l->cursor = CMD_ON_NO_CURSOR;
    l->fd = open(device, O_RDWR);
    l->backlight = PCF8574_BL;
    if(l->fd < 0) {
        return -errno;
    }
    if(ioctl(l->fd, I2C_SLAVE, i2c_addr) < 0) {
        return -errno;
    }
    /* test read from expander - just to check
       early if device is accessible */
    uint8_t expander_register;
    if(read(l->fd, &expander_register, 1) < 0) {
        return -errno;
    }
    lcd_init(l);
    return 0;
}

void lcd_cgram_define(struct lcd *l, int position, uint8_t *rows) {
    lcd_write(l, CMD_CGRAM_OFFSET + (position * 8), COMMAND);
    int i;
    for (i = 0; i < 7; i++) {
        lcd_write(l, rows[i], DATA);
    }
    lcd_write(l, 0x00, DATA);
    lcd_write(l, CMD_DDRAM_OFFSET, COMMAND);
}

void lcd_cursor_pos(struct lcd *l, int r, int c) {
    r = r % l->rows;
    c = c % l->cols;

    if (l->direct) {
        lcd_write(l, line_offsets[r] + c, COMMAND);
    } else {
        l->buf_pos = l->cols*r + c;
    }
    return;
}

void lcd_shift_right(struct lcd *l) {
    lcd_write(l, CMD_SHIFT_RIGHT, COMMAND);
}

void lcd_shift_left(struct lcd *l) {
    lcd_write(l, CMD_SHIFT_LEFT, COMMAND);
}

void lcd_off(struct lcd *l) {
    l->on = 0;
    lcd_write(l, CMD_OFF, COMMAND);
}

void lcd_on(struct lcd *l) {
    l->on = 1;
    lcd_write(l, l->cursor, COMMAND);
}

void lcd_cursor_no(struct lcd *l) {
    l->cursor = CMD_ON_NO_CURSOR;
    if (l->on)
        lcd_on(l);
}

void lcd_cursor_solid(struct lcd *l) {
    l->cursor = CMD_ON_SOLID_CURSOR;
    if (l->on)
        lcd_on(l);
}

void lcd_cursor_blink(struct lcd *l) {
    l->cursor = CMD_ON_BLINK_CURSOR;
    if (l->on)
        lcd_on(l);
}


void lcd_backlight_on(struct lcd *l) {
    l->backlight = PCF8574_BL;
    i2c_write(l, l->backlight);
    return;
}

void lcd_backlight_off(struct lcd *l) {
    l->backlight = 0;
    i2c_write(l, l->backlight);
    return;
}

void lcd_clear(struct lcd *l) {
    if (l->direct) {
        lcd_write(l, CMD_CLEAR, COMMAND);
    } else {
        memset(l->buffer, ' ', l->buf_bsize);
    }
}

void lcd_print(struct lcd *l, const char *str) {
    unsigned i;
    for (i = 0; i < strlen(str); i++) {
        lcd_write(l, str[i], DATA);
    }
}

void lcd_buffer_flush(struct lcd *l) {
    printf("flush\n");
    int i,j;
    lcd_cursor_pos(l, 0, 0);
    int row_order[] = {0, 2, 1, 3};
    for (i = 0; i < l->rows; i++) {
        for (j = 0; j < l->cols; j++) {
            l->direct = 1;
            lcd_write(l, l->buffer[row_order[i]*l->cols+j], DATA);
            l->direct = 0;
        }
    }
    printf("flush done\n\n");
}

void lcd_buffer_on(struct lcd *l) {
    l->direct = 0;
}

void lcd_buffer_off(struct lcd *l) {
    l->direct = 1;
}

static void lcd_init(struct lcd *l) {
    /* initialize display to use 4-bit interface */
    i2c_write(l, 0x30);
    i2c_write(l, 0x30 | PCF8574_EN);
    usleep(5000);
    i2c_write(l, 0x30);
    i2c_write(l, 0x30 | PCF8574_EN);
    usleep(200);
    i2c_write(l, 0x30);
    i2c_write(l, 0x30 | PCF8574_EN);
    usleep(200);
    i2c_write(l, 0x20);
    i2c_write(l, 0x20 | PCF8574_EN);
    usleep(5000);

    /* 4-bit 2-line */
    lcd_write(l, 0x28, COMMAND);
    /* clear display and put cursor at home position */
    lcd_write(l, 0x01, COMMAND);
    /* turn on display and hide cursor */
    lcd_write(l, 0x0C, COMMAND);

    return;
}

static void lcd_write(struct lcd *l, uint8_t data, enum LCD_Input type) {

    uint8_t low_bits = (data & 0x0f) << 4;
    uint8_t high_bits = data & 0xf0;

    uint8_t control = l->backlight;
    int delay = 0;
    switch (type) {
        case DATA:
            control = control | PCF8574_RS;
            delay = 50;  /* wait 200us for data writes */
            break;
        case COMMAND:
            delay = 5000; /* wait 5ms for command execution */
            break;
        default:
            fprintf(stderr, "unknown data type");
    }

    if (l->direct) {
    /* write higher part of the byte */
        i2c_write(l, high_bits | control);
        usleep(100);
        i2c_write(l, high_bits | control | PCF8574_EN );
        usleep(100);
        i2c_write(l, high_bits | control);
        usleep(delay);
        /* write lower part of the byte */
        i2c_write(l, low_bits | control);
        usleep(100);
        i2c_write(l, low_bits | control | PCF8574_EN);
        usleep(100);
        i2c_write(l, low_bits | control);
        usleep(delay);
    } else {
        l->buffer[l->buf_pos++] = data;
    }
    return;
}

void lcd_bar_horizontal_cgram(struct lcd *l) {
    uint8_t em[] = {0x15,0x00,0x00,0x00,0x00,0x00,0x15};
    uint8_t c1[] = {0x15,0x10,0x10,0x10,0x10,0x10,0x15};
    uint8_t c2[] = {0x1d,0x18,0x18,0x18,0x18,0x18,0x1d};
    uint8_t c3[] = {0x1d,0x1c,0x1c,0x1c,0x1c,0x1c,0x1d};
    uint8_t c4[] = {0x1b,0x1a,0x1a,0x1a,0x1a,0x1a,0x1b};
    uint8_t c5[] = {0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b};
    uint8_t er[] = {0x15,0x00,0x01,0x00,0x01,0x00,0x15};
    uint8_t el[] = {0x15,0x00,0x10,0x00,0x10,0x00,0x15};

    lcd_cgram_define(l, 0, em);
    lcd_cgram_define(l, 1, c1);
    lcd_cgram_define(l, 2, c2);
    lcd_cgram_define(l, 3, c3);
    lcd_cgram_define(l, 4, c4);
    lcd_cgram_define(l, 5, c5);
    lcd_cgram_define(l, 6, el);
    lcd_cgram_define(l, 7, er);
}

void lcd_bar_display(struct lcd *l, int columns, int cols_max) {
    /* TODO: configurable char width? */
    int full_cols = columns / 5;
    int par_cols = columns % 5;
    int cols = cols_max / 5;
    int i;
    for (i = 0; i < full_cols; i++) {
        lcd_write(l, 5, DATA);
    }
    int cols_empty = cols - full_cols;
    if (par_cols) {
        lcd_write(l, (uint8_t)par_cols, DATA);
        cols_empty--;
    }
    while(cols_empty > 0) {
        if (cols_empty == cols) {
            lcd_write(l, 6, DATA);
        } else if (cols_empty == 1) {
            lcd_write(l, 7, DATA);
        } else {
            lcd_write(l, 0, DATA);
        }
        cols_empty--;
    }
}

static void i2c_write(struct lcd *l, uint8_t byte) {
    int ret = write(l->fd, &byte, 1);
    if (ret < 1) {
        perror("i2c_write");
    }
}

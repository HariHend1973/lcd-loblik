#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "lcd.h"
#include "usage.h"

#define CHAR_WIDTH 5
#define LCD_CHAR_WIDTH 20

int main(void) {

    struct lcd l;
    if (lcd_setup(&l, "/dev/i2c-0", 0x3F, 20, 4) < 0) {  /* initialize the LCD */
        perror("lcd_setup");
        exit(EXIT_FAILURE);
    }

    lcd_bar_horizontal_cgram(&l);              /* send symbols for bar drawing
                                                into CGRAM memory */
    /* enabling off-screen buffer mode
         Symbols are not sent into to the LCD memory inmediatelly, but rather stored
         in library internal buffer. When lcd_buffer_flush() is called, whole
         buffer is writen to the LCD. This can be faster sometimes since it takes
         much longer to execute LCD command (like moving cursor) than just write
         some data. */
    lcd_buffer_on(&l);

    struct timespec next;
    struct cpu_stat cstat_old, cstat_new, cstat_diff;
    struct mem_stat mstat_now;

    /* schedule LCD update for the next wall-time second */
    clock_gettime(CLOCK_REALTIME, &next);
    next.tv_sec += 1;
    next.tv_nsec = 0;

    cpu_stat_read(&cstat_old, PROC_STAT);
    while (1) {
        /* wake up each second tick */
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL);
        next.tv_sec += 1;

        cpu_stat_read(&cstat_new, PROC_STAT);
        cpu_stat_diff(&cstat_diff, &cstat_old, &cstat_new);
        cstat_old = cstat_new;
        mem_stat_read(&mstat_now, PROC_MEMINFO);

        char buffer[LCD_CHAR_WIDTH];
        float perc;
        int max_cols;

        /* print memory usage and draw some bar */
        perc = (float)mem_stat_resident(&mstat_now)/mstat_now.total;
        sprintf(buffer, "Mem:%3.0f%% ", 100 - perc*100);

        max_cols = (LCD_CHAR_WIDTH - strlen(buffer)) * CHAR_WIDTH;
        lcd_cursor_pos(&l, 0, 0);
        lcd_print(&l, buffer);
        lcd_bar_display(&l, max_cols*(1-perc), max_cols);

        /* print cpu usage */
        perc = cpu_stat_usage(&cstat_diff);
        sprintf(buffer, "CPU:%3.0f%% ", perc*100);
        max_cols = (LCD_CHAR_WIDTH - strlen(buffer)) * CHAR_WIDTH;
        lcd_cursor_pos(&l, 1, 0);
        lcd_print(&l, buffer);
        lcd_bar_display(&l, max_cols*perc, max_cols);

        time_t time_epoch;
        time(&time_epoch);
        lcd_cursor_pos(&l, 3, 0);
        strftime(buffer, LCD_CHAR_WIDTH, "Time: %T", localtime(&time_epoch));
        lcd_print(&l, buffer);

        struct statvfs st;
        if (statvfs("/", &st) < 0) { /* get some filesystem stats */
            return 2;
        }
        /* print root filesystem usage and draw percentage bar */
        perc = (double)st.f_bavail/st.f_blocks;
        sprintf(buffer, "Fs/:%lldkB %.0f%% ",
                (st.f_blocks - st.f_bavail)*st.f_frsize >> 10,
                100 - perc*100);

        lcd_cursor_pos(&l, 2, 0);
        lcd_print(&l, buffer);

        max_cols = (LCD_CHAR_WIDTH - strlen(buffer)) * CHAR_WIDTH;
        lcd_bar_display(&l, max_cols*(1-perc), max_cols);

        /* send all to the LCD now */
        lcd_buffer_flush(&l);
    }
    return 0;
};

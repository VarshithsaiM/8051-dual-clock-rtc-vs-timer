/*
 * 8051 Digital Clock (software clock, no RTC needed)
 * Crystal  : 12 MHz
 */

#include <reg51.h>

sbit RS      = P2^0;
sbit EN      = P2^1;
sbit BTN_HR  = P1^0;
sbit BTN_MIN = P1^1;

#define LCD_DATA  P3

/* start time */
volatile unsigned char hours = 12, minutes = 0, seconds = 0;
volatile unsigned char ticks = 0;
volatile unsigned char tick_flag = 1;

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++)
            ;
}

void lcd_cmd(unsigned char cmd)
{
    RS = 0;
    LCD_DATA = cmd;
    EN = 1;
    delay_ms(1);
    EN = 0;
    delay_ms(2);
}

void lcd_data(unsigned char d)
{
    RS = 1;
    LCD_DATA = d;
    EN = 1;
    delay_ms(1);
    EN = 0;
    delay_ms(2);
}

void lcd_init(void)
{
    delay_ms(20);
    lcd_cmd(0x38);   /* 8-bit, 2 lines, 5x7 */
    lcd_cmd(0x0C);   /* display on, cursor off */
    lcd_cmd(0x06);   /* auto-increment cursor */
    lcd_cmd(0x01);   /* clear */
}

void lcd_print(const char *s)
{
    while (*s)
        lcd_data(*s++);
}

void lcd_two_digits(unsigned char n)
{
    lcd_data('0' + n / 10);
    lcd_data('0' + n % 10);
}

void show_time(void)
{
    unsigned char h, m, s;

    EA = 0;                 /* take a consistent snapshot */
    h = hours; m = minutes; s = seconds;
    EA = 1;

    lcd_cmd(0x80);          /* line 1, column 0 */
    lcd_print("Time ");
    lcd_two_digits(h);
    lcd_data(':');
    lcd_two_digits(m);
    lcd_data(':');
    lcd_two_digits(s);
}

/* Timer0 overflows every 50 ms; 20 overflows = 1 second */
void timer0_isr(void) interrupt 1
{
    TH0 = 0x3C;             /* 65536 - 50000 = 0x3CB0 */
    TL0 = 0xB0;

    if (++ticks >= 20) {
        ticks = 0;
        if (++seconds >= 60) {
            seconds = 0;
            if (++minutes >= 60) {
                minutes = 0;
                if (++hours >= 24)
                    hours = 0;
            }
        }
        tick_flag = 1;
    }
}

void main(void)
{
    BTN_HR  = 1;            /* inputs */
    BTN_MIN = 1;

    lcd_init();

    TMOD = 0x01;            /* Timer0, mode 1 (16-bit) */
    TH0  = 0x3C;
    TL0  = 0xB0;
    ET0  = 1;
    EA   = 1;
    TR0  = 1;

    while (1) {
        if (tick_flag) {
            tick_flag = 0;
            show_time();
        }

        if (!BTN_HR) {
            delay_ms(200);
            EA = 0;
            if (++hours >= 24) hours = 0;
            EA = 1;
            tick_flag = 1;
        }

        if (!BTN_MIN) {
            delay_ms(200);
            EA = 0;
            if (++minutes >= 60) minutes = 0;
            seconds = 0;
            EA = 1;
            tick_flag = 1;
        }
    }
}
#include <reg51.h>

#define lcd P3
sbit rs = P2^0;
sbit e  = P2^1;

/* I2C pins - matched to actual schematic wiring:
   P0.1 -> RTC SDA, P0.0 -> RTC SCL */
sbit c = P0^1;   // SCL
sbit d = P0^0;   // SDA

#define RTC_ADDR 0xD0   /* DS1307 7-bit address 0x68, shifted for write */

void delay(int);
void cmd(unsigned char);
void display(unsigned char);
void string(char *);
void init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(unsigned char);
unsigned char i2c_read(void);
void i2c_ack(void);
void i2c_noack(void);
void write(unsigned char, unsigned char, unsigned char);
unsigned char read(unsigned char, unsigned char);
void set_time(unsigned char, unsigned char, unsigned char);

unsigned char code no[10] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57};
unsigned char temp1[3] = {0, 0, 0};

void delay(int d_val) {
    unsigned char i = 0;
    for(; d_val > 0; d_val--) {
        for(i = 250; i > 0; i--);
        for(i = 248; i > 0; i--);
    }
}

void cmd(unsigned char c_val) {
    lcd = c_val;
    rs = 0;
    e = 1;
    delay(5);
    e = 0;
}

void display(unsigned char c_val) {
    lcd = c_val;
    rs = 1;
    e = 1;
    delay(5);
    e = 0;
}

void string(char *p) {
    while(*p) {
        display(*p++);
    }
}

void init(void) {
    cmd(0x38);
    cmd(0x0c);
    cmd(0x01);
    cmd(0x80);
}

#include <intrins.h> // Required for _nop_()

void i2c_delay(void) {
    _nop_();
    _nop_();
}

void i2c_start(void) {
    d = 1;
    c = 1;
    i2c_delay();
    d = 0;
    i2c_delay();
    c = 0;
    i2c_delay();
}

void i2c_stop(void) {
    d = 0;
    c = 0;
    i2c_delay();
    c = 1;
    i2c_delay();
    d = 1;
    i2c_delay();
}

void i2c_write(unsigned char a) {
    unsigned char j;
    for(j = 0; j < 8; j++) {
        c = 0;
        i2c_delay();
        d = (a & 0x80) ? 1 : 0; // Send MSB first
        i2c_delay();
        c = 1;                  // Pulse clock HIGH to latch data
        i2c_delay();
        c = 0;                  // Pull clock back LOW
        a <<= 1;
    }
}

unsigned char i2c_read(void) {
    unsigned char j;
    unsigned char temp = 0;
    d = 1; // Release SDA so slave can control it
    for(j = 0; j < 8; j++) {
        c = 0;
        i2c_delay();
        c = 1; // Pulse clock HIGH to read data
        i2c_delay();
        temp = (temp << 1) | (d ? 1 : 0);
        c = 0;
        i2c_delay();
    }
    return temp;
}

void i2c_ack(void) {
    unsigned int timeout = 0;
    c = 0;
    d = 1; // Release SDA
    i2c_delay();
    c = 1;
    i2c_delay();
    while((d == 1) && (timeout < 500)) {
        timeout++;
    }
    c = 0;
    i2c_delay();
}

void i2c_noack(void) {
    c = 0;
    d = 1; // Master leaves SDA HIGH to signal NACK
    i2c_delay();
    c = 1;
    i2c_delay();
    c = 0;
    i2c_delay();
}

void write(unsigned char sa, unsigned char w_addr, unsigned char dat) {
    i2c_start();
    i2c_write(sa);
    i2c_ack();
    i2c_write(w_addr);
    i2c_ack();
    i2c_write(dat);
    i2c_ack();
    i2c_stop();
    delay(10);
}

unsigned char read(unsigned char sa, unsigned char w_addr) {
    unsigned char buf = 0;
    i2c_start();
    i2c_write(sa);
    i2c_ack();
    i2c_write(w_addr);
    i2c_ack();
    i2c_start();
    i2c_write(sa | 0x01);
    i2c_ack();
    buf = i2c_read();
    i2c_noack();
    i2c_stop();
    return buf;
}

void set_time(unsigned char hour, unsigned char min, unsigned char sec) {
    unsigned int temp[3];
    temp[0] = (((unsigned char)(sec / 10)) << 4) | ((unsigned char)(sec % 10));
    temp[1] = (((unsigned char)(min / 10)) << 4) | ((unsigned char)(min % 10));
    temp[2] = (((unsigned char)(hour / 10)) << 4) | ((unsigned char)(hour % 10));
    write(RTC_ADDR, 0x00, temp[0]);
    write(RTC_ADDR, 0x01, temp[1]);
    write(RTC_ADDR, 0x02, temp[2]);
}

void main() {
    unsigned char temp = 0;
    init();
    string(" Digital Clock ");
    cmd(0xc0);
    string(" using 8051 ");
    delay(3000);
    cmd(0x01);
    set_time(12, 59, 51);
    delay(500);

    while(1) {
        cmd(0x80);
        temp = read(RTC_ADDR, 0x00);
        temp1[0] = ((temp & 0x7F) >> 4) * 10 + (temp & 0x0F);
        temp = read(RTC_ADDR, 0x01);
        temp1[1] = (temp >> 4) * 10 + (temp & 0x0F);
        temp = read(RTC_ADDR, 0x02);
        temp1[2] = (temp >> 4) * 10 + (temp & 0x0F);

        display(no[(temp1[2] / 10) % 10]);
        display(no[temp1[2] % 10]);
        display(':');
        display(no[(temp1[1] / 10) % 10]);
        display(no[temp1[1] % 10]);
        display(':');
        display(no[(temp1[0] / 10) % 10]);
        display(no[temp1[0] % 10]);
    }
}
#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define I2C_ADDR   0x27 // I2C device address


void clear_lcd(void); // clr LCD return home
void lcd_init(void);
void line_position(int line); //move cursor
void write_float(float myFloat);
void write_string(const char *s);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"

#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 1
#define UART_RX_PIN 0

//Color Definitions
#define BLACK 0
#define BLUE 31
#define GREEN 2016
#define GRAY 33840
#define BROWN 48192
#define RED 63488
#define YELLOW 65504
#define WHITE 65535

//XCEN Definitions
#define HORIZONTAL_LEFT 0
#define HORIZONTAL_CENTER 1
#define HORIZONTAL_RIGHT 2

//YCEN Definitions
#define VERTICAL_TOP 0
#define VERTICAL_CENTER 1
#define VERTICAL_BOTTOM 2

//STA DEF
#define STA_CROPIMAGE 0
#define STA_SOLIDCOLOR 1
#define STA_IMAGE 2
#define STA_NONE 3

int nextion_init();
int send_cmd(uint8_t* cmd);

//Functions built off of send_cmd
int clear_screen(int color);
int fill(int x, int y, int w, int h, int color);
int draw_line(int x, int y, int x2, int y2, int color);
int draw_rect(int x, int y, int x2, int y2, int color);
int page(int pagenum);
int set_value(char* object, int value);
int text_box(int x, int y, int w, int h, int font, int pco, int bco, int xcen, int ycen, int sta, char* text); //Fonts should be defined in hud class.
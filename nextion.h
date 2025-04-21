#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 8

#define BLACK 0
#define WHITE 65535

void nextion_init();
int send_cmd(uint8_t* cmd);

//Functions built off of send_cmd
int draw_rect(int x, int y, int x2, int y2, int color);


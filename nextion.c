#include "nextion.h"

void nextion_init(){
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    uart_set_translate_crlf(UART_ID, false);

    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    bi_decl(bi_1pin_with_func(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN)));
    
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));
    bi_decl(bi_1pin_with_func(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN)));

}

void send_cmd(uint8_t cmd){
    const uint8_t term = 0xFF; //Terminating Character

    uart_write_blocking(UART_ID, &cmd, 1);

    int i = 0;
    while(i <= 2){
        uart_write_blocking(UART_ID, &term, 1);
        i++;
    }

}
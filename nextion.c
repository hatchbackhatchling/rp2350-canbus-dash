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

int send_cmd(uint8_t* cmd){
    const uint8_t term = 0xFF; //Terminating Character

    //Send the entire string until the null termninator
    size_t len = 0;
    while(cmd[len] != '\0'){
        uart_write_blocking(UART_ID, &cmd[len], 1);
        len++;
    }

    int i = 0;
    while(i <= 2){
        uart_write_blocking(UART_ID, &term, 1);
        i++;
    }

    return 0;
}

//Functions built off from send_cmd

int clear_screen(int color){
    
    //Creating strings from int to concatenate to command
    char strcolor[5];

    //Converting input parameters to strings
    itoa(color, strcolor, 10);

    //Creating single command to send to HMI.
    char command[50]="cls ";

    strcat(command, strcolor);

    send_cmd(command); //Send finished command to HMI.
}

int draw_rect(int x, int y, int x2, int y2, int color){
    
    //Creating strings from int to concatenate to command
    char strx[3];
    char stry[3];
    char strx2[3];
    char stry2[3];
    char strcolor[5];
    
    //Converting input parameters to strings
    itoa(x, strx, 10);
    itoa(y, stry, 10);
    itoa(x2, strx2, 10);
    itoa(y2, stry2, 10);
    itoa(color, strcolor, 10);

    //Creating single command to send to HMI.
    char command[50]="draw ";
    char comma[1]=",";
    
    //Will figure out a more efficient method later, but for now this is how command is constructed.
    strcat(command, strx);
    strcat(command, comma);
    strcat(command, stry);
    strcat(command, comma);
    strcat(command, strx2);
    strcat(command, comma);
    strcat(command, stry2);
    strcat(command, comma);
    strcat(command, strcolor);

    send_cmd(command); //Send finished command to HMI.
}
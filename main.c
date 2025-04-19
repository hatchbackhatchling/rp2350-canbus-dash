//Importing Dependencies
#include <stdio.h>
#include "pico/stdlib.h"
#include "xl2515.h" //CAN controller libraries

//Nextion Serial interface will be on GP8 TX GP9 RX UART1

int main(){
    stdio_init_all();

    //Starting CAN controller with 1000KBPS BitRate
    xl2515_init(KBPS1000);
    
    //Main loop
    while(true){

    }
    return 0;
}
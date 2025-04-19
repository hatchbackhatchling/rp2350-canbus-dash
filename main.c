//Importing Dependencies
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "nextion.h"
#include "xl2515.h" //CAN controller libraries

//Variables

//0x1000
int rpm;
int map;
signed int baro;
float tps;
float cot;

//0x1001
float egt;
float spd;
float afr1;
float afr2;

//0x1002
int status_flags;
int error_flags;
float pibot;
float sibot;

//0x1003
float iat;
float clt;
float auxt;
int ign_adv;
int inj_dur;
int gear;
int selected_map;
float battery;


//Nextion Serial interface will be on GP8 TX GP9 RX UART1
void core1_entry(){
    while (true){
        printf("Core 1 is running.");
        sleep_ms(1000);
    }
}

int main(){
    stdio_init_all();

    //Starting Nextion Interface on UART1
    nextion_init();   
    multicore_launch_core1(core1_entry);

    //Starting CAN controller with 1000KBPS BitRate
    xl2515_init(KBPS1000);
    
    //Main loop
    while(true){

    }
    return 0;
}
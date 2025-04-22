//Importing Dependencies
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "f1.h"
#include "nextion.h"
#include "xl2515.h" //CAN controller libraries

//Variables
uint32_t msSinceBoot_core0;
uint32_t currentms_core0;

uint32_t msSinceBoot_core1;
uint32_t currentms_core1;

const int updateInterval = 5;

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

uint8_t data_buffer[8];
uint8_t recv_len = 0;

//Nextion Serial interface will be on GP8 TX GP9 RX UART1

//Core 1 will handle nextion refreshing and data output.
void nextion_drawloop(){
    
    //Start off by getting current time
    msSinceBoot_core1 = to_ms_since_boot(get_absolute_time());
    while (true){
        currentms_core1 = to_ms_since_boot(get_absolute_time());
        if((currentms_core1 - msSinceBoot_core1) >= 5){
            //draw background
            drawbg();
        }
    }
}

bool is_id_of_interest(uint32_t id){
    return (id == 0x00001000 ||
            id == 0x00001001 ||
            id == 0x00001002 ||
            id == 0x00001003);
}

int main(){
    stdio_init_all();

    //Starting Nextion Interface on UART1
    nextion_init();   
    multicore_launch_core1(nextion_drawloop);

    //Starting CAN controller with 1000KBPS BitRate
    xl2515_init(KBPS1000);
    
    //Start off by getting current time
    msSinceBoot_core0 = to_ms_since_boot(get_absolute_time());
    while(true){
        currentms_core0 = to_ms_since_boot(get_absolute_time());
        if((currentms_core0 - msSinceBoot_core0) >= 5){
            //Run actual can data extraction method here
            if(xl2515_recv(0x00001000, data_buffer, &recv_len)){
                printf("0x1000");
            }else if(xl2515_recv(0x00001001, data_buffer, &recv_len)){
                printf("0x1001");
            }else if(xl2515_recv(0x00001002, data_buffer, &recv_len)){
                printf("0x1002");
            }else if(xl2515_recv(0x00001003, data_buffer, &recv_len)){
                printf("0x1003");
            }
            //Terminate loop by resetting once executed.
            msSinceBoot_core0 = to_ms_since_boot(get_absolute_time());
        }
    }

    return 0;
}
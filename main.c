//Importing Dependencies
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "f1.h"
#include "nextion.h"
#include "xl2515.h" //CAN controller libraries

//Variables
bool debug = false;
uint32_t msSinceBoot_core0;
uint32_t currentms_core0;

uint32_t msSinceBoot_core1;
uint32_t currentms_core1;

const int updateInterval = 5;

//ECU CANBUS

//0x1000
int rpm;
float map;
float baro;
int tps;
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

//TPMS CANBUS
uint16_t tp[4];

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
            //No need to redraw background now
            updateTacho(rpm); //Update Tachometer
            updateSpeedo(spd); //Update Speedometer
        }
    }
}

//ECU CANBus Data Processing Function

int process_0x00001000(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1000:"); //Print Frame ID
    
    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
    }

    //Extract and convert data according to protocol
    rpm = (data[0] << 8) | data[1];
    map = ((data[2] << 8) | data[3]) / 10.0f;
    baro = ((int16_t)(data[4] << 8) | data[5]) + 1000;
    tps = data[6];
    cot = data[7] * 0.0488f;

    return 0;
}

int process_0x00001001(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1001:"); //Print Frame ID
    
    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
    }

    //Extract and convert data according to protocol
    egt = (data[0] << 8) | data[1];
    spd = ((data[2] << 8) | data[3]) * 0.0141;
    afr1 = ((data[4] << 8) | data[5]) / 10.0f;
    afr2 = ((data[6] << 8) | data[7]) / 10.0f;

    return 0;
}

int process_0x00001002(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1002:"); //Print Frame ID
    
    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
    }

    //Extract and convert data according to protocol
    status_flags = (data[0] << 8) | data[1];
    error_flags = ((data[2] << 8) | data[3]);
    pibot = ((data[4] << 8) | data[5]) * 1.526e-3f;
    sibot = ((data[6] << 8) | data[7]) * 1.526e-3f;
    return 0;
}

int process_0x00001003(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1003:"); //Print Frame ID
    
    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
    }

    //Extract and convert data according to protocol
    iat = data[0] - 40;
    clt = data[1] - 40;
    auxt = data[2] - 40;
    ign_adv = ((int8_t)data[3]) / 2.0f;
    inj_dur = data[4];
    gear = data[5];
    selected_map = data[6];
    battery = data[7] / 11.0f;

    return 0;
}

//TPMS Data Processing Functions
int process_0x18FEF433(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0X18FEF433:"); //Print Frame ID
    
    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
    }
    
    //Extracting the frame index (compound offset) from byte 7
    uint8_t frame_index = data[7] & 0x0F;

    if(frame_index < 4){
        tp[frame_index] = (data[1] << 8) | data[2];
    }

    return 0;
}

int main(){
    stdio_init_all();

    //Starting Nextion Interface on UART1 and draw background.
    nextion_init();   
    drawbg();

    multicore_launch_core1(nextion_drawloop);

    //Starting CAN controller with 1000KBPS BitRate
    xl2515_init(KBPS1000);
    
    //Start off by getting current time
    msSinceBoot_core0 = to_ms_since_boot(get_absolute_time());
    while(true){
        currentms_core0 = to_ms_since_boot(get_absolute_time());
        if((currentms_core0 - msSinceBoot_core0) >= 5){

            //Extract CAN Data from ECU
            if(xl2515_recv(0x00001000, data_buffer, &recv_len)){
                process_0x00001000(data_buffer, recv_len, debug);
            }
            if(xl2515_recv(0x00001001, data_buffer, &recv_len)){
                process_0x00001001(data_buffer, recv_len, debug);
            }
            if(xl2515_recv(0x00001002, data_buffer, &recv_len)){
                process_0x00001002(data_buffer, recv_len, debug);
            }
            if(xl2515_recv(0x00001003, data_buffer, &recv_len)){
                process_0x00001003(data_buffer, recv_len, debug);
            }

            //Extract CAN Data from ECUMaster TPMS
            if(xl2515_recv(0x18FEF433,data_buffer,&recv_len)){
                process_0x18FEF433(data_buffer, recv_len, debug);
            }

            //Terminate loop by resetting once executed.
            msSinceBoot_core0 = to_ms_since_boot(get_absolute_time());
        }
    }

    return 0;
}
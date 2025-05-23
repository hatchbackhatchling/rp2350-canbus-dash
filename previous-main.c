//Importing Dependencies
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "f1.h"
#include "nextion.h"
//#include "xl2515.h" //CAN controller libraries
#include "xl2515_claude.h"

#define LED_PIN   25

//Variables
bool debug = true;
uint32_t msSinceBoot;
uint32_t currentMS;
int counter = 0;

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
uint16_t tp[4] = {0,0,0,0}; //Initialize tire pressure as 0 to avoid null values.

uint8_t data_buffer[8];
uint8_t recv_len = 0;

//ECU CANBus Data Processing Function

int process_0x1000(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1000:"); //Print Frame ID

    //Extract and convert data according to protocol
    rpm = (data[0] << 8) | data[1];
    map = ((data[2] << 8) | data[3]) / 10.0f;
    baro = ((int16_t)(data[4] << 8) | data[5]) + 1000;
    tps = data[6];
    cot = data[7] * 0.0488f;

    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");

        // Print each extracted value
        printf("RPM: %u\r\n", rpm);
        printf("MAP: %.1f kPa\r\n", map);
        printf("BARO: %u mbar\r\n", baro);
        printf("TPS: %u%%\r\n", tps);
        printf("COT: %.2f C\r\n", cot);
    }
    return 0;
}

int update_0x1000(){
    updateTacho(rpm);
    updateTPS(tps);
    return 0;
}

int process_0x1001(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1001:"); //Print Frame ID

    //Extract and convert data according to protocol
    egt = (data[0] << 8) | data[1];
    spd = ((data[2] << 8) | data[3]) * 0.0141;
    afr1 = ((data[4] << 8) | data[5]) / 10.0f;
    afr2 = ((data[6] << 8) | data[7]) / 10.0f;

    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
        printf("EGT: %u C\r\n", egt);
        printf("Speed: %.2f km/h\r\n", spd);
        printf("AFR1: %.1f\r\n", afr1);
        printf("AFR2: %.1f\r\n", afr2);
    }

    return 0;
}

int update_0x1001(){
    updateSpeedo(spd);
    updateAFR(afr2);
    return 0;
}

int process_0x1002(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1002:"); //Print Frame ID
    
    //Extract and convert data according to protocol
    status_flags = (data[0] << 8) | data[1];
    error_flags = ((data[2] << 8) | data[3]);
    pibot = ((data[4] << 8) | data[5]) * 1.526e-3f;
    sibot = ((data[6] << 8) | data[7]) * 1.526e-3f;

    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
        // Print each extracted value
        printf("Status flags: 0x%04x\r\n", status_flags);
        printf("Error flags: 0x%04x\r\n", error_flags);
        printf("PIBOT: %.3f V\r\n", pibot);
        printf("SIBOT: %.3f V\r\n", sibot);
    }

    return 0;
}

int process_0x1003(uint8_t* data, uint8_t len, bool debug){
    printf("Received 0x1003:"); //Print Frame ID
    

    //Extract and convert data according to protocol
    iat = data[0] - 40;
    clt = data[1] - 40;
    auxt = data[2] - 40;
    ign_adv = ((int8_t)data[3]) / 2.0f;
    inj_dur = data[4];
    gear = data[5];
    selected_map = data[6];
    battery = data[7] / 11.0f;
    
    if(debug){
        for(int i=0;i<len; i++){
            printf("%02x ", data[i]);
        }
        printf("\r\n");
        // Print each extracted value
        printf("IAT: %d C\r\n", iat);
        printf("CLT: %d C\r\n", clt);
        printf("AUXT: %d C\r\n", auxt);
        printf("Ignition advance: %.1f deg\r\n", ign_adv);
        printf("Injection duration: %u ms\r\n", inj_dur);
        printf("Gear: %u\r\n", gear);
        printf("Selected map: %u\r\n", selected_map);
        printf("Battery: %.1f V\r\n", battery);
    }

    return 0;
}

int update_0x1003(){
    updateIAT(iat);
    updateCLT(clt);
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
    bool led_state = false;
    stdio_init_all();
    sleep_ms(2000);  // 2 second delay
    printf("\n\n\n");

    printf("SERIAL INIT\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, led_state);

    xl2515_init(KBPS500);
    printf("CANBUS INIT\n");

    int next_status = nextion_init();
    if(next_status == 0){
        printf("NEXTION INIT SUCCESS \n");    
    }else{
        printf("NEXTION INIT FAIL \n");
    }

    uint32_t frame_ids[] = {0x1000, 0x1001, 0x1002, 0x1003};
    const int num_ids = sizeof(frame_ids) / sizeof(frame_ids[0]);
    uint8_t data_buffer[8];
    uint8_t recv_len;
        
    msSinceBoot = to_ms_since_boot(get_absolute_time());
    
    while (true) {
        currentMS = to_ms_since_boot(get_absolute_time());
        bool any_frame_received = false;
        
        // Check for each frame ID
        for (int i = 0; i < num_ids; i++) {
            if (xl2515_recv(frame_ids[i], data_buffer, &recv_len)) {
                any_frame_received = true;
                printf("Received frame ID: 0x%08lX\r\n", frame_ids[i]);
                
                // Process based on the received frame ID
                switch (frame_ids[i]) {
                    case 0x1000:
                        process_0x1000(data_buffer, recv_len, debug);
                        break;
                    case 0x1001:
                        process_0x1001(data_buffer, recv_len, debug);
                        break;
                    case 0x1002:
                        process_0x1002(data_buffer, recv_len, debug);
                        break;
                    case 0x1003:
                        process_0x1003(data_buffer, recv_len, debug);
                        break;
                }
                // Break to restart checking from the first ID
                break;
            }
        }
    
        // Small delay if no frame was received
        if (!any_frame_received) {
            sleep_ms(1);
        }

        if(currentMS - msSinceBoot >= 1000){
            printf("KEEPALIVE\n");
            led_state = !led_state;
            gpio_put(LED_PIN, led_state); 
            msSinceBoot = to_ms_since_boot(get_absolute_time());
        }

    }

}

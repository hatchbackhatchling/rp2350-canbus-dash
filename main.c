#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "xl2515_claude.h"

// Emerald CAN protocol packet IDs
#define EMERALD_PKT_ENGINE_DATA     0x1000
#define EMERALD_PKT_TEMP_SPEED      0x1001
#define EMERALD_PKT_INJECTOR_STATUS 0x1002
#define EMERALD_PKT_SENSORS         0x1003

// Data structures for holding parsed CAN data
typedef struct {
    uint16_t rpm;           // Engine RPM
    float map;              // Manifold Absolute Pressure (kPa)
    float baro;             // Barometric pressure (mBar)
    uint8_t tps;            // Throttle Position (%)
    float coil_on_time;     // Coil on-time (mS)
} emerald_engine_data_t;

typedef struct {
    float egt;              // Exhaust Gas Temperature (°C)
    float road_speed;       // Road speed (mph)
    float afr_lambda_1;     // Air/Fuel Ratio - Lambda 1
    float afr_lambda_2;     // Air/Fuel Ratio - Lambda 2
} emerald_temp_speed_t;

typedef struct {
    uint16_t status_flags;  // Status flags
    uint16_t error_flags;   // Error flags
    float primary_inj_time; // Primary injector bank on-time (mS)
    float secondary_inj_time; // Secondary injector bank on-time (mS)
} emerald_injector_status_t;

typedef struct {
    float air_temp;         // Air temperature (°C)
    float coolant_temp;     // Coolant temperature (°C)
    float aux_temp;         // Auxiliary temperature (°C)
    float ignition_advance; // Ignition advance (°BTDC)
    uint8_t injector_duration; // Injector duration (%)
    uint8_t gear;           // Gear (0=1st, 1=2nd, etc.)
    uint8_t selected_map;   // Selected map (0=map1, 1=map2, 2=map3)
    float battery;          // Battery voltage (Volts)
} emerald_sensors_t;

// Global variables to store the data
emerald_engine_data_t engine_data;
emerald_temp_speed_t temp_speed;
emerald_injector_status_t injector_status;
emerald_sensors_t sensors;

// Flag to track if data has been received for each packet type
bool received_engine_data = false;
bool received_temp_speed = false;
bool received_injector_status = false;
bool received_sensors = false;

// Unknown packets buffer for display
#define MAX_UNKNOWN_PACKETS 10
typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
    bool active;
} unknown_packet_t;

unknown_packet_t unknown_packets[MAX_UNKNOWN_PACKETS];
int unknown_packet_count = 0;

// Function to parse engine data (0x1000)
void parse_engine_data(uint8_t *data, uint8_t len) {
    if (len >= 8) {
        // RPM (bytes 0-1)
        engine_data.rpm = (data[0] | (data[1] << 8));
        
        // MAP (bytes 2-3)
        uint16_t map_raw = (data[2] | (data[3] << 8));
        engine_data.map = map_raw / 10.0f;
        
        // Barometric pressure (bytes 4-5)
        int16_t baro_raw = (data[4] | (data[5] << 8));
        engine_data.baro = baro_raw + 1000.0f;
        
        // Throttle position (byte 6)
        engine_data.tps = data[6];
        
        // Coil on-time (byte 7)
        engine_data.coil_on_time = data[7] * 0.0488f;
        
        received_engine_data = true;
    }
}

// Function to parse temperature and speed data (0x1001)
void parse_temp_speed(uint8_t *data, uint8_t len) {
    if (len >= 8) {
        // EGT (bytes 0-1)
        int16_t egt_raw = (data[0] | (data[1] << 8));
        temp_speed.egt = egt_raw;
        
        // Road speed (bytes 2-3)
        uint16_t speed_raw = (data[2] | (data[3] << 8));
        temp_speed.road_speed = speed_raw * (2.25f/256.0f);
        
        // AFR/Lambda 1 (bytes 4-5)
        uint16_t afr1_raw = (data[4] | (data[5] << 8));
        temp_speed.afr_lambda_1 = afr1_raw / 10.0f;
        
        // AFR/Lambda 2 (bytes 6-7)
        uint16_t afr2_raw = (data[6] | (data[7] << 8));
        temp_speed.afr_lambda_2 = afr2_raw / 10.0f;
        
        received_temp_speed = true;
    }
}

// Function to parse injector status (0x1002)
void parse_injector_status(uint8_t *data, uint8_t len) {
    if (len >= 8) {
        // Status flags (bytes 0-1)
        injector_status.status_flags = (data[0] | (data[1] << 8));
        
        // Error flags (bytes 2-3)
        injector_status.error_flags = (data[2] | (data[3] << 8));
        
        // Primary injector bank on-time (bytes 4-5)
        uint16_t pri_inj_raw = (data[4] | (data[5] << 8));
        injector_status.primary_inj_time = pri_inj_raw * 1.526e-3f;
        
        // Secondary injector bank on-time (bytes 6-7)
        uint16_t sec_inj_raw = (data[6] | (data[7] << 8));
        injector_status.secondary_inj_time = sec_inj_raw * 1.526e-3f;
        
        received_injector_status = true;
    }
}

// Function to parse sensor data (0x1003)
void parse_sensors(uint8_t *data, uint8_t len) {
    if (len >= 8) {
        // Air temperature (byte 0)
        sensors.air_temp = data[0] - 40.0f;
        
        // Coolant temperature (byte 1)
        sensors.coolant_temp = data[1] - 40.0f;
        
        // Auxiliary temperature (byte 2)
        sensors.aux_temp = data[2] - 40.0f;
        
        // Ignition advance (byte 3)
        int8_t advance_raw = (int8_t)data[3];
        sensors.ignition_advance = advance_raw / 2.0f;
        
        // Injector duration (byte 4)
        sensors.injector_duration = data[4];
        
        // Gear (byte 5)
        sensors.gear = data[5];
        
        // Selected map (byte 6)
        sensors.selected_map = data[6];
        
        // Battery voltage (byte 7)
        sensors.battery = data[7] / 11.0f;
        
        received_sensors = true;
    }
}

// Function to add or update unknown packet
void process_unknown_packet(uint32_t can_id, uint8_t *data, uint8_t len) {
    // Check if this ID is already in our list
    for (int i = 0; i < unknown_packet_count; i++) {
        if (unknown_packets[i].id == can_id) {
            // Update existing packet data
            memcpy(unknown_packets[i].data, data, len);
            unknown_packets[i].len = len;
            unknown_packets[i].active = true;
            return;
        }
    }
    
    // If not found and we have space, add it
    if (unknown_packet_count < MAX_UNKNOWN_PACKETS) {
        unknown_packets[unknown_packet_count].id = can_id;
        memcpy(unknown_packets[unknown_packet_count].data, data, len);
        unknown_packets[unknown_packet_count].len = len;
        unknown_packets[unknown_packet_count].active = true;
        unknown_packet_count++;
    }
}

// Function to display all current data
void display_data(void) {
    // Clear terminal
    printf("\033[2J\033[H");  // ANSI escape sequence to clear screen and move cursor to home
    
    printf("=== Emerald CAN Protocol Monitor ===\n\n");
    
    // Display engine data if received
    printf("Packet 0x1000 - Engine Data: %s\n", received_engine_data ? "ACTIVE" : "WAITING");
    if (received_engine_data) {
        printf("  RPM: %d rpm\n", engine_data.rpm);
        printf("  MAP: %.1f kPa\n", engine_data.map);
        printf("  Barometric Pressure: %.1f mBar\n", engine_data.baro);
        printf("  Throttle Position: %d%%\n", engine_data.tps);
        printf("  Coil On-Time: %.3f ms\n", engine_data.coil_on_time);
    }
    printf("\n");
    
    // Display temperature and speed data if received
    printf("Packet 0x1001 - Temp & Speed: %s\n", received_temp_speed ? "ACTIVE" : "WAITING");
    if (received_temp_speed) {
        printf("  EGT: %.1f °C\n", temp_speed.egt);
        printf("  Road Speed: %.1f mph\n", temp_speed.road_speed);
        printf("  AFR/Lambda 1: %.2f\n", temp_speed.afr_lambda_1);
        printf("  AFR/Lambda 2: %.2f\n", temp_speed.afr_lambda_2);
    }
    printf("\n");
    
    // Display injector status if received
    printf("Packet 0x1002 - Injector Status: %s\n", received_injector_status ? "ACTIVE" : "WAITING");
    if (received_injector_status) {
        printf("  Status Flags: 0x%04X\n", injector_status.status_flags);
        printf("  Error Flags: 0x%04X\n", injector_status.error_flags);
        printf("  Primary Injector Bank On-Time: %.3f ms\n", injector_status.primary_inj_time);
        printf("  Secondary Injector Bank On-Time: %.3f ms\n", injector_status.secondary_inj_time);
    }
    printf("\n");
    
    // Display sensor data if received
    printf("Packet 0x1003 - Sensors: %s\n", received_sensors ? "ACTIVE" : "WAITING");
    if (received_sensors) {
        printf("  Air Temp: %.1f °C\n", sensors.air_temp);
        printf("  Coolant Temp: %.1f °C\n", sensors.coolant_temp);
        printf("  Aux Temp: %.1f °C\n", sensors.aux_temp);
        printf("  Ignition Advance: %.1f °BTDC\n", sensors.ignition_advance);
        printf("  Injector Duration: %d%%\n", sensors.injector_duration);
        printf("  Gear: %d\n", sensors.gear + 1);  // Display as 1-based
        printf("  Selected Map: %d\n", sensors.selected_map + 1);  // Display as 1-based
        printf("  Battery: %.1f V\n", sensors.battery);
    }
    printf("\n");
    
    // Display unknown packets
    printf("Unknown CAN Packets: %d\n", unknown_packet_count);
    for (int i = 0; i < unknown_packet_count; i++) {
        if (unknown_packets[i].active) {
            printf("  ID: 0x%04X  Data: ", unknown_packets[i].id);
            for (int j = 0; j < unknown_packets[i].len; j++) {
                printf("%02X ", unknown_packets[i].data[j]);
            }
            printf("\n");
            
            // Reset active flag for next cycle
            unknown_packets[i].active = false;
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Give some time for the USB to connect
    
    printf("Initializing Emerald CAN Monitor...\n");
    printf("Using XL2515 CAN controller\n");
    
    // Initialize the CAN controller at 1Mbps
    xl2515_init(KBPS1000);
    
    printf("CAN initialized at 1Mbps\n");
    printf("Starting CAN monitoring...\n\n");
    
    uint8_t rx_data[8];
    uint8_t rx_len = 0;
    uint32_t current_time, last_display_time = 0;
    
    while (1) {
        // Process known Emerald protocol packets
        if (xl2515_recv(EMERALD_PKT_ENGINE_DATA, rx_data, &rx_len)) {
            parse_engine_data(rx_data, rx_len);
        }
        
        if (xl2515_recv(EMERALD_PKT_TEMP_SPEED, rx_data, &rx_len)) {
            parse_temp_speed(rx_data, rx_len);
        }
        
        if (xl2515_recv(EMERALD_PKT_INJECTOR_STATUS, rx_data, &rx_len)) {
            parse_injector_status(rx_data, rx_len);
        }
        
        if (xl2515_recv(EMERALD_PKT_SENSORS, rx_data, &rx_len)) {
            parse_sensors(rx_data, rx_len);
        }
        
        // Check for other CAN IDs (non-Emerald or unknown)
        // We'll use a loop to check a range of common IDs
        for (uint32_t id = 0; id < 0x9C7; id++) {
            // Skip the known Emerald IDs
            if (id == EMERALD_PKT_ENGINE_DATA || 
                id == EMERALD_PKT_TEMP_SPEED || 
                id == EMERALD_PKT_INJECTOR_STATUS || 
                id == EMERALD_PKT_SENSORS) {
                continue;
            }
            
            if (xl2515_recv(id, rx_data, &rx_len)) {
                process_unknown_packet(id, rx_data, rx_len);
            }
        }
        
        // Update display every 500ms
        current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_display_time >= 500) {
            display_data();
            last_display_time = current_time;
        }
        
        // Brief delay to prevent CPU hogging
        sleep_ms(5);
    }
    
    return 0;
}
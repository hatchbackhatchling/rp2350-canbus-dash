#include "xl2515.h"
#include "hardware/spi.h"
#include <string.h>

#define XL2515_SPI_PORT spi1
#define XL2515_SCLK_PIN 10
#define XL2515_MOSI_PIN 11
#define XL2515_MISO_PIN 12
#define XL2515_CS_PIN 9
#define XL2515_INT_PIN 8


bool g_xl2515_recv_flag = false;
static void xl2515_write_reg(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t buf[len + 2];
    buf[0] = CAN_WRITE;
    buf[1] = reg;
    memcpy(buf + 2, data, len);
    gpio_put(XL2515_CS_PIN, 0);
    spi_write_blocking(XL2515_SPI_PORT, buf, len + 2);
    gpio_put(XL2515_CS_PIN, 1);
}

static void xl2515_read_reg(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t buf[2];
    buf[0] = CAN_READ;
    buf[1] = reg;
    gpio_put(XL2515_CS_PIN, 0);
    spi_write_blocking(XL2515_SPI_PORT, buf, 2);
    spi_read_blocking(XL2515_SPI_PORT, 0, data, len);
    gpio_put(XL2515_CS_PIN, 1);
}

static void xl2515_write_reg_byte(uint8_t reg, uint8_t byte)
{
    uint8_t cmd = CAN_WRITE;
    gpio_put(XL2515_CS_PIN, 0);
    spi_write_blocking(XL2515_SPI_PORT, &cmd, 1);
    spi_write_blocking(XL2515_SPI_PORT, &reg, 1);
    spi_write_blocking(XL2515_SPI_PORT, &byte, 1);
    gpio_put(XL2515_CS_PIN, 1);
}

static uint8_t xl2515_read_reg_byte(uint8_t reg)
{
    uint8_t cmd = CAN_READ;
    uint8_t data = 0;
    gpio_put(XL2515_CS_PIN, 0);
    spi_write_blocking(XL2515_SPI_PORT, &cmd, 1);
    spi_write_blocking(XL2515_SPI_PORT, &reg, 1);
    // spi_write_blocking(XL2515_SPI_PORT, &byte, 1);
    spi_read_blocking(XL2515_SPI_PORT, 0, &data, 1);
    gpio_put(XL2515_CS_PIN, 1);
    return data;
}

void xl2515_reset(void)
{
    uint8_t buf = CAN_RESET;
    gpio_put(XL2515_CS_PIN, 0);
    spi_write_blocking(XL2515_SPI_PORT, &buf, 1);
    gpio_put(XL2515_CS_PIN, 1);
}

void gpio_callback(uint gpio, uint32_t events)
{
    if (events & GPIO_IRQ_EDGE_FALL)
    {
        // printf("xl2515 recv data done!\r\n");
        g_xl2515_recv_flag = true;
    }
}

void xl2515_init(xl2515_rate_kbps_t rate_kbps)
{
    // Set baud rate to 1Mbit/s
    // For 1Mbit/s with 16MHz oscillator:
    uint8_t cnf1 = 0x00;  // BRP=0 (TQ = 2*(0+1)/16M = 0.125us)
    uint8_t cnf2 = 0x90;  // PS1=4, PRSEG=1 (one time quantum)
    uint8_t cnf3 = 0x02;  // PS2=3

    spi_init(XL2515_SPI_PORT, 10 * 1000 * 1000);
    gpio_set_function(XL2515_SCLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(XL2515_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(XL2515_MISO_PIN, GPIO_FUNC_SPI);

    gpio_init(XL2515_CS_PIN);
    gpio_init(XL2515_INT_PIN);

    gpio_set_dir(XL2515_CS_PIN, GPIO_OUT);
    gpio_set_dir(XL2515_INT_PIN, GPIO_IN);
    gpio_pull_up(XL2515_INT_PIN);
    gpio_set_irq_enabled_with_callback(XL2515_INT_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, gpio_callback);

    xl2515_reset();
    sleep_ms(100);
    
    // Use the supplied baud rate if not setting fixed 1Mbit/s
    if (rate_kbps != KBPS1000) {
        uint8_t can_rate_arr[10][3] = {
            {0xA7, 0XBF, 0x07},
            {0x31, 0XA4, 0X04},
            {0x18, 0XA4, 0x04},
            {0x09, 0XA4, 0x04},
            {0x04, 0x9E, 0x03},
            {0x03, 0x9E, 0x03},
            {0x01, 0x1E, 0x03},
            {0x00, 0x9E, 0x03},
            {0x00, 0x92, 0x02},
            {0x00, 0x82, 0x02}};
            
        xl2515_write_reg_byte(CNF1, can_rate_arr[rate_kbps][0]);
        xl2515_write_reg_byte(CNF2, can_rate_arr[rate_kbps][1]);
        xl2515_write_reg_byte(CNF3, can_rate_arr[rate_kbps][2]);
    } else {
        // Use our 1Mbit/s settings
        xl2515_write_reg_byte(CNF1, cnf1);
        xl2515_write_reg_byte(CNF2, cnf2);
        xl2515_write_reg_byte(CNF3, cnf3);
    }

    // Configure for extended frames
    xl2515_write_reg_byte(RXB0CTRL, 0x04);  // Enable extended ID, accept valid messages
    
    // Set up to receive all messages
    xl2515_write_reg_byte(RXF0SIDH, 0x00);
    xl2515_write_reg_byte(RXF0SIDL, 0x00);  // Standard ID part
    xl2515_write_reg_byte(RXF0EID8, 0x00);  // Extended ID part
    xl2515_write_reg_byte(RXF0EID0, 0x00);  // Extended ID part
    
    // Set mask to accept all
    xl2515_write_reg_byte(RXM0SIDH, 0x00); 
    xl2515_write_reg_byte(RXM0SIDL, 0x00);
    xl2515_write_reg_byte(RXM0EID8, 0x00);
    xl2515_write_reg_byte(RXM0EID0, 0x00);

    // CAN interrupt
    xl2515_write_reg_byte(CANINTF, 0x00); // Clean interrupt flag
    xl2515_write_reg_byte(CANINTE, 0x01); // Receive Buffer 0 Full Interrupt Enable Bit

    xl2515_write_reg_byte(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);
    uint8_t dummy = xl2515_read_reg_byte(CANSTAT);
    if ((dummy & 0xe0) != OPMODE_NORMAL)
    {
        printf("OPMODE_NORMAL\r\n");
        xl2515_write_reg_byte(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED); // Set normal mode
    }
}

void xl2515_send(uint32_t can_id, uint8_t *data, uint8_t len)
{
    uint8_t dly = 0;
    while ((xl2515_read_reg_byte(TXB0CTRL) & 0x08) && (dly < 50))
    {
        sleep_ms(1);
        dly++;
    }

    xl2515_write_reg_byte(TXB0SIDH, (can_id >> 3) & 0XFF);
    xl2515_write_reg_byte(TXB0SIDL, (can_id & 0x07) << 5);

    xl2515_write_reg_byte(TXB0EID8, 0);
    xl2515_write_reg_byte(TXB0EID0, 0);
    xl2515_write_reg_byte(TXB0DLC, len);

    xl2515_write_reg(TXB0D0, data, len);
    // for (uint8_t j = 0; j < len; j++) {
    //     xl2515_write_reg_byte(TXB0D0 + j, data[j]);
    // }
    xl2515_write_reg_byte(TXB0CTRL, 0x08);
}

bool xl2515_recv(uint32_t can_id, uint8_t *data, uint8_t *len)
{
    if (g_xl2515_recv_flag == false)
    {
        return false;
    }
    g_xl2515_recv_flag = false;

    // Check if there's a message in buffer 0
    uint8_t canintf = xl2515_read_reg_byte(CANINTF);
    if (!(canintf & 0x01))  // No message in RXB0
    {
        return false;
    }

    // Read the received ID (extended 29-bit ID)
    uint8_t sidh = xl2515_read_reg_byte(RXB0SIDH);
    uint8_t sidl = xl2515_read_reg_byte(RXB0SIDL);
    uint8_t eid8 = xl2515_read_reg_byte(RXB0EID8);
    uint8_t eid0 = xl2515_read_reg_byte(RXB0EID0);
    
    // Check if it's an extended frame
    bool is_extended = (sidl & 0x08) != 0;
    
    uint32_t received_id;
    if (is_extended) {
        // Construct 29-bit extended ID
        received_id = ((uint32_t)sidh << 21) |
                      ((uint32_t)(sidl & 0xE0) << 13) |
                      ((uint32_t)(sidl & 0x03) << 16) |
                      ((uint32_t)eid8 << 8) |
                      (uint32_t)eid0;
    } else {
        // Construct standard 11-bit ID (though we're expecting extended)
        received_id = ((uint32_t)sidh << 3) | ((sidl >> 5) & 0x07);
    }
    
    // Compare with the expected ID
    if (received_id != can_id)
    {
        // Not the ID we're looking for, leave the buffer intact
        g_xl2515_recv_flag = true;  // Re-enable flag for next check
        return false;
    }
    
    // Read the data length
    *len = xl2515_read_reg_byte(RXB0DLC) & 0x0F; // Get only the DLC bits
    if (*len > 8) *len = 8;  // Sanity check
    
    // Read the data
    for (uint8_t i = 0; i < *len; i++)
    {
        data[i] = xl2515_read_reg_byte(RXB0D0 + i);
    }

    // Clear the interrupt flag for this buffer
    xl2515_write_reg_byte(CANINTF, xl2515_read_reg_byte(CANINTF) & ~0x01);
    
    return true;
}

//DO NOT USE SEND FUNCTION THIS IS A TEST LIBRARY

#include <stdio.h>
#include "Resistor.h"

static i2c_master_bus_handle_t bus_handle = NULL;
static i2c_master_dev_handle_t dev_handle = NULL;

void Resistor_Init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = RESIS_SDA,
        .scl_io_num = RESIS_SCL,
        .flags.enable_internal_pullup = false,
        .trans_queue_depth = 8,
    };
    i2c_new_master_bus(&bus_config, &bus_handle);

    i2c_device_config_t dev_config = {
        .dev_addr_length = 7,
        .device_address = 0x2F,
        .flags.disable_ack_check = false,
        .scl_speed_hz = 100 * 1000,
    };
    i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
}

void set_resis_level(uint8_t data)
{
    uint8_t buf[2] = {0x00, data};
    i2c_master_transmit(dev_handle, buf, 2, 10);
}

void set_resis_level_by_percent(uint8_t percent_data)
{
    double percent = percent_data;
    percent /= 128.0;
    set_resis_level((uint8_t)percent);
}
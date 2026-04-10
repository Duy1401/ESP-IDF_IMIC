#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "stdint.h"
#include <string.h>

#define LED_PIN 2
#define PWM_PIN 4

void toggle_mode(uint32_t level){
    gpio_set_level(LED_PIN, level);
}

void pwm_mode(uint32_t pulse){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pulse);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_12_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };ledc_timer_config(&timer_config);
    
    ledc_channel_config_t channel_config = {
        .gpio_num = PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .duty = 0,
        .hpoint = 0,
    };ledc_channel_config(&channel_config);

    const int uart_buffer_size = (1024*2);
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    uint8_t data[128] = {0};
    int mode_count = 0;
    uint32_t mode_1_value = 0;
    uint32_t mode_2_value = 0;
    uint32_t mode_3_value = 0;
    while(1){
        int length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&length));
        if (length > 0){
            memset(data, 0, sizeof(data));
            mode_count = 0;
            int read_length = uart_read_bytes(UART_NUM_0, data, length, 100);
            data[read_length] = '\0';

        }
        if(strstr((char*)data, "1") != 0){
            mode_count++;
            if(mode_count == 10){
                mode_count = 0;
                mode_1_value ^= (1 << 0);
                toggle_mode(mode_1_value);
            }
        }
        else if(strstr((char*)data, "2") != 0){
            mode_count++;
            if(mode_count == 30){
                mode_count = 0;
                mode_2_value ^= (1 << 0);
                toggle_mode(mode_2_value);
            }
        }
        else if(strstr((char*)data, "3") != 0){
            mode_3_value += 100;
            if(mode_3_value > 4095) mode_3_value = 0;
            pwm_mode(mode_3_value);
        }
        else if(strstr((char*)data, "ON") != 0){
            gpio_set_level(LED_PIN, 1);
        }
        else if(strstr((char*)data, "OFF") != 0){
            gpio_set_level(LED_PIN, 0);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include <esp_wifi.h>
#include <driver/gpio.h>
#include <esp_assert.h>

[[maybe_unused]]static const char *TAG = "LED DEMO";

typedef struct {
    gpio_num_t gpio;
    int on_period;
    int off_period;
} LED_DATA;

const static LED_DATA blue_task = {
        .gpio=GPIO_NUM_5,
        .on_period =pdMS_TO_TICKS(400),
        .off_period =pdMS_TO_TICKS(1000)
};
const static LED_DATA green_task = {
        .gpio=GPIO_NUM_6,
        .on_period =pdMS_TO_TICKS(800),
        .off_period =pdMS_TO_TICKS(600)
};

const static LED_DATA red_task = {
        .gpio=GPIO_NUM_7,
        .on_period =pdMS_TO_TICKS(1200),
        .off_period =pdMS_TO_TICKS(200)
};

[[noreturn]] static void led_task(void *arg);


extern "C" void app_main() {
    ESP_UNUSED(app_main);
    gpio_config_t gpio{
            .pin_bit_mask = 1ULL << GPIO_NUM_4,
            .mode = GPIO_MODE_OUTPUT_OD,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type =GPIO_INTR_DISABLE};
    gpio_config(&gpio);
    gpio_set_level(GPIO_NUM_4,0); //LED GND
    xTaskCreate(led_task, "red_task", 12048, (void *) &red_task, 10, nullptr);
    xTaskCreate(led_task, "blue_task", 12048, (void *) &blue_task, 10, nullptr);
    xTaskCreate(led_task, "green_task", 12048, (void *) &green_task, 10, nullptr);
}

[[noreturn]]static void led_task(void *voidArg) {
    const LED_DATA *arg = (LED_DATA *) voidArg;
    gpio_config_t gpio{
            .pin_bit_mask = 1ULL << arg->gpio,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type =GPIO_INTR_DISABLE};
    gpio_config(&gpio);
    while (true) {
        gpio_set_level(arg->gpio, 1);
        vTaskDelay(arg->on_period);
        gpio_set_level(arg->gpio, 0);
        vTaskDelay(arg->off_period);
    }
}



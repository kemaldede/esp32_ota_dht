/* */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "wifi_utils.h"
#include "user_https_ota.h"
#include "dht.h"

#define LED_GPIO 21

#define LED_TAG "LED TAG"
#define APP_TAG "APP TAG"
#define DHT_TAG "DHT_TAG"

static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t dht_gpio = 4;

static float temp = 0.0;
static float hum = 0.0;
static bool dht_err = false;

void led_task(void *pvparameters){
    while (1)
    {   
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        gpio_set_level(LED_GPIO,1);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

static void configure_led(void)
{
    ESP_LOGI(LED_TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(LED_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

void dht_task(void *pvparameters){
    while (1)
    {
        if (dht_read_float_data(sensor_type, dht_gpio, &hum, &temp) == ESP_OK){
            ESP_LOGI(DHT_TAG,"Humidity: %f%% Temp: %fC",hum,temp);
            dht_err = false;
        }    
        else{
            ESP_LOGE(DHT_TAG, "Could not read data from sensor");
            dht_err = true;
        }     
        vTaskDelay(3000/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    wifi_initialise();
    wifi_wait_connected();
    vTaskDelay(1000/portTICK_PERIOD_MS);
    configure_led();

#if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE) && defined(CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK)
    /**
     * We are treating successful WiFi connection as a checkpoint to cancel rollback
     * process and mark newly updated firmware image as active. For production cases,
     * please tune the checkpoint behavior per end application requirement.
     */
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
                ESP_LOGI(APP_TAG, "App is valid, rollback cancelled successfully");
            } else {
                ESP_LOGE(APP_TAG, "Failed to cancel rollback");
            }
        }
    }
#endif

    xTaskCreate(&advanced_ota_example_task, "advanced_ota_example_task", 1024 * 8, NULL, 1, NULL);
    xTaskCreate(&led_task,"LED Blinking Task",1024,NULL,5,NULL);
    xTaskCreate(&dht_task,"DHT Sensor Task",1024 * 2,NULL, 3,NULL);
}

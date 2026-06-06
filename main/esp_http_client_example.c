/* Minimal ESP-IDF HTTP POST example.

   Configure WiFi with:
     idf.py menuconfig
     Example Connection Configuration -> WiFi SSID / WiFi Password
*/

#include <inttypes.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#define SERVER_URL "http://192.168.1.9:3000/emergency"
#define BUTTON_GPIO GPIO_NUM_4
#define BUTTON_PRESSED_LEVEL 0
#define BUTTON_DEBOUNCE_MS 50

static const char *TAG = "emergency_button";

static void configure_button(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = BIT64(BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

static void send_emergency_request(void)
{
    const char *post_data = "{\"event\":\"button_pressed\"}";

    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "POST status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

static void wait_for_button_press_loop(void)
{
    int previous_level = gpio_get_level(BUTTON_GPIO);

    ESP_LOGI(TAG, "Waiting for button press on GPIO%d", BUTTON_GPIO);

    while (true) {
        int current_level = gpio_get_level(BUTTON_GPIO);

        if (previous_level != BUTTON_PRESSED_LEVEL && current_level == BUTTON_PRESSED_LEVEL) {
            vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS));

            if (gpio_get_level(BUTTON_GPIO) == BUTTON_PRESSED_LEVEL) {
                ESP_LOGI(TAG, "Button pressed");
                send_emergency_request();

                while (gpio_get_level(BUTTON_GPIO) == BUTTON_PRESSED_LEVEL) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                ESP_LOGI(TAG, "Button released");
            }
        }

        previous_level = current_level;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    configure_button();

    ESP_LOGI(TAG, "Connected to network");
    wait_for_button_press_loop();
}

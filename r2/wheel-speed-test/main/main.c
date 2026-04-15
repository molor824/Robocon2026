#include <stdio.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"

#define LF_DIR GPIO_NUM_32
#define LF_PWM GPIO_NUM_33
#define LF_PWM_CHANNEL LEDC_CHANNEL_0
#define LF_REVERSE 0

#define RF_DIR GPIO_NUM_25
#define RF_PWM GPIO_NUM_26
#define RF_PWM_CHANNEL LEDC_CHANNEL_1
#define RF_REVERSE 0

#define LB_INA GPIO_NUM_27
#define LB_INB GPIO_NUM_14
#define LB_PWM GPIO_NUM_13
#define LB_PWM_CHANNEL LEDC_CHANNEL_2
#define LB_REVERSE 0

#define RB_INA GPIO_NUM_19
#define RB_INB GPIO_NUM_18
#define RB_PWM GPIO_NUM_5
#define RB_PWM_CHANNEL LEDC_CHANNEL_3
#define RB_REVERSE 0

#define PWM_FREQ 1000

static const char *TAG = "wheel-speed";
static const char *SSID = "hello everynyan how are you?";
static const char *PASSWORD = "fine sankyu";

static EventGroupHandle_t s_wifi_event_group;

// Range: [-1024, 1024]
void set_motor1_speed(gpio_num_t dir_pin, ledc_channel_t pwm_channel, int speed) {
    gpio_set_level(dir_pin, speed > 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel, speed >= 0 ? speed : -speed);
}
void set_motor2_speed(gpio_num_t ina_pin, gpio_num_t inb_pin, ledc_channel_t pwm_channel, int speed) {
    gpio_set_level(ina_pin, speed > 0);
    gpio_set_level(inb_pin, speed < 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel, speed >= 0 ? speed : -speed);
}
void set_lf_speed(int speed) {
    set_motor1_speed(LF_DIR, LF_PWM_CHANNEL, LF_REVERSE ? -speed : speed);
}
void set_rf_speed(int speed) {
    set_motor1_speed(RF_DIR, RF_PWM_CHANNEL, RF_REVERSE ? -speed : speed);
}
void set_lb_speed(int speed) {
    set_motor2_speed(LB_INA, LB_INB, LB_PWM_CHANNEL, LB_REVERSE ? -speed : speed);
}
void set_rb_speed(int speed) {
    set_motor2_speed(LB_INA, LB_INB, LB_PWM_CHANNEL, RB_REVERSE ? -speed : speed);
}

void task_motor_speed(void *args) {

}
void channel_init(ledc_channel_t channel, uint32_t gpio) {
    ledc_channel_config_t config = {
        .channel = channel,
        .gpio_num = gpio,
        .duty = 0,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&config));
}
void timer_init() {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
}
void gpio_init() {
    gpio_config_t io_conf = {
        .pin_bit_mask = BIT(LF_DIR) | BIT(LB_INA) | BIT(LB_INB) | BIT(RF_DIR) | BIT(RB_INA) | BIT(RB_INB),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}
void nvs_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
void event_loop_init() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "Connecting...");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ip = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "got IP: " IPSTR, IP2STR(&ip->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}
void wifi_init() {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(s_wifi_event_group, BIT0, 1, 0, portMAX_DELAY);
}
void app_main(void)
{
    gpio_init();
    timer_init();

    nvs_init();
    event_loop_init();
    wifi_init();

    int pins[] = {LF_PWM, RF_PWM, LB_PWM, RB_PWM};
    int channels[] = {LF_PWM_CHANNEL, RF_PWM_CHANNEL, LB_PWM_CHANNEL, RB_PWM_CHANNEL};
    for (int i = 0; i < sizeof(pins) / sizeof(int); i++) {
        channel_init(channels[i], pins[i]);
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, channels[i], 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, channels[i]));
    }

    xTaskCreate(task_motor_speed, "motor_speed", 1024, NULL, 0, NULL);
}

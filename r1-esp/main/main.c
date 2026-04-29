#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#define TAG "main"
#define LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)

#define SPEED_MODE LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0

const ledc_channel_t motor_a_channels[] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};
const ledc_channel_t motor_b_channels[] = {LEDC_CHANNEL_4, LEDC_CHANNEL_5, LEDC_CHANNEL_6, LEDC_CHANNEL_7};

const gpio_num_t motor_a_gpios[] = {GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_23};
const gpio_num_t motor_b_gpios[] = {GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18};

const gpio_num_t encoder_a_gpios[] = {GPIO_NUM_36, GPIO_NUM_39, GPIO_NUM_34, GPIO_NUM_35};
const gpio_num_t encoder_b_gpios[] = {GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_26};

const int encoder_dirs[] = {1, 1, 1, 1};

const int encoder_args[] = {0, 1, 2, 3};

static QueueHandle_t encoder_queues[4];
static SemaphoreHandle_t encoder_mutexes[4];
static int64_t encoder_counts[4];

static void print_encoders(void *arg) {
    for (;;) {
        int64_t counts[4];
        for (int i = 0; i < 4; i++) {
            xSemaphoreTake(encoder_mutexes[i], portMAX_DELAY);
            counts[i] = encoder_counts[i];
            xSemaphoreGive(encoder_mutexes[i]);
        }
        LOGI("Encoders: %lld, %lld, %lld, %lld\n", counts[0], counts[1], counts[2], counts[3]);

        vTaskDelay(100);
    }
}
static void IRAM_ATTR encoder_isr(void *arg) {
    int index = *(int *)arg;
    uint8_t state = gpio_get_level(encoder_b_gpios[index]);

    BaseType_t higherTaskWoken = pdFALSE;
    xQueueSendFromISR(encoder_queues[index], &state, &higherTaskWoken);
    portYIELD_FROM_ISR(higherTaskWoken);
}
static void encoder_receiver(void *arg) {
    int index = *(int *)arg;
    for (;;) {
        uint8_t state;
        if (xQueueReceive(encoder_queues[index], &state, portMAX_DELAY)) {
            xSemaphoreTake(encoder_mutexes[index], portMAX_DELAY);
            encoder_counts[index] += state ? encoder_dirs[index] : -encoder_dirs[index];
            xSemaphoreGive(encoder_mutexes[index]);
        }
    }
}
static void init_isr() {
    for (int i = 0; i < 4; i++) {
        encoder_queues[i] = xQueueCreate(0x1000, sizeof(uint8_t));
        encoder_mutexes[i] = xSemaphoreCreateMutex();
    }

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    for (int i = 0; i < 4; i++) {
        ESP_ERROR_CHECK(gpio_isr_handler_add(encoder_a_gpios[i], encoder_isr, (void *)&encoder_args[i]));
        xTaskCreate(encoder_receiver, "Encoder Receiver", 0x1000, (void *)&encoder_args[i], 20, NULL);
    }

    LOGI("Initialized ISR.");
}
static void init_gpio() {
    gpio_config_t out_config = {
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config_t in_a_config = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config_t in_b_config = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    for (int i = 0; i < 4; i++) {
        out_config.pin_bit_mask |= (1ULL << motor_a_gpios[i]) | (1ULL << motor_b_gpios[i]);
        in_a_config.pin_bit_mask |= (1ULL << encoder_a_gpios[i]);
        in_b_config.pin_bit_mask |= (1ULL << encoder_b_gpios[i]);
    }
    ESP_ERROR_CHECK(gpio_config(&out_config));
    ESP_ERROR_CHECK(gpio_config(&in_a_config));
    ESP_ERROR_CHECK(gpio_config(&in_b_config));

    LOGI("Initialized GPIO.");
}
static void init_pwm() {
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .speed_mode = SPEED_MODE,
        .timer_num = PWM_TIMER,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    for (int i = 0; i < 4; i++) {
        ledc_channel_config_t a_channel = {
            .channel = motor_a_channels[i],
            .speed_mode = SPEED_MODE,
            .gpio_num = motor_a_gpios[i],
            .timer_sel = PWM_TIMER,
        };
        ledc_channel_config_t b_channel = {
            .channel = motor_b_channels[i],
            .speed_mode = SPEED_MODE,
            .gpio_num = motor_b_gpios[i],
            .timer_sel = PWM_TIMER,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&a_channel));
        ESP_ERROR_CHECK(ledc_channel_config(&b_channel));
    }

    LOGI("Initialized PWM");
}
void app_main(void)
{
    init_pwm();
    init_gpio();
    init_isr();

    xTaskCreate(print_encoders, "Print encoders", 0x1000, NULL, 1, NULL);
}

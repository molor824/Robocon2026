#include <driver/gpio.h>
#include <driver/i2c_slave.h>
#include <esp_event.h>
#include <stdatomic.h>
#include <math.h>

#include "tag.h"
#include "i2c.h"
#include "spi.h"
#include "ledc.h"
#include "wheel.h"
#include "encoder.h"
#include "pid.h"

#define MIN_DELTA_TICK 10

#define SERVO_P 1.0f
#define SERVO_I 0.0f
#define SERVO_D 0.0f

void motor_test(void *arg) {
    // Test 4 motors
    for (;;) {
        for (int i = 0; i < WHEEL_COUNT; i++) {
            printf("motor: %d\n", i);
            // Sweep from 0 to 255
            for (int s = 0; s < 256; s++) {
                wheel_set_motor_speed(i, s);
                wheel_motor_update();
                vTaskDelay(4 / portTICK_PERIOD_MS);
            }
            for (int s = 255; s > -256; s--) {
                wheel_set_motor_speed(i, s);
                wheel_motor_update();
                vTaskDelay(4 / portTICK_PERIOD_MS);
            }
            // Sweep back
            for (int s = -255; s <= 0; s++) {
                wheel_set_motor_speed(i, s);
                wheel_motor_update();
                vTaskDelay(4 / portTICK_PERIOD_MS);
            }
            // Wait 1 second
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
void print_encoders(void *arg) {
    for (;;) {
        int counts[WHEEL_COUNT];
        for (int i = 0; i < WHEEL_COUNT; i++) {
            counts[i] = encoder_get_count(i);
        }
        LOGI("Encoders: %d, %d, %d, %d", counts[0], counts[1], counts[2], counts[3]);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ledc_init();
    spi_init();
    i2c_init();

    wheel_init();
    encoder_init();

    // xTaskCreate(motor_test, "motor_test", 0x1000, NULL, 2, NULL);
    // xTaskCreate(print_encoders, "print_encoders", 0x1000, NULL, 1, NULL);
    // return;

    pid_t pids[WHEEL_COUNT];
    for (int i = 0; i < WHEEL_COUNT; i++) {
        pids[i] = pid_new(SERVO_P, SERVO_I, SERVO_D);
    }
    
    TickType_t lastElapsed = xTaskGetTickCount();
    const float dt = (float)pdTICKS_TO_MS(MIN_DELTA_TICK) * 0.001f;

    for (;;) {
        vTaskDelayUntil(&lastElapsed, MIN_DELTA_TICK);

        int positions[WHEEL_COUNT];
        i2c_read_servo_positions(positions);

        printf("positions: ");

        for (int i = 0; i < WHEEL_COUNT; i++) {
            int error = positions[i] - encoder_get_count(i);
            printf("%d, ", positions[i]);
            float speed = pid_correct(&pids[i], error, dt);
            wheel_set_motor_speed(i, roundf(speed));
        }
        wheel_motor_update();

        printf("\n");
    }
}

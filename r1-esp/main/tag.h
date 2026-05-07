#pragma once

#include <esp_log.h>

#define TAG "main"
#define LOGW(format, ...) ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)

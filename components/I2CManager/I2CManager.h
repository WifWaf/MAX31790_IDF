#ifndef _I2CMANAGER_H
#define _I2CMANAGER_H

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include <freertos/task.h> 
#include <freertos/semphr.h>

extern SemaphoreHandle_t xI2CBinary;

#define I2CMUTEX_TAKE do { if(!xSemaphoreTake(xI2CBinary, pdMS_TO_TICKS(1000))) return ESP_ERR_TIMEOUT; } while(0)
#define I2CMUTEX_GIVE do { xSemaphoreGive(xI2CBinary); } while(0)

esp_err_t I2CMANAGER_initiate();

#endif 
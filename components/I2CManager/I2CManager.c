#include "I2CManager.h"
#include <esp_log.h>
#include <driver/i2c.h>

#define SDA_IO 25         /*!< gpio number for I2C master data  */
#define SCL_IO 26         /*!< gpio number for I2C master clock */

#define FREQ_HZ 100000    /*!< I2C master clock frequency */
#define TX_BUF_DISABLE 0  /*!< I2C master doesn't need buffer */
#define RX_BUF_DISABLE 0  /*!< I2C master doesn't need buffer */

static const char *TAG = "I2C Manager";

SemaphoreHandle_t xI2CBinary;

static esp_err_t I2CMANAGERInstallDrivers();
static esp_err_t I2CMANAGERInitiateSempahores();

esp_err_t I2CMANAGER_initiate()
{
  esp_err_t rslt = 0;   
  esp_log_level_set(TAG, ESP_LOG_DEBUG);
  
  ESP_LOGD(TAG, "Initiating I2C Manager. Core: %d Pri: %d", xPortGetCoreID(), uxTaskPriorityGet(NULL));

  rslt += I2CMANAGERInstallDrivers(); 
  rslt += I2CMANAGERInitiateSempahores();

  ESP_LOGD(TAG, "Initiating I2C Manager Finished");

  return rslt;
}

static esp_err_t I2CMANAGERInstallDrivers()
{
  ESP_LOGD(TAG, "Installing Drivers. Core: %d Pri: %d", xPortGetCoreID(), uxTaskPriorityGet(NULL));

  esp_err_t rslt = 0;                                                                          // Holder for I2C error result

  i2c_config_t i2c_config =                                                                    // Config profile for I2C
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = SDA_IO,
    .scl_io_num = SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_DISABLE,
    .scl_pullup_en = GPIO_PULLUP_DISABLE,
    .master.clk_speed = FREQ_HZ
  };
  
  rslt += i2c_param_config(I2C_NUM_0, &i2c_config);                                             // Push config profile
  rslt += i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, RX_BUF_DISABLE, TX_BUF_DISABLE, 0);    // Install I2C Drivers

  return rslt;     
}

static esp_err_t I2CMANAGERInitiateSempahores()
{
  ESP_LOGD(TAG, "Initiating Semaphores. Core: %d Pri: %d", xPortGetCoreID(), uxTaskPriorityGet(NULL));

  xI2CBinary = xSemaphoreCreateBinary();                                                        // Setup binary semaphore
  xSemaphoreGive(xI2CBinary);                                                                   // Allocate token

  return !xI2CBinary ? ESP_ERR_INVALID_STATE : ESP_OK;
}

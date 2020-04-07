#include "esp_log.h"
#include "MAX31790.h"
#include "I2CManager.h"

static const char *TAG = "MAIN";

void test();
void x_call_fan_con(void *arg);

max31790_master_config_t cfg =
{
   .adr = 0x20,
   .global_cfg = 0x00,
   .fan_failed_seq_start_cfg = 0x45,
   .fan_cfg[0] = (0xFF & (MAX31790_FAN_CFG_SPIN_UP_0_5 |  MAX31790_FAN_CFG_TACH_INPUT)),
   .fan_cfg[1] = (0xFF & (MAX31790_FAN_CFG_SPIN_UP_0_5 |  MAX31790_FAN_CFG_TACH_INPUT)),
   .fan_cfg[2] = (0xFF & (MAX31790_FAN_CFG_SPIN_UP_0_5 |  MAX31790_FAN_CFG_TACH_INPUT)),
   .fan_cfg[3] = (0xFF & (MAX31790_FAN_CFG_CON_MON_MON | MAX31790_FAN_CFG_TACH_INPUT | MAX31790_FAN_CFG_PWM_TACH_TACH)),
   .fan_cfg[4] = (0xFF & (MAX31790_FAN_CFG_CON_MON_MON | MAX31790_FAN_CFG_TACH_INPUT | MAX31790_FAN_CFG_PWM_TACH_TACH)),
   .fan_cfg[5] = (0xFF & (MAX31790_FAN_CFG_CON_MON_MON)),
   .fan_dyn = {0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C},
   .fan_hallcount = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
   .fault_mask_1 = 0x3f,
   .fault_mask_2 = 0x3f
};

void app_main()
{
    I2CMANAGER_initiate();
    MAX31790_initiate(&cfg);    
    
    MAX31790_set_target_dutybits(0, 222);
    MAX31790_set_target_duty(1, 22.2f);
    xTaskCreate(x_call_fan_con, "x_call_fan_con", 2048, NULL, 2, NULL);
}

void x_call_fan_con(void *arg)
{
    /* test variables */
    uint8_t x = 0;
    uint16_t u16b = 0;
    float fb = 0;
    uint32_t u32b = 0;

    for(;;)
    { 
        if(x > 5)
            x = 0;

        MAX31790_get_duty(x, false, &fb);
        MAX31790_get_dutybits(x, false, &u16b);
        MAX31790_get_rpm(x, false, &u32b);

        ESP_LOGD(TAG,"Channel: %d    Duty: %.1f%%    Dutyb: %d    RPM: %d", x, fb, u16b,u32b);

        x++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
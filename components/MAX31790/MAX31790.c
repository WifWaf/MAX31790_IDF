/****************************************************** 
  Description: IDF MAX31790 Fan Controler  
       Author: Jonathan Dempsey JDWifWaf@gmail.com  
      Version: 1.0.0
      License: Apache 2.0
 *******************************************************/

#include "MAX31790.h"
#include "I2CManager.h"

#include <driver/i2c.h>
#include <math.h>
#include <esp_log.h>

static max31790_master_config_t *max31790_config;

static const char *TAG = "MAX31790";
static const uint8_t sr_map[6] = {1, 2, 4, 8, 16, 32};

static esp_err_t MAX31790_write(uint8_t w_adr, uint8_t w_len);
static esp_err_t MAX31790_read(uint8_t r_adr, uint8_t r_len);
static inline esp_err_t MAX31790_read8(uint8_t r_adr, uint8_t *ret_val);

esp_err_t MAX31790_initiate(max31790_master_config_t *cfg)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    ESP_LOGD(TAG, "Initiate");

    return MAX31790_set_master_config(cfg);
}  

/* Set --------------------------------------------------------------------------------------- */
esp_err_t MAX31790_set_master_config(max31790_master_config_t *cfg)
{
    max31790_config = cfg;

    esp_err_t err_ret = ESP_OK;

    err_ret += MAX31790_set_global_config(max31790_config->global_cfg);
    err_ret += MAX31790_set_failed_fan_seq_start(max31790_config->fan_failed_seq_start_cfg);

    for(uint8_t x = 0; x < NUM_CHANNEL; x++)
    {
        err_ret += MAX31790_set_fan_config(max31790_config->fan_cfg[x], x);
        err_ret += MAX31790_set_fan_dynamic(max31790_config->fan_dyn[x], x);
    }

    max31790_config->write_buff[0] = max31790_config->fault_mask_1;
    err_ret += MAX31790_write(MAX31790_REG_FAN_FAULT_MASK_1, 1);
    max31790_config->write_buff[0] = max31790_config->fault_mask_2;
    err_ret += MAX31790_write(MAX31790_REG_FAN_FAULT_MASK_2, 1);
    
    return err_ret;
}

esp_err_t MAX31790_set_target_rpm(uint8_t channel, uint32_t RPM)
{
    uint16_t calc = 0;

    CHCK_CHAN(channel);

    RPM = CONSTRAIN(RPM, RPM_MIN, RPM_MAX);
    calc = CALC_RPM_OR_BIT(RPM, sr_map[(MAX31790_FAN_DYN_SR_MASK & max31790_config->fan_dyn[channel]) >> 5], max31790_config->fan_hallcount[channel]);

    max31790_config->write_buff[0] = LFTJST_TO_MSB(calc, 11);
    max31790_config->write_buff[1] = LFTJST_TO_LSB(calc, 11);

    return MAX31790_write(MAX31790_REG_TARGET_COUNT(channel), 2);
}

esp_err_t MAX31790_set_target_dutybits(uint8_t channel, uint16_t dutybits)
{
    CHCK_CHAN(channel);

    dutybits = CONSTRAIN(dutybits, 0, 511);

    max31790_config->write_buff[0] = LFTJST_TO_MSB(dutybits, 9);
    max31790_config->write_buff[1] = LFTJST_TO_LSB(dutybits, 9);

    return MAX31790_write(MAX31790_REG_TARGET_DUTY(channel), 2); 
}

esp_err_t MAX31790_set_fault_mask(uint8_t fan_number)
{ 
    CHCK_TACH_CHAN(fan_number);

    if(fan_number > 5)
    {
        max31790_config->fault_mask_2 |= (0x01 << (6 - fan_number));
        max31790_config->write_buff[0] = max31790_config->fault_mask_2;
    }
    else
    {
        max31790_config->fault_mask_1 |= (0x01 << (6 - fan_number));
        max31790_config->write_buff[0] = max31790_config->fault_mask_1;
    } 

    return MAX31790_write((fan_number > 5) ? MAX31790_REG_FAN_FAULT_MASK_2 : MAX31790_REG_FAN_FAULT_MASK_1, 1);
}

esp_err_t MAX31790_set_window(uint8_t cfg, uint8_t channel)
{
    CHCK_CHAN(channel);
    max31790_config->write_buff[0] = cfg;
    return MAX31790_write(MAX31790_REG_WINDOW(channel), 1);  
}

esp_err_t MAX31790_set_failed_fan_seq_start(uint8_t ff_ss)
{
    max31790_config->write_buff[0] = ff_ss;
    return MAX31790_write(MAX31790_REG_SEQ_START_CONFIG, 1);
}

esp_err_t MAX31790_set_global_config(uint8_t cfg)
{
    max31790_config->write_buff[0] = cfg;
    return MAX31790_write(MAX31790_REG_GLOBAL_CONFIG, 1);
}

esp_err_t MAX31790_set_pwm_feq(uint8_t bit_freq)
{
    max31790_config->write_buff[0] = bit_freq;
    return MAX31790_write(MAX31790_REG_FREQ_START, 1);
}

esp_err_t MAX31790_set_fan_config(uint8_t fan_cfg, uint8_t channel)
{
    CHCK_CHAN(channel);
    max31790_config->write_buff[0] = fan_cfg;
    return MAX31790_write(MAX31790_REG_FAN_CONFIG(channel), 1);
}

esp_err_t MAX31790_set_fan_dynamic(uint8_t fan_dyn, uint8_t channel)
{
    CHCK_CHAN(channel);
    max31790_config->write_buff[0] = fan_dyn;
    return MAX31790_write(MAX31790_REG_FAN_DYNAMIC(channel), 1);
}

/* Get --------------------------------------------------------------------------------------- */
esp_err_t MAX31790_get_rpm(uint8_t fan_number, bool isTarget, uint32_t *rpm)
{
    esp_err_t err_ret = ESP_OK;

    CHCK_TACH_CHAN(fan_number);

    err_ret = MAX31790_read((isTarget ? MAX31790_REG_TARGET_COUNT(FAN_TO_CHAN(fan_number)) : MAX31790_REG_TACH_COUNT(fan_number)), 2);
    *rpm = REG_TO_LFTJST(11, max31790_config->read_buff[0], max31790_config->read_buff[1]);    
    *rpm = CALC_RPM_OR_BIT(*rpm, sr_map[(MAX31790_FAN_DYN_SR_MASK & max31790_config->fan_dyn[FAN_TO_CHAN(fan_number)]) >> 5], max31790_config->fan_hallcount[fan_number]); 

    return err_ret;
}

esp_err_t MAX31790_get_dutybits(uint8_t channel, bool isTarget, uint16_t *dutybits)
{
    esp_err_t err_ret = ESP_OK;

    CHCK_CHAN(channel);

    MAX31790_read((isTarget ? MAX31790_REG_TARGET_DUTY(channel) : MAX31790_REG_PWM_DUTY(channel)), 2);
    *dutybits = REG_TO_LFTJST(9, max31790_config->read_buff[0], max31790_config->read_buff[1]);

    return err_ret;
}

esp_err_t MAX31790_get_duty(uint8_t channel, bool isTarget, float *fl_duty)
{    
    uint16_t u16buff = 0;

    CHCK_CHAN(channel);
    esp_err_t err_ret = ESP_OK;
    err_ret = MAX31790_get_dutybits(channel, isTarget, &u16buff);
    *fl_duty = MAX31790_bits_to_fduty(u16buff);
    return err_ret;
}

esp_err_t MAX31790_get_target_duty(uint8_t channel, float *tar_duty)
{    
    CHCK_CHAN(channel);
    esp_err_t err_ret = ESP_OK;
    err_ret = MAX31790_get_dutybits(channel, true, (uint16_t*)tar_duty);
    *tar_duty = MAX31790_bits_to_fduty((uint16_t)*tar_duty);
    return err_ret;
}

esp_err_t MAX31790_get_global_config(uint8_t *gl_cfg)
{
    return MAX31790_read8(MAX31790_REG_GLOBAL_CONFIG, gl_cfg);
}

esp_err_t MAX31790_get_pwm_freq(uint8_t *pwm_freq)
{    
    return MAX31790_read8(MAX31790_REG_FREQ_START, pwm_freq);
}

esp_err_t MAX31790_get_failed_fan_seq_opt(uint8_t *ff_seq_opt)
{
    return MAX31790_read8(MAX31790_REG_SEQ_START_CONFIG, ff_seq_opt);
}

esp_err_t MAX31790_get_fan_config(uint8_t channel, uint8_t *fan_cfg)
{
    CHCK_CHAN(channel);
    return MAX31790_read8(MAX31790_REG_FAN_CONFIG(channel), fan_cfg);
}

esp_err_t MAX31790_get_fan_dynamic(uint8_t channel, uint8_t *fan_dyn)
{
    CHCK_CHAN(channel);
    return MAX31790_read8(MAX31790_REG_FAN_DYNAMIC(channel), fan_dyn);
}

esp_err_t MAX31790_get_fault_mask(uint8_t num, uint8_t *f_mask)
{ 
    CHCK_TACH_CHAN(num);
    return ((num < 5) ? MAX31790_read8(MAX31790_REG_FAN_FAULT_MASK_1, f_mask) : MAX31790_read8(MAX31790_REG_FAN_FAULT_MASK_2, f_mask));
}

esp_err_t MAX31790_get_fault_status(uint8_t num, uint8_t *f_status)
{ 
    CHCK_TACH_CHAN(num);
    return ((num < 5) ? MAX31790_read8(MAX31790_REG_FAN_FAULT_STATUS_1, f_status) : MAX31790_read8(MAX31790_REG_FAN_FAULT_STATUS_2, f_status));
}

esp_err_t MAX31790_get_window(uint8_t cfg, uint8_t channel, uint8_t *window)
{
    CHCK_CHAN(channel);
    return MAX31790_read8(MAX31790_REG_WINDOW(channel), window);
}

/* Utility -------------------------------------------------------------------------------------------- */
static esp_err_t MAX31790_write(uint8_t w_adr, uint8_t w_len)
{
    I2CMUTEX_TAKE;

    esp_err_t ret_err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);    

    ret_err += i2c_master_write_byte(cmd, (max31790_config->adr << 1) | I2C_MASTER_WRITE, true);
    ret_err += i2c_master_write_byte(cmd, w_adr, true);

    ret_err += i2c_master_write(cmd, max31790_config->write_buff, w_len, true);

    ret_err += i2c_master_stop(cmd);
    ret_err += i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(500));

    i2c_cmd_link_delete(cmd);
    
    max31790_config->write_buff[0] = 0; max31790_config->write_buff[1] = 0;

    I2CMUTEX_GIVE;

    return ret_err;
}

static esp_err_t MAX31790_read(uint8_t r_adr, uint8_t r_len)
{
    I2CMUTEX_TAKE;

    max31790_config->read_buff[0] = 0; max31790_config->read_buff[1] = 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret_err = ESP_OK;

    ret_err += i2c_master_start(cmd);
    ret_err += i2c_master_write_byte(cmd, (max31790_config->adr << 1) | I2C_MASTER_WRITE, true);
    ret_err += i2c_master_write_byte(cmd, r_adr, true);

    ret_err += i2c_master_start(cmd);
    ret_err += i2c_master_write_byte(cmd, (max31790_config->adr << 1) | I2C_MASTER_READ, true);

    if (r_len > 1)
        ret_err += i2c_master_read(cmd, max31790_config->read_buff, r_len-1, I2C_MASTER_ACK);

    ret_err += i2c_master_read_byte(cmd, max31790_config->read_buff + r_len-1, I2C_MASTER_NACK);
    ret_err += i2c_master_stop(cmd);

    ret_err += i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(500));

    i2c_cmd_link_delete(cmd);
    
    I2CMUTEX_GIVE;

    return ret_err; 
}

static inline esp_err_t MAX31790_read8(uint8_t r_adr, uint8_t *ret_val)
{
    esp_err_t err_ret = ESP_OK;
    err_ret = MAX31790_read(r_adr, 1);
    *ret_val = max31790_config->read_buff[0];
    return err_ret;
}

float MAX31790_bits_to_fduty(uint16_t bits)
{
    float calc = bits;   
    calc = round(MAP((calc*10), 0, 511,0, 100));
    return calc / 10;
}

uint16_t MAX31790_fduty_to_bits(float duty)
{
    float calc = duty;   
    return round(MAP(calc, 0, 100, 0, 511)); 
}

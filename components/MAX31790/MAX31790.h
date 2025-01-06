/****************************************************** 
  Description: IDF MAX31790 Fan Controler  
       Author: Jonathan Dempsey JDWifWaf@gmail.com  
      Version: 1.0.0
      License: Apache 2.0
 *******************************************************/

#ifndef MAX31790_H
#define MAX31790_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

/* MAX31790 Addresses ---------------------- */
#define MAX31790_DEF_ADDR                   0x20     // 0x40 >> 1  ADD0: GND ADD1: GND

/* MAX31790 Options --------------------------------- */ 
// Global Config Bits //
#define MAX31790_GLO_RUN_STANDBY_RUN        0x00
#define MAX31790_GLO_RUN_STANDBY_STANDBY    0x80
#define MAX31790_GLO_RESET_NORMAL           0x00
#define MAX31790_GLO_RESET_RESET            0x40
#define MAX31790_GLO_BUS_TIMEOUT_DIS        0x20
#define MAX31790_GLO_OSC_INT                0x00
#define MAX31790_GLO_OSC_EXT                0x08
#define MAX31790_GLO_I2C_WD                 0x04
#define MAX31790_GLO_I2C_WD_STATUS          0x01

// Fan Config Bits //
#define MAX31790_FAN_CFG_MODE_PWM           0x00
#define MAX31790_FAN_CFG_MODE_RPM           0x80
#define MAX31790_FAN_CFG_SPIN_UP_0_5        0x20
#define MAX31790_FAN_CFG_SPIN_UP_1          0x30
#define MAX31790_FAN_CFG_SPIN_UP_2          0x40
#define MAX31790_FAN_CFG_CON_MON_CON        0x00
#define MAX31790_FAN_CFG_CON_MON_MON        0x10
#define MAX31790_FAN_CFG_TACH_INPUT         0x08
#define MAX31790_FAN_CFG_TACH_LOCK_TACH     0x00
#define MAX31790_FAN_CFG_TACH_LOCK_LOCK     0x04
#define MAX31790_FAN_CFG_PWM_TACH_PWM       0x00
#define MAX31790_FAN_CFG_PWM_TACH_TACH      0x01

// Fan Dynamic Bits //
#define MAX31790_FAN_DYN_SR_1               0x00
#define MAX31790_FAN_DYN_SR_2               0x20
#define MAX31790_FAN_DYN_SR_4               0x40
#define MAX31790_FAN_DYN_SR_8               0xC0
#define MAX31790_FAN_DYN_SR_16              0x80
#define MAX31790_FAN_DYN_SR_32              0xA0
#define MAX31790_FAN_DYN_SR_MASK            0xE0

#define MAX31790_FAN_DYN_PWM_ROC_0          0x00
#define MAX31790_FAN_DYN_PWM_ROC_1_9        0x04
#define MAX31790_FAN_DYN_PWM_ROC_3_9        0x08
#define MAX31790_FAN_DYN_PWM_ROC_7_8        0x0C
#define MAX31790_FAN_DYN_PWM_ROC_15_6       0x10
#define MAX31790_FAN_DYN_PWM_ROC_31_2       0x14
#define MAX31790_FAN_DYN_PWM_ROC_62_5       0x18
#define MAX31790_FAN_DYN_PWM_ROC_125        0x1C
#define MAX31790_FAN_DYN_PWM_ROC_MASK       0x1C

#define MAX31790_FAN_DYN_ASYM_ROC           0x02

// Fan Failed / Sequetional Start //
#define MAX31790_FAN_FAILED_SEQ_SSD_0       0x00
#define MAX31790_FAN_FAILED_SEQ_SSD_250     0x20
#define MAX31790_FAN_FAILED_SEQ_SSD_500     0x40
#define MAX31790_FAN_FAILED_SEQ_SSD_1000    0xC0
#define MAX31790_FAN_FAILED_SEQ_SSD_2000    0x80
#define MAX31790_FAN_FAILED_SEQ_SSD_4000    0xA0
#define MAX31790_FAN_FAILED_SEQ_SSD_MASK    0xE0

#define MAX31790_FAN_FAILED_SEQ_FFO_0       0xE0
#define MAX31790_FAN_FAILED_SEQ_FFO_CON     0x04
#define MAX31790_FAN_FAILED_SEQ_FFO_100     0x08
#define MAX31790_FAN_FAILED_SEQ_FFO_A100    0x0C
#define MAX31790_FAN_FAILED_SEQ_FFO_MASK    0x0C

#define MAX31790_FAN_FAILED_SEQ_FFQ_1       0x00
#define MAX31790_FAN_FAILED_SEQ_FFQ_2       0x01
#define MAX31790_FAN_FAILED_SEQ_FFQ_4       0x02
#define MAX31790_FAN_FAILED_SEQ_FFQ_6       0x04
#define MAX31790_FAN_FAILED_SEQ_FFQ_MASK    0x04

// PWM Freq Bits // 
#define MAX31790_PWM_FREQ_25                0x00
#define MAX31790_PWM_FREQ_30                0x01
#define MAX31790_PWM_FREQ_35                0x02
#define MAX31790_PWM_FREQ_100               0x03
#define MAX31790_PWM_FREQ_125               0x04
#define MAX31790_PWM_FREQ_149_7             0x05
#define MAX31790_PWM_FREQ_1_25K             0x06
#define MAX31790_PWM_FREQ_1_47K             0x07
#define MAX31790_PWM_FREQ_3_57K             0x08
#define MAX31790_PWM_FREQ_5K                0x09
#define MAX31790_PWM_FREQ_12_5K             0x0A
#define MAX31790_PWM_FREQ_25K               0x0B

/* MAX31790 Register -------------------------------- */
// Core Bits //
#define MAX31790_REG_TACH_COUNT(C)         (0x18 + ((C) * 2))
#define MAX31790_REG_PWM_DUTY(C)           (0x30 + ((C) * 2))

// Config Bits //
#define MAX31790_REG_GLOBAL_CONFIG         0x00
#define MAX31790_REG_FAN_CONFIG(C)         (0x02 + (C))
#define MAX31790_REG_FAN_DYNAMIC(C)        (0x08 + (C))
#define MAX31790_REG_SEQ_START_CONFIG      0x14
#define MAX31790_REG_FAILED_FAN_CONFIG     0x14
#define MAX31790_REG_TARGET_DUTY(C)        (0x40 + ((C) * 2))
#define MAX31790_REG_TARGET_COUNT(C)       (0x50 + ((C) * 2))
#define MAX31790_REG_WINDOW(C)             (0x60 + (C))

// Status Bits //
#define MAX31790_REG_FREQ_START            0x01
#define MAX31790_REG_FAN_FAULT_STATUS_2    0x10
#define MAX31790_REG_FAN_FAULT_STATUS_1    0x11
#define MAX31790_REG_FAN_FAULT_MASK_2      0x12
#define MAX31790_REG_FAN_FAULT_MASK_1      0x13

// User Bits //
#define MAX31790_REG_USER_BYTE_0            0xE
#define MAX31790_REG_USER_BYTE_1            0xF
#define MAX31790_REG_USER_BYTE_2            0x15
#define MAX31790_REG_USER_BYTE_3            0x16
#define MAX31790_REG_USER_BYTE_4            0x17
#define MAX31790_REG_USER_BYTE_5            0x4C
#define MAX31790_REG_USER_BYTE_6            0x4D
#define MAX31790_REG_USER_BYTE_7            0x4E
#define MAX31790_REG_USER_BYTE_8            0x4F
#define MAX31790_REG_USER_BYTE_9            0x5C
#define MAX31790_REG_USER_BYTE_10           0x5D
#define MAX31790_REG_USER_BYTE_11           0x5E
#define MAX31790_REG_USER_BYTE_12           0x5F
#define MAX31790_REG_USER_BYTE_13           0x66
#define MAX31790_REG_USER_BYTE_14           0x67

#define NUM_CHANNEL         6
#define NUM_TACH_CHANNEL    12
#define RPM_MIN             120
#define RPM_MAX             7864320

#define CALC_RPM_OR_BIT(X,SR,NP)              ((60 * (SR) * 8192) / (uint64_t)((X) * (NP)))
#define FAN_TO_CHAN(F)                        (((F) > 5) ? (F - 6) : F)
#define CONSTRAIN(X, LOW, HIGH)               (((X) < (LOW)) ? (LOW) : ((X) > (HIGH) ? (HIGH) : (X)))
#define MAP(X, I_MIN, I_MAX, O_MIN, O_MAX)    ((((X) - (I_MIN)) * ((O_MAX) - (O_MIN))) / (((I_MAX) - (I_MIN)) + (O_MIN)))
#define REG_TO_LFTJST(N, MSB, LSB)            (0xFFFF & ((MSB) << ((N) - 8) | (LSB) >> (16 - (N))))
#define LFTJST_TO_MSB(N, LJ)                  (0xFF & ((N) >> ((LJ) - 8)))
#define LFTJST_TO_LSB(N, LJ)                  (0xFF & ((N) << (16 - LJ)))

#define CHCK_TACH_CHAN(C)                     do {if((C) >= NUM_TACH_CHANNEL) return ESP_ERR_INVALID_ARG;} while(0)   
#define CHCK_CHAN(C)                          do {if((C) >= NUM_CHANNEL) return ESP_ERR_INVALID_ARG;} while(0)  

typedef struct
{
   uint8_t adr;                                 // 0x40 >> 1
   uint8_t global_cfg;                          // 0x00;
   uint8_t fan_failed_seq_start_cfg;            // 0x45;
   uint8_t fan_cfg[NUM_CHANNEL];                // {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   uint8_t fan_dyn[NUM_CHANNEL];                // {0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C,};
   uint8_t fan_hallcount[NUM_TACH_CHANNEL];     // {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
   uint8_t fault_mask_1;                        // 0x3f;
   uint8_t fault_mask_2;                        // 0x3f;
   uint8_t write_buff[2];
   uint8_t read_buff[2];
} max31790_master_config_t;

/* Utility -------------------------------------------------------------------------------- */
float MAX31790_bits_to_fduty(uint16_t bits);
uint16_t MAX31790_fduty_to_bits(float duty);

/* Setup ---------------------------------------------------------------------------------- */
esp_err_t MAX31790_initiate(max31790_master_config_t *cfg);

/* Set ------------------------------------------------------------------------------------ */
esp_err_t MAX31790_set_master_config(max31790_master_config_t *cfg);

esp_err_t MAX31790_set_target_dutybits(uint8_t channel, uint16_t dutybits);

esp_err_t MAX31790_set_target_rpm(uint8_t channel, uint32_t RPM);               // When changing from PWM mode to RPM mode, best results are obtained by loading this register with the current TACH count before changing to RPM mode.

esp_err_t MAX31790_set_fault_mask(uint8_t fan_number);

esp_err_t inline MAX31790_set_target_duty(uint8_t channel, float fduty) { return MAX31790_set_target_dutybits(channel, MAX31790_fduty_to_bits(fduty)); };

esp_err_t MAX31790_set_window(uint8_t cfg, uint8_t channel);

esp_err_t MAX31790_set_failed_fan_seq_start(uint8_t ff_ss);

esp_err_t MAX31790_set_global_config(uint8_t cfg);

esp_err_t MAX31790_set_pwm_feq(uint8_t bit_freq);

esp_err_t MAX31790_set_fan_config(uint8_t fan_cfg, uint8_t channel);

esp_err_t MAX31790_set_fan_dynamic(uint8_t fan_dyn, uint8_t channel);

/* Get ------------------------------------------------------------------------------------- */
esp_err_t MAX31790_get_dutybits(uint8_t channel, bool isTarget, uint16_t *dutybits);

esp_err_t MAX31790_get_rpm(uint8_t fan_number, bool isTarget, uint32_t *rpm);

esp_err_t MAX31790_get_global_config(uint8_t *gl_cfg); 

esp_err_t inline MAX31790_get_target_dutybits(uint8_t channel, uint16_t *tar_dutybits){ return MAX31790_get_dutybits(channel, true, tar_dutybits); };

esp_err_t MAX31790_get_duty(uint8_t channel, bool isTarget, float *fl_duty);

esp_err_t MAX31790_get_target_duty(uint8_t channel, float *tar_duty);

esp_err_t inline MAX31790_get_target_tach(uint8_t fan_number, uint32_t *rpm){ return MAX31790_get_rpm(fan_number, true, rpm); };

esp_err_t MAX31790_get_pwm_freq(uint8_t *pwm_freq);

esp_err_t MAX31790_get_failed_fan_seq_opt(uint8_t *ff_seq_opt);

esp_err_t MAX31790_get_fan_config(uint8_t channel, uint8_t *fan_cfg);

esp_err_t MAX31790_get_fan_dynamic(uint8_t channel, uint8_t *fan_dyn);

esp_err_t MAX31790_get_fault_mask(uint8_t num, uint8_t *f_mask);

esp_err_t MAX31790_get_fault_status(uint8_t num, uint8_t *f_status);

esp_err_t MAX31790_get_window(uint8_t cfg, uint8_t channel, uint8_t *window);

#endif

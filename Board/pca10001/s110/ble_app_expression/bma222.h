 /* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef BMA222_H
#define BMA222_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/** @file
* @brief BMA222 digital temperature sensor driver.
*
*
* @defgroup nrf_drivers_bma222 BMA222 digital accelerometer 
*   		sensor driver @{
* @ingroup nrf_drivers
* @brief BMA222 digital accelerometer sensor driver.
*/

/**
 * Initializes BMA222 accelerometer sensor.
 *
 * @note Before calling this function, you must initialize twi_master first.
 *
 * @param device_address Bits [2:0] for the device address. All other bits must be zero.
 * @return
 * @retval true If communication succeeded with the device.
 * @retval false If communication failed with the device.
 */

static bool bma222_write(uint8_t myRegister, uint8_t myData);

bool bma222_init(uint8_t device_address);
bool bma222_write(uint8_t register_address, uint8_t register_data);
bool bma222_reset(void);
bool bma222_acc_read(void);
int8_t bma222_getX(void);
int8_t bma222_getY(void);
int8_t bma222_getZ(void);
bool bma222_read_reg(uint8_t reg_address, uint8_t *dataout);

void bma222_setDeviceAddress(uint8_t device_address);
/**
 * Reads temperature from the sensor.
 *
 * @param acceleration_high Memory location to store upper byte 
 *  						acceleration.
 * @param acceleration_low Memory location to store lower byte 
 *  					   acceleration.
 * @return
 * @retval true Acceleration was successfully read
 * @retval false Acceleration reading failed or conversion was 
 *  	   not yet complete
 */

//bool bma222_init(uint8_t device_address);

bool bma_accel_read(int8_t *acceleration_high, int8_t *acceleration_low);
bool bma222_read_all_reg(void);

static uint8_t bma222_range_read(void);

// BMA222 uses 0x08 or 0x09 as Base Address //!< BMA222 Only
#define BMA222_BASE_ADDRESS_DEV0  (0x08) // << 1) //!<BMA222 base address
#define BMA222E_BASE_ADDRESS_DEV0 (0x18) // << 1) //!<BMA222E base address
#define BMA222E_BASE_ADDRESS_DEV1 (0x19) // << 1) //!<BMA222E base address

#define BMA222_RANGE_REG_2G_MODE 0x03
#define BMA222_RANGE_REG_4G_MODE 0x05
#define BMA222_RANGE_REG_8G_MODE 0x08
#define BMA222_RANGE_REG_16G_MODE 0x0C

#define BMA222_INT_ELEC_INT2_OD   (1 << 3)
#define BMA222_INT_ELEC_INT2_LVL  (1 << 2)
#define BMA222_INT_ELEC_INT1_OD   (1 << 1)
#define BMA222_INT_ELEC_INT1_LVL  (1 << 0)

// Bandwidth Register 0x10
#define BMA222_7_81HZ  0x08
#define BMA222_15_63HZ 0x09
#define BMA222_31_25HZ 0x0A

// Power Modes 0x11
#define BMA222_SUSPEND_SLEEP_500US  0  << 1
#define BMA222_SUSPEND_SLEEP_1MS    6  << 1
#define BMA222_SUSPEND_SLEEP_2MS    7  << 1
#define BMA222_SUSPEND_SLEEP_4MS    8  << 1
#define BMA222_SUSPEND_SLEEP_6MS    9  << 1
#define BMA222_SUSPEND_SLEEP_10MS  10  << 1
#define BMA222_SUSPEND_SLEEP_25MS  11  << 1
#define BMA222_SUSPEND_SLEEP_50MS  12  << 1
#define BMA222_SUSPEND_SLEEP_100MS 13  << 1
#define BMA222_SUSPEND_SLEEP_500MS 14  << 1
#define BMA222_SUSPEND_SLEEP_1S    15  << 1
   
#define BMA222_SUSPEND_SUSPEND_ON  1   << 7
#define BMA222_SUSPEND_SUSPEND_OFF 0   << 7     

#define BMA222_SUSPEND_LOWPOWER_ON  1   << 6
#define BMA222_SUSPEND_LOWPOWER_OFF 0   << 6


// Interupt Settings (0x16) Bit defines 
#define BMA222_INT_SETTING1_FLAT_EN    (1 << 7)
#define BMA222_INT_SETTING1_ORIENT_EN  (1 << 6)
#define BMA222_INT_SETTING1_S_TAP_EN   (1 << 5)
#define BMA222_INT_SETTING1_D_TAP_EN   (1 << 4)
#define BMA222_INT_SETTING1_SLOPE_Z_EN (1 << 2)
#define BMA222_INT_SETTING1_SLOPE_Y_EN (1 << 1)
#define BMA222_INT_SETTING1_SLOPE_X_EN (1 << 0)

// Interrupt Settings 2 (0x17) Bit defines
#define BMA222_INT_SETTING2_DATA_EN    (1 << 4)
#define BMA222_INT_SETTING2_LOW_EN     (1 << 3)
#define BMA222_INT_SETTING2_HIGHG_Z_EN (1 << 2)
#define BMA222_INT_SETTING2_HIGHG_Y_EN (1 << 1)
#define BMA222_INT_SETTING2_HIGHG_X_EN (1 << 0)

// Interrupt Mapping 1 & 3 (0x19 & 0x1B) Bit defines
#define BMA222_INT_MAPPING_INT_FLAT    (1 << 7)
#define BMA222_INT_MAPPING_INT_ORIENT  (1 << 6)
#define BMA222_INT_MAPPING_INT_S_TAP   (1 << 5)
#define BMA222_INT_MAPPING_INT_D_TAP   (1 << 4)
#define BMA222_INT_MAPPING_INT_SLOPE   (1 << 2)
#define BMA222_INT_MAPPING_INT_HIGH    (1 << 1)
#define BMA222_INT_MAPPING_INT_LOW     (1 << 0)

// Interrupt Mapping 2 (0x1A) Bit defines
#define BMA222_INT_MAPPING2_INT2_DATA   (1 << 7)
#define BMA222_INT_MAPPING2_INT1_DATA   (1 << 0)

// Interrupt reset mode selection
#define BMA222_INT_RESET_NO_LATCH       (0x00)
#define BMA222_INT_RESET_250MS          (0x01)
#define BMA222_INT_RESET_500MS          (0x02)
#define BMA222_INT_RESET_1S             (0x03)
#define BMA222_INT_RESET_2S             (0x04)
#define BMA222_INT_RESET_4S             (0x05)
#define BMA222_INT_RESET_8S             (0x06)
#define BMA222_INT_RESET_500US          (0x09)
#define BMA222_INT_RESET_1MS            (0x0B)
#define BMA222_INT_RESET_12_5MS         (0x0C)
#define BMA222_INT_RESET_25MS           (0x0D)
#define BMA222_INT_RESET_50MS           (0x0E)
#define BMA222_INT_RESET_LATCHED        (0x0F)



/**
 *@}
 **/

/*lint --flb "Leave library region" */ 
#endif

/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_srv_lbs LEDButton Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief LEDButton Service module.
 *
 * @details This module implements the LEDButton Service with the Battery Level characteristic.
 *          During initialization it adds the LEDButton Service and Battery Level characteristic
 *          to the BLE stack datalbse. Optionally it can also add a Report Reference descriptor
 *          to the Battery Level characteristic (used when including the LEDButton Service in
 *          the HID service).
 *
 *          If specified, the module will support notification of the Battery Level characteristic
 *          through the ble_lbs_battery_level_update() function.
 *          If an event handler is supplied by the application, the LEDButton Service will
 *          generate LEDButton Service events to the application.
 *
 * @note The application must propagate BLE stack events to the LEDButton Service module by calling
 *       ble_lbs_on_ble_evt() from the from the @ref ble_stack_handler callback.
 */

#ifndef BLE_ACS_H__
#define BLE_ACS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#define LBS_UUID_BASE {0x8C, 0x75, 0xD3, 0x5A, 0x36, 0x21, 0x86, 0xFE, 0xD5, 0xBF, 0x68, 0x5D, 0x00, 0x00, 0x00, 0x00}
#define ACS_UUID_SERVICE 0x1523
#define LBS_UUID_LED_CHAR 0x1525
#define LBS_UUID_BUTTON_CHAR 0x1524


/**
 *  @brief HID Service
 */
#define BT_UUID_ACS \
	BT_UUID_DECLARE_16(ACS_UUID_SERVICE)

// Forward declaration of the ble_lbs_t type. 
typedef struct ble_acs_s ble_acs_t;

/**@brief LEDButton Service event handler type. */
typedef void (*ble_acs_led_write_handler_t) (ble_acs_t * p_lbs, uint8_t new_state);

/**@brief LEDButton Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_acs_led_write_handler_t   led_write_handler;                   
} ble_acs_init_t;


/**@brief LEDButton Service structure. This contains various status information for the service. */
typedef struct ble_acs_s
{         
    uint16_t                     service_handle;                 
    ble_gatts_char_handles_t     led_char_handles;          
    ble_gatts_char_handles_t     button_char_handles;          
    uint8_t                      uuid_type;
    uint8_t                      current_led_state;
	  uint8_t                      x_axis;
	  uint8_t                      y_axis;
	  uint8_t                      z_axis;             
    uint16_t                     conn_handle;  
    bool                         is_notifying;
    ble_acs_led_write_handler_t  led_write_handler;
} ble_acs_t;


/**@brief Initialize the LEDButton Service.
 *
 * @param[out]  p_lbs       
 * @param[in]   p_lbs_init  
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_acs_init(ble_acs_t * p_lbs, const ble_acs_init_t * p_acs_init);

/**@brief LEDButton Service BLE stack event handler.
 *
 * @details Handles all events from the BLE stack of interest to the LEDButton Service.
 *
 * @param[in]   p_lbs      LEDButton Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_acs_on_ble_evt(ble_acs_t * p_lbs, ble_evt_t * p_ble_evt);

/**@brief Handler to notify the LEDButton service on button presses.
 *
 * @param[in]   p_lbs      LEDButton Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
uint32_t ble_acs_on_button_change(ble_acs_t * p_lbs, uint8_t button_state, uint8_t axis_x, uint8_t axis_y, uint8_t axis_z);

#endif // BLE_LBS_H__

/** @} */

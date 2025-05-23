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
 * @defgroup ble_sdk_app_hrs_eval_main main.c
 * @{
 * @ingroup ble_sdk_app_hrs_eval
 * @brief Main file for Heart Rate Service Sample Application for nRF51822 evaluation board
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services) for the nRF51822 evaluation board (PCA10001).
 * This application uses the @ref ble_sdk_lib_conn_params module.
 */
// Un-comment this define to use a hard coded BTLE UUID instead of content from the UICR
// #define USE_FIXED_CONFIG 1
//#if defined(BOARD_NRF6310)
  
//#elif defined(BOARD_PCA10000)
  
//#elif defined(BOARD_PCA10001)
  
//#elif defined(BOARD_PCA10003)
  
//#else
    //#define REAL_ACCEL
//#endif



//  Memory contents will consists of a known header, a BTLE address and a checksum.  
// The checksum will be a 8 bit sum of the header and BTLE address ( a total of 10 bytes)
#define RAM_HEADER  { 0x55, 0xAA, 0x33, 0xCC }
#define BTLE_SN     { 0x57, 0xF9, 0x99, 0x49, 0x3D, 0xFF }


#define _GPIOTE_CONFIG_BUTTON(PIN_NO)             \
        (                                         \
          (GPIOTE_CONFIG_POLARITY_HiToLo << 16) | \
          ((PIN_NO) << 8) |                       \
          GPIOTE_CONFIG_MODE_Event                \
        )

#define GPIOTE_CONFIG_BUTTON(PIN_NO) _GPIOTE_CONFIG_BUTTON(PIN_NO)
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_bas.h"
#include "ble_hrs.h"
#include "ble_acs.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "led.h"
#include "battery.h"
#include "device_manager.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "ble_debug_assert_handler.h"
#include "pstorage.h"
#include "app_trace.h"
#include "boards.h"


#include "bma222.h"
#include "twi_master.h"
#include "uicr.h"
#include "app_util_platform.h"


#define IS_SRVC_CHANGED_CHARACT_PRESENT      0                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/
                                                                             
#define HR_INC_BUTTON_PIN_NO                 BUTTON_0                                   /**< Button used to increment heart rate. */
#define HR_DEC_BUTTON_PIN_NO                 BUTTON_1                                   /**< Button used to decrement heart rate. */
#define BOND_DELETE_ALL_BUTTON_ID            HR_DEC_BUTTON_PIN_NO                       /**< Button used for deleting all bonded centrals during startup. */
#define UICR_ADDR                        0x10001080

#define DEVICE_NAME                          "IR Expression"                            /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "Idea Rockets LLC."                        /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                     40                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS           180                                        /**< The advertising timeout in units of seconds. */

//#define DICE_LED1_PIN_NO                 17                                           /*!< Pin for LED1 on Dice 600 board*/
#define DICE_LED1_PIN_NO                 23                                             /*!< Pin for LED1. original prototype */
#define DICE_LED2_PIN_NO                 29                                             /*!< Pin for LED1. original prototype */
#define ACCEL_INT1_PIN_NO                1                                              /*!< Pin for INT1. */
#define ACCEL_INT2_PIN_NO                2                                              /*!< Pin for INT2. */

#define APP_TIMER_PRESCALER                  0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 5                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              5                                          /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER) /**< Battery level measurement interval (ticks). */
#define BMA222_MEAS_INTERVAL                 APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)   /**< Acceloromter level measurement interval (ticks). */

#define TX_TIMEOUT_DURATION                  APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)  /*!< The time to stay a awake after waking interrupts stop being asserted. */
#define TX_TIMEOUT_SCOUNT                    (TX_TIMEOUT_DURATION / BMA222_MEAS_INTERVAL)  /*!< Calculated number of samples to meet the approximate terminal time*/

#define HEART_RATE_MEAS_INTERVAL             APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) /**< Heart rate measurement interval (ticks). */
#define MIN_HEART_RATE                       60                                         /**< Minimum heart rate as returned by the simulated measurement function. */
#define MAX_HEART_RATE                       300                                        /**< Maximum heart rate as returned by the simulated measurement function. */
#define HEART_RATE_CHANGE                    2                                          /**< Value by which the heart rate is incremented/decremented during button press. */

#define APP_GPIOTE_MAX_USERS                 1                                          /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define MIN_CONN_INTERVAL                    MSEC_TO_UNITS(500, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL                    MSEC_TO_UNITS(1000, UNIT_1_25_MS)          /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                        0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT         30                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_TIMEOUT                    30                                         /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                       1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                       0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES            BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                        0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE               7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE               16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                            0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */



static ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
// BMA222 configuration
#define BMA222_DEVICE_ID                 0x00
#define BMA222_SDA_PIN_NO                TWI_MASTER_CONFIG_DATA_PIN_NUMBER              // BMA serial pin interfaces
#define BMA222_SCL_PIN_NO                TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER             // BMA serial pin interfaces 

// LED FLASH configuration
#define LED_TOGGLE_INTERVAL              APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)
#define LED_TOGGLE_DURATION              APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)    /*!< The time to stay a awake after waking interrupts stop being asserted. */
#define LED_TOGGLE_SCOUNT                (LED_TOGGLE_DURATION / LED_TOGGLE_INTERVAL)   /*!< Calculated number of samples to meet the approximate terminal time*/
ble_bas_t                                    bas;                                       /**< Structure used to identify the battery service. */
static ble_hrs_t                             m_hrs;                                     /**< Structure used to identify the heart rate service. */
static ble_acs_t                             m_lbs;
static volatile uint16_t                     m_cur_heart_rate;                          /**< Current heart rate value. */
                                                                                       
static app_timer_id_t                        m_battery_timer_id;                        /**< Battery timer. */
static app_timer_id_t                        m_accel_timer_id;                          /**< Accelerometer timer. */

static bool                                  m_memory_access_in_progress = false;       /**< Flag to keep track of ongoing operations on persistent memory. */
static dm_application_instance_t             m_app_handle;                              /**< Application identifier allocated by device manager */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt);

static void sys_evt_dispatch(uint32_t sys_evt);


/*****************************************************************************
* Error Handling Functions
*****************************************************************************/


static bool                              s_useLEDs = false;         /*!< Enable LED control for use by application*/
static uint8_t                           s_LED_state = 0;           /*!< Keep track of LED's states to control turn on and off of LED's*/

void dly5ms(void);				  // 5 miliseconds delay
void dly50ms(void);               // 50 milisecond delay
void dly100ms(void);              // 100 milisecond delay
void dly250ms(void);			  // 250 miliseconds delay 
static void doubleFlashAlternating(void);
static void ledParser(uint8_t report_val);
uint8_t my_address[6] =  /*!< Assign a default address. */
{
    0x01, /* 0x01  */
    0x02, /* 0x02  */
    0x03, /* 0x03  */
    0x04, /* 0x04  */
    0x05, /* 0x05  */
    0x06  /* 0x06  */
};

uint16_t tx_timeout = TX_TIMEOUT_SCOUNT;
uint16_t flash_timeout = 0;
/**@brief Function for error handling, which is called when an error has occurred. 
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    // This call can be used for debug purposes during application development.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Un-comment the line below to use.
    // ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover with a reset.
    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


static void led_write_handler(ble_acs_t * p_lbs, uint8_t led_state)
{
	//ledParser(led_state);
}


/*****************************************************************************
* Static Timeout Handling Functions
*****************************************************************************/

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *          This function will start the ADC.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_start();
}




/**@brief Function for handling the Heart rate measurement timer timeout.
 *
 * @details This function will be called each time the heart rate measurement timer expires.
 *          It will exclude RR Interval data from every third measurement.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void heart_rate_meas_timeout_handler(void * p_context)
{
    uint32_t err_code;

    UNUSED_PARAMETER(p_context);

    err_code = ble_hrs_heart_rate_measurement_send(&m_hrs, m_cur_heart_rate);

    if (
        (err_code != NRF_SUCCESS)
        &&
        (err_code != NRF_ERROR_INVALID_STATE)
        &&
        (err_code != BLE_ERROR_NO_TX_BUFFERS)
        &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    )
    {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
    {
    nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
    nrf_gpio_cfg_output(DICE_LED1_PIN_NO);
    nrf_gpio_pin_clear(DICE_LED2_PIN_NO);
    nrf_gpio_cfg_output(DICE_LED2_PIN_NO);
}


/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if (button_action == APP_BUTTON_PUSH)
    {
        switch (pin_no)
        {
            case HR_INC_BUTTON_PIN_NO:
                // Increase Heart Rate measurement
                m_cur_heart_rate += HEART_RATE_CHANGE;
                if (m_cur_heart_rate > MAX_HEART_RATE)
                {
                    m_cur_heart_rate = MIN_HEART_RATE; // Loop back
                }
                break;
                
            case HR_DEC_BUTTON_PIN_NO:
                // Decrease Heart Rate measurement
                m_cur_heart_rate -= HEART_RATE_CHANGE;
                if (m_cur_heart_rate < MIN_HEART_RATE)
                {
                    m_cur_heart_rate = MAX_HEART_RATE; // Loop back
                }
                break;
                
            default:
                APP_ERROR_HANDLER(pin_no);
                break;
        }
    }    
}


/*****************************************************************************
* Static Initialization Functions
*****************************************************************************/

/**@brief Accelerometer measurement timer timeout handler.
 *
 * @details This function will be called each time the BMA222 accelration measurement timer expires.
 *
 * @param[in]   p_ev_data   Event data.
 */
static void bma222_accel_meas_timeout_handler(void * p_context)
{
   uint32_t err_code;
   
   static uint8_t prev_axis_x, prev_axis_y, prev_axis_z;
   UNUSED_PARAMETER(p_context);
   nrf_gpio_pin_toggle(DICE_LED2_PIN_NO);
   #ifdef REAL_ACCEL
      uint8_t axis_x, axis_y, axis_z;
      // Retrieve accelerometer data from BMA222
	    nrf_gpio_pin_set(DICE_LED1_PIN_NO);
      bma222_acc_read();
	    nrf_gpio_pin_set(DICE_LED2_PIN_NO);
         
      // Transmit buffer
      axis_x = bma222_getX();
      axis_y = bma222_getY();
      axis_z = bma222_getZ();
   #else
      static uint8_t axis_x, axis_y, axis_z;
      uint8_t delay;
      delay = 0;
         
      if (delay==0) {
          axis_x = axis_x + 1;
          axis_y = axis_y + 1;
          axis_z = axis_z + 1;
      }
   #endif

   if ((axis_x != prev_axis_x) | (axis_y != prev_axis_y) | (axis_z != prev_axis_z))
   {
       ble_acs_on_button_change(&m_lbs, 0, axis_x, axis_y, axis_z);
       prev_axis_x = axis_x;
       prev_axis_y = axis_y;
       prev_axis_z = axis_z;
   }
}

/**@brief Function for the Timer initialization.
 *
* @details Initializes the timer module. This creates and starts application timers.
*/
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_accel_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                bma222_accel_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    ble_uuid_t adv_uuids[] =
    {
        //{BLE_UUID_HEART_RATE_SERVICE,         BLE_UUID_TYPE_BLE},
        {0x1523,         BLE_UUID_TYPE_BLE},
        {BLE_UUID_BATTERY_SERVICE,            BLE_UUID_TYPE_BLE},
        {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
        
    };
		// YOUR_JOB: Use UUIDs for service(s) used in your application.
    ble_uuid_t adv_lbs_uuids[] = {{ACS_UUID_SERVICE, m_lbs.uuid_type}
    };

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(flags);
    advdata.flags.p_data            = &flags;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;                           // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
}


/**@brief Function for initializing the services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_hrs_init_t hrs_init;
    ble_bas_init_t bas_init;
    ble_dis_init_t dis_init;
	ble_acs_init_t acs_init;
    ble_dis_pnp_id_t pnp_id;
  	ble_srv_utf8_str_t sn_str_t;
    uint8_t        body_sensor_location;
    char sn_str[13]; // 12 characters and terminator

    // Initialize Heart Rate Service.
    body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

    memset(&hrs_init, 0, sizeof(hrs_init));

    hrs_init.is_sensor_contact_supported = false;
    hrs_init.p_body_sensor_location      = &body_sensor_location;

    // Here the sec level for the Heart Rate Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_bsl_attr_md.write_perm);

    err_code = ble_hrs_init(&m_hrs, &hrs_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
    
	// Initialize the Dice Accelerometer Service	
    acs_init.led_write_handler = led_write_handler;
       // m_lbs.uuid_type = BLE_UUID_TYPE_BLE;
    err_code = ble_acs_init(&m_lbs, &acs_init);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_hrs.hrm_handles.cccd_handle;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in]   p_evt   Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const    * p_handle,
                                           dm_event_t const     * p_event,
                                           api_result_t           event_result)
{
    APP_ERROR_CHECK(event_result);
    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 */
static void device_manager_init(void)
{
    uint32_t                err_code;
    dm_init_param_t         init_data;
    dm_application_param_t  register_param;
    
    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    // Clear all bonded centrals if the Bonds Delete button is pushed.
    init_data.clear_persistent_data = (nrf_gpio_pin_read(BOND_DELETE_ALL_BUTTON_ID) == 0);

    err_code = dm_init(&init_data);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));
    
    register_param.sec_param.timeout      = SEC_PARAM_TIMEOUT;
    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_4000MS_CALIBRATION, false);

    // Enable BLE stack 
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GPIOTE module.
 */
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


/**@brief Function for initializing the button module.
 */
static void buttons_init(void)
{
    // Configure HR_INC_BUTTON_PIN_NO and HR_DEC_BUTTON_PIN_NO as wake up buttons and also configure
    // for 'pull up' because the eval board does not have external pull up resistors connected to
    // the buttons.
    static app_button_cfg_t buttons[] =
    {
        {HR_INC_BUTTON_PIN_NO, false, BUTTON_PULL, button_event_handler},
        {HR_DEC_BUTTON_PIN_NO, false, BUTTON_PULL, button_event_handler}  // Note: This pin is also BONDMNGR_DELETE_BUTTON_PIN_NO
    };
    
    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);
}


/*****************************************************************************
* Static Start Functions
*****************************************************************************/

/**@brief Function for starting the application timers.
 */
static void application_timers_start(void)
{
    uint32_t err_code;

    // Start application timers
    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_accel_timer_id, BMA222_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    led_start();
}


/**@brief Function for putting the chip in System OFF Mode
 */
static void system_off_mode_enter(void)
{
    uint32_t err_code;
    uint32_t count;
    
    // Verify if there is any flash access pending, if yes delay starting advertising until 
    // it's complete.
    err_code = pstorage_access_status_get(&count);
    APP_ERROR_CHECK(err_code);
    
    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }

    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/*****************************************************************************
* Static Event Handling Functions
*****************************************************************************/

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t        err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            led_stop();
            
            // Initialize the current heart rate to the average of max and min values. So that
            // everytime a new connection is made, the heart rate starts from the same value.
            m_cur_heart_rate = (MAX_HEART_RATE + MIN_HEART_RATE) / 2;

            // Start timers used to generate battery and HR measurements.
            application_timers_start();

            // Start handling button presses
            //err_code = app_button_enable();
            //APP_ERROR_CHECK(err_code);
				
            break;

        case BLE_GAP_EVT_DISCONNECTED:            
            // @note Flash access may not be complete on return of this API. System attributes are now
            // stored to flash when they are updated to ensure flash access on disconnect does not
            // result in system powering off before data was successfully written.

            // Go to system-off mode, should not return from this function, wakeup will trigger
            // a reset.
            system_off_mode_enter();
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
            {
                led_stop();
                
                nrf_gpio_cfg_sense_input(HR_INC_BUTTON_PIN_NO,
                                         BUTTON_PULL, 
                                         NRF_GPIO_PIN_SENSE_LOW);

                nrf_gpio_cfg_sense_input(HR_DEC_BUTTON_PIN_NO,
                                         BUTTON_PULL, 
                                         NRF_GPIO_PIN_SENSE_LOW);

                system_off_mode_enter();
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void on_sys_evt(uint32_t sys_evt)
{
    switch(sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
        case NRF_EVT_FLASH_OPERATION_ERROR:
            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                system_off_mode_enter();
            }
            break;
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_hrs_on_ble_evt(&m_hrs, p_ble_evt);
    ble_acs_on_ble_evt(&m_lbs, p_ble_evt);
    ble_bas_on_ble_evt(&bas, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 *@brief Function for the Power manager.
 **/
void GPIOTE_IRQHandler(void)
{
    uint32_t               err_code;
        
        // Handle button interrupts
    if (
       (NRF_GPIOTE->EVENTS_IN[0] != 0)
        &&
       ((NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk) != 0)
       )
       {
        NRF_GPIOTE->EVENTS_IN[0] = 0;
        
        err_code = app_timer_start(m_accel_timer_id, BMA222_MEAS_INTERVAL, NULL);

        APP_ERROR_CHECK(err_code);
        
		if (s_useLEDs)
        {    
           flash_timeout = LED_TOGGLE_SCOUNT;
		   //if (s_LED_state == 0) {
	       //   nrf_gpio_pin_set(DICE_LED1_PIN_NO);  
	       //} else if (s_LED_state == 1) {
	       //   nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
	       //}
        } 

        // Set the timeout counter for count down
        tx_timeout = TX_TIMEOUT_SCOUNT;
    }


    // Handle unexpected interrupts
    if (
       (NRF_GPIOTE->EVENTS_IN[1] != 0)
       &&
       ((NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN1_Msk) != 0)
       )
    {
        //s_freefall = true;
        NRF_GPIOTE->EVENTS_IN[1] = 0;
        // Interupt 1 routine
    }

    // unexpected interrupts
    if (
       (NRF_GPIOTE->EVENTS_IN[2] != 0)
       &&
       ((NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN2_Msk) != 0)
       )
    {
        NRF_GPIOTE->EVENTS_IN[2] = 0;
}

    if (
       (NRF_GPIOTE->EVENTS_IN[3] != 0)
       &&
       ((NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN3_Msk) != 0)
       )
    {
        NRF_GPIOTE->EVENTS_IN[3] = 0;
}

    if (
       (NRF_GPIOTE->EVENTS_PORT != 0)
       &&
       ((NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_PORT_Msk) != 0)
       )
    {
        NRF_GPIOTE->EVENTS_PORT = 0;
}
}


/**@brief Initialize GPIOTE module for detecting acceleration interrupts.
 */
static void acc_int_init(void)
{
    //uint32_t err_code;
    //uint32_t per_rdy;

    // Initialize GPIOTE module
    NRF_GPIOTE->INTENCLR = 0xffffffffUL;
    NRF_GPIOTE->CONFIG[0]    = GPIOTE_CONFIG_BUTTON(ACCEL_INT1_PIN_NO);
    NRF_GPIOTE->CONFIG[1]    = GPIOTE_CONFIG_BUTTON(ACCEL_INT2_PIN_NO);
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NRF_GPIOTE->EVENTS_IN[1] = 0;
    NRF_GPIOTE->INTENSET     = GPIOTE_INTENSET_IN0_Msk | GPIOTE_INTENSET_IN1_Msk ;

    // Turn on the sense detect to trigger wakeup when the device is asleep.
    NRF_GPIO->PIN_CNF[ACCEL_INT1_PIN_NO] = (GPIO_PIN_CNF_SENSE_High     << GPIO_PIN_CNF_SENSE_Pos);
    NRF_GPIO->PIN_CNF[ACCEL_INT2_PIN_NO] = (GPIO_PIN_CNF_SENSE_High     << GPIO_PIN_CNF_SENSE_Pos);

    //err_code = sd_nvic_ClearPendingIRQ(GPIOTE_IRQn);
	NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    // nrf_gpio_pin_clear(DICE_LED2_PIN_NO);
    // nrf_gpio_pin_set(DICE_LED2_PIN_NO);
    
    NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_HIGH);

	NVIC_EnableIRQ(GPIOTE_IRQn);
}

 /*
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
}


/*****************************************************************************
* Main Function
*****************************************************************************/

/**@brief Function for the application main entry.
 */
int main(void)
{
    uint32_t err_code;
    // Initialize
    leds_init();
	
	// Double flash the LED's to validate initialization 
	//doubleFlashAlternating();

    // Activate the TWI interface for accelerometer comms
    twi_master_init();
	  //doubleFlashAlternating();

    // Reset and initialize the accelerometer
    bma222_setDeviceAddress(BMA222E_BASE_ADDRESS_DEV0);
    if (bma222_reset()) {
        bma222_init(BMA222E_BASE_ADDRESS_DEV0);
	} else {
		bma222_setDeviceAddress(BMA222_BASE_ADDRESS_DEV0);
		bma222_reset();
		bma222_init(BMA222_BASE_ADDRESS_DEV0);
		
	}
	
	
    timers_init();
    gpiote_init();

	
    buttons_init();
    
    ble_stack_init();
    doubleFlashAlternating();


    device_manager_init();


    // Initialize Bluetooth Stack parameters.
    gap_params_init();
    advertising_init();
    services_init();
    conn_params_init();

    // Start advertising.
    advertising_start();


    // Enter main loop.
    for (;;)
    {
        // Switch to a low power state until an event is available for the application
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}

void doubleFlashAlternating(void) {
	   uint8_t i = 0;
    // Turn LED's off and on alternatley.  Always turn off previous LED first before turning on other LED.
           //nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
           //  for (i = 0; i < 10; i++) {
           //     dly5ms();
           //  }
           // nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
           //  for (i = 0; i < 10; i++) {
           //     dly5ms();
           //  }
       for (int jj = 0; jj < 2; jj = jj +1) {
           nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
                 //dly50ms();
                 //nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_set(DICE_LED1_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_set(DICE_LED1_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_clear(DICE_LED1_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_set(DICE_LED1_PIN_NO);

                 //nrf_gpio_pin_clear(DICE_LED2_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_clear(DICE_LED2_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_set(DICE_LED2_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_clear(DICE_LED2_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_set(DICE_LED2_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_clear(DICE_LED2_PIN_NO);
                 dly50ms();
                 nrf_gpio_pin_set(DICE_LED2_PIN_NO);
       }

}

/**
 * @}
 */

void dly5ms(void)				  // 5 miliseconds delay 
{ unsigned char j = 100      ;		// 100 * 50usec = 5 msec 
	while (--j) {
		nrf_delay_us(50)   ;
	}  
}

void dly50ms(void)				  // 50 miliseconds delay 
{ unsigned int j = 1000      ;		// 1000 * 50usec = 50 msec 
	while (--j) {
		nrf_delay_us(50)   ;
	}  
}

void dly100ms(void)				  // 100 miliseconds delay 
{ unsigned int j = 2000      ;		// 2000 * 50usec = 100 msec 
	while (--j) {
		nrf_delay_us(50)   ;
	}  
}

void dly250ms(void)				  // 250 miliseconds delay 
{ unsigned char k = 50       ;		//  50 * 5 msec = 250 msec 
	while (--k) {
		dly5ms()      ;
	}	 // usefull for messages' display
}  

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

//438340e7-7675-4236-a2d4-446e41499891 

#define CUSTOM_SERVICE_UUID_BASE         {0x91, 0x98, 0x49, 0x41, 0x6E, 0x44, 0xD4, 0xA2, \
                                          0x36, 0x42, 0x75, 0x76, 0xE7, 0x40, 0x83, 0x43}

#define CUSTOM_VALUE_CHAR_UUID            0x1401
#define CUSTOM_SERVICE_UUID               0x1400
#define SETTINGS_CHAR_UUID                0x1399
#define CALIBRATION_CHAR_UUID             0x1398

/**@brief   Macro for defining a ble_cus instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_CUS_DEF(_name)                                                                          \
static ble_cus_t _name;  

// Forward declaration of the ble_cus_t type.
typedef struct ble_cus_s ble_cus_t;

typedef void (*ble_lbs_calib_write_handler_t) (uint16_t conn_handle, ble_cus_t * p_cus, uint8_t new_state);
typedef void (*ble_lbs_setting_write_handler_t) (uint16_t conn_handle, ble_cus_t * p_cus, uint8_t new_state);

/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/

typedef enum
{
    BLE_CUS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_CUS_EVT_NOTIFICATION_DISABLED,                            /**< Custom value notification disabled event. */
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED
} ble_cus_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_cus_evt_type_t evt_type;                                  /**< Type of event. */
} ble_cus_evt_t;

typedef void (*ble_cus_evt_handler_t) (ble_cus_t * p_cus, ble_cus_evt_t * p_evt);

typedef struct
{
    ble_cus_evt_handler_t         evt_handler;  
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_lbs_calib_write_handler_t calibration_handler;
    ble_lbs_setting_write_handler_t settings_handler;
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */

} ble_cus_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_s
{
    ble_cus_evt_handler_t         evt_handler;
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;          /**< Handles related to the Custom Value characteristic. */
    ble_gatts_char_handles_t      settings_handles; 
    ble_gatts_char_handles_t      calibration_handles; 
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);

uint32_t ble_lbs_on_button_change(uint16_t conn_handle, uint8_t * data_arr);

void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt);

static void calibration_handler(uint16_t conn_handle, ble_cus_t * p_cus, uint8_t calib_state);

static void settings_handler(uint16_t conn_handle, ble_cus_t * p_cus, uint8_t setting_state);

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value);
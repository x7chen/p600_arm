
#ifndef BLE_DEVINFO_H__
#define BLE_DEVINFO_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>
#include "ble_type.h"
#define BLE_UUID_DEVINFO_SERVICE 0x1601                      /**< The UUID of the Nordic UART Service. */
#define BLE_DEVINFO_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

typedef struct ble_devinfo_s ble_devinfo_t;

typedef void (*ble_devinfo_data_handler_t) (ble_devinfo_t * p_devinfo, uint8_t * p_data, uint16_t length);

typedef struct
{
    ble_devinfo_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_devinfo_init_t;


struct ble_devinfo_s
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
	ble_gatts_char_handles_t navigation_handles;
	ble_gatts_char_handles_t navigation_b_handles;
	ble_gatts_char_handles_t message_info_handles;
	ble_gatts_char_handles_t message_data_handles;
	ble_gatts_char_handles_t computers_handles;
	ble_gatts_char_handles_t feedback_handles;
    ble_gatts_char_handles_t time_sync_handles;
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
};

typedef struct{
    callback_t on_navigation_update;
    callback_t on_navigation_b_update;
    callback_t on_message_info_update;
    callback_t on_message_data_update;
    callback_t on_computers_update;
    callback_t on_feedback_update;
    callback_t on_time_sync_update;
}devinfo_callbacks_t;

void register_devinfo_callbacks(devinfo_callbacks_t *callbacks);
uint32_t ble_devinfo_init(ble_devinfo_t * p_devinfo, const ble_devinfo_init_t * p_devinfo_init);
void ble_devinfo_on_ble_evt(ble_devinfo_t * p_devinfo, ble_evt_t * p_ble_evt);


#endif 

/** @} */

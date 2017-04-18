
#ifndef BLE_FIRMWARE_H__
#define BLE_FIRMWARE_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>
#include "ble_type.h"
#define BLE_UUID_FIRMWARE_SERVICE 0x1801                      
#define BLE_FIRMWARE_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) 

typedef struct ble_firmware_s ble_firmware_t;

typedef void (*ble_firmware_data_handler_t) (ble_firmware_t * p_firmware, uint8_t * p_data, uint16_t length);

typedef struct
{
    ble_firmware_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_firmware_init_t;


struct ble_firmware_s
{
    uint8_t                  uuid_type;               /**< UUID type for FIRMWARE Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of FIRMWARE Service (as provided by the S110 SoftDevice). */
	ble_gatts_char_handles_t firmware_data_handles;
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; 
};


typedef struct{
    callback_t on_firmware_data_update;
}firmware_callbacks_t;

void register_firmware_callbacks(firmware_callbacks_t *callbacks);
uint32_t ble_firmware_init(ble_firmware_t * p_firmware, const ble_firmware_init_t * p_firmware_init);
void ble_firmware_on_ble_evt(ble_firmware_t * p_firmware, ble_evt_t * p_ble_evt);


#endif 

/** @} */

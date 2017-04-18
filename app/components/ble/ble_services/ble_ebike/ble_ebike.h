
#ifndef BLE_EBIKE_H__
#define BLE_EBIKE_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#define BLE_UUID_EBIKE_SERVICE 0x1501                      /**< The UUID of the Nordic UART Service. */
#define BLE_EBIKE_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

typedef struct ble_ebike_s ble_ebike_t;

typedef void (*ble_ebike_data_handler_t) (ble_ebike_t * p_ebike, uint8_t * p_data, uint16_t length);

typedef struct
{
    bool                          is_notification_enabled;
} ble_ebike_init_t;


struct ble_ebike_s
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t info_handles;				//1226
	ble_gatts_char_handles_t battery_info_handles;
	ble_gatts_char_handles_t firmware_id_handles;		//1226
	ble_gatts_char_handles_t sensor_data_handles;
	ble_gatts_char_handles_t trip_handles;
	ble_gatts_char_handles_t update_handles;
	ble_gatts_char_handles_t battery_handles;
	ble_gatts_char_handles_t battery_cell_handles;
	ble_gatts_char_handles_t stats_handles;
	ble_gatts_char_handles_t serial_number_handles;
	ble_gatts_char_handles_t feedback_handles;
	ble_gatts_char_handles_t error_handles;
	ble_gatts_char_handles_t history_error_handles;
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
};


uint32_t ble_ebike_init(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init);


void ble_ebike_on_ble_evt(ble_ebike_t * p_ebike, ble_evt_t * p_ble_evt);

uint32_t ble_ebike_info_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_battery_info_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_firmware_id_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_sensor_data_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_trip_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_update_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_battery_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_battery_cell_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_stats_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_serial_number_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_feedback_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_error_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);
uint32_t ble_ebike_history_error_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length);



#endif 

/** @} */

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

#include "ble_firmware.h"
#include <string.h>
#include "sdk_common.h"
#include "ble_srv_common.h"
#include "ble_type.h"

#define BLE_UUID_FIRMWARE_CHARACTERISTIC_NAVIGATION		0x1802

#define FIRMWARE_BASE_UUID                  {{0xDB,0x45,0x5A,0xAF,0x93,0xE0,0xDA,0xBC,0xC6,0x40,0x00,0xAF,0x00,0x00,0x1A,0xDA}}

static firmware_callbacks_t * m_callbacks = NULL;

void firmware_callback_is_null()
{

}
void register_firmware_callbacks(firmware_callbacks_t *callbacks)
{
    m_callbacks = callbacks;
    if(m_callbacks->on_firmware_data_update == NULL)
    {
        m_callbacks->on_firmware_data_update = (callback_t)firmware_callback_is_null;
    }
}

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_firmware     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_firmware_t * p_firmware, ble_evt_t * p_ble_evt)
{
    p_firmware->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_firmware     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_firmware_t * p_firmware, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_firmware->conn_handle = BLE_CONN_HANDLE_INVALID;
   // p_firmware->is_notification_enabled = false;

}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_firmware     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_firmware_t * p_firmware, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_firmware->firmware_data_handles.value_handle)
    {
        m_callbacks->on_firmware_data_update(p_evt_write->data, p_evt_write->len);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}

static uint32_t firmWare_data_characteristic_add(ble_firmware_t * p_firmware, const ble_firmware_init_t * p_firmware_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_firmware->uuid_type;
    ble_uuid.uuid = BLE_UUID_FIRMWARE_CHARACTERISTIC_NAVIGATION;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_FIRMWARE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_firmware->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_firmware->firmware_data_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

void ble_firmware_on_ble_evt(ble_firmware_t * p_firmware, ble_evt_t * p_ble_evt)
{
    if ((p_firmware == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_firmware, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_firmware, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_firmware, p_ble_evt);
            break;
        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_firmware_init(ble_firmware_t * p_firmware, const ble_firmware_init_t * p_firmware_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t firmware_base_uuid = FIRMWARE_BASE_UUID;

    if ((p_firmware == NULL) || (p_firmware_init == NULL))
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.
    p_firmware->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_firmware->is_notification_enabled = false;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&firmware_base_uuid, &p_firmware->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_firmware->uuid_type;
    ble_uuid.uuid = BLE_UUID_FIRMWARE_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_firmware->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

	err_code = firmWare_data_characteristic_add(p_firmware, p_firmware_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

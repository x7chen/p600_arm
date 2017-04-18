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
#include <string.h>
#include "ble_ebike.h"
#include "ble_srv_common.h"
#include "sdk_common.h"


#define BLE_UUID_EBIKE_CHARACTERISTIC_INFO			0x1502
#define BLE_UUID_EBIKE_CHARACTERISTIC_BATTERY_INFO	0x1503
#define BLE_UUID_EBIKE_CHARACTERISTIC_FIRMWARE_ID	0x1504
#define BLE_UUID_EBIKE_CHARACTERISTIC_SENSOR_DATA	0x1505
#define BLE_UUID_EBIKE_CHARACTERISTIC_TRIP			0x1506
#define BLE_UUID_EBIKE_CHARACTERISTIC_UPDATE		0x1507
#define BLE_UUID_EBIKE_CHARACTERISTIC_BATTERY		0x1508
#define BLE_UUID_EBIKE_CHARACTERISTIC_BATTERY_CELL	0x1509
#define BLE_UUID_EBIKE_CHARACTERISTIC_STATS			0x150A
#define BLE_UUID_EBIKE_CHARACTERISTIC_SERIAL_NUMBER	0x150B
#define BLE_UUID_EBIKE_CHARACTERISTIC_FEEDBACK		0x150C
#define BLE_UUID_EBIKE_CHARACTERISTIC_ERROR			0x150D
#define BLE_UUID_EBIKE_CHARACTERISTIC_HISTORY_ERROR	0x150E
#define EBIKE_BASE_UUID                  {{0xDB,0x45,0x5A,0xAF,0x93,0xE0,0xDA,0xBC,0xC6,0x40,0x00,0xAF,0x00,0x00,0x1A,0xDA}}
//#define EBIKE_BASE_UUID                  {{0xFF, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x18}}


/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_ebike     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_ebike_t * p_ebike, ble_evt_t * p_ble_evt)
{
    p_ebike->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_ebike     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_ebike_t * p_ebike, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ebike->conn_handle = BLE_CONN_HANDLE_INVALID;
   // p_ebike->is_notification_enabled = false;

}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_ebike     ebike Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_ebike_t * p_ebike, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (
        (p_evt_write->handle == p_ebike->battery_handles.cccd_handle)
        &&
        (p_evt_write->len == 2)
       )
    {
//        if (ble_srv_is_notification_enabled(p_evt_write->data))
//        {
//            p_ebike->is_notification_enabled = true;
//        }
//        else
//        {
//            p_ebike->is_notification_enabled = false;
//        }
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}
static void on_tx_complete(ble_ebike_t * p_ebike, ble_evt_t * p_ble_evt)
{

    //if there's event need to schedule

}


static uint32_t info_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_INFO;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->info_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}
static uint32_t battery_info_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_BATTERY_INFO;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->battery_info_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}
static uint32_t firmware_id_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_FIRMWARE_ID;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->firmware_id_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}
static uint32_t sensor_data_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
	
	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_SENSOR_DATA;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->sensor_data_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}



static uint32_t trip_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_TRIP;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->trip_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}



static uint32_t update_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_UPDATE;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->update_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

static uint32_t battery_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_BATTERY;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->battery_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

static uint32_t battery_cell_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_BATTERY_CELL;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->battery_cell_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

static uint32_t stats_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_STATS;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->stats_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}
static uint32_t serial_number_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_SERIAL_NUMBER;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->serial_number_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}


static uint32_t feedback_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_FEEDBACK;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->feedback_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

static uint32_t error_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_ERROR;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->error_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

static uint32_t history_error_characteristic_add(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
	/**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

	char_md.char_props.read = 1;
    char_md.char_props.notify = (p_ebike->is_notification_enabled) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_CHARACTERISTIC_HISTORY_ERROR;

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
    attr_char_value.init_len  = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_EBIKE_MAX_DATA_LEN;

    return sd_ble_gatts_characteristic_add(p_ebike->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ebike->history_error_handles);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}

void ble_ebike_on_ble_evt(ble_ebike_t * p_ebike, ble_evt_t * p_ble_evt)
{
    if ((p_ebike == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ebike, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ebike, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ebike, p_ble_evt);
            break;
        case BLE_EVT_TX_COMPLETE:
            on_tx_complete(p_ebike,p_ble_evt);
            break;
        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_ebike_init(ble_ebike_t * p_ebike, const ble_ebike_init_t * p_ebike_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t ebike_base_uuid = EBIKE_BASE_UUID;

    if ((p_ebike == NULL) || (p_ebike_init == NULL))
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.
    p_ebike->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_ebike->is_notification_enabled = p_ebike_init->is_notification_enabled;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&ebike_base_uuid, &p_ebike->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_ebike->uuid_type;
    ble_uuid.uuid = BLE_UUID_EBIKE_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_ebike->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
//    if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }

	err_code = info_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = battery_info_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = firmware_id_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = sensor_data_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = trip_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = update_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = battery_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = battery_cell_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = stats_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = serial_number_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = feedback_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = error_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = history_error_characteristic_add(p_ebike, p_ebike_init);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}
uint32_t ble_ebike_info_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->info_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->info_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}
uint32_t ble_ebike_battery_info_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->battery_info_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->battery_info_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_firmware_id_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->firmware_id_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->firmware_id_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}
uint32_t ble_ebike_sensor_data_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->sensor_data_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->sensor_data_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_trip_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->trip_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->trip_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_update_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->update_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->update_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}



uint32_t ble_ebike_battery_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->battery_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->battery_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}
uint32_t ble_ebike_battery_cell_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->battery_cell_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->battery_cell_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_stats_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->stats_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->stats_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}
uint32_t ble_ebike_serial_number_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->serial_number_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->serial_number_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_feedback_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->feedback_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->feedback_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_error_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->error_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->error_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

uint32_t ble_ebike_history_error_write(ble_ebike_t * p_ebike, uint8_t * p_dat, uint16_t length)
{
	uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    if (p_ebike == NULL)
    {
        return NRF_ERROR_NULL;
    }
	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = length;
	gatts_value.offset  = 0;
	gatts_value.p_value = p_dat;


	// Update database.
	err_code = sd_ble_gatts_value_set(p_ebike->conn_handle,
									  p_ebike->history_error_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_ebike->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ebike->is_notification_enabled)
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_ebike->history_error_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_ebike->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}
    

    return err_code;
}

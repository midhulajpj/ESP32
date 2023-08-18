/*
 * BLEManager.h
 *
 *  Created on: Aug 2, 2023
 *      Author: MPJ
 */

#ifndef MAIN_BLEMANAGER_H_
#define MAIN_BLEMANAGER_H_

#define BT_SERV_PROFILE_NUM               1
#define BT_SERV_PROFILE_APP_IDX           0
#define BT_SERV_RATE_APP_ID            0x55
#define DEVICE_NAME                "BT_SERV"
#define BT_SERV_SVC_INST_ID               0

#define CHAR_VAL_LEN_MAX               0xFF
#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

#define BT_SERV_CHAR_1_UUID           0xF0F1
#define BT_SERV_CHAR_2_UUID           0xF0F2
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

///Attributes State Machine
enum
{
    BTS_IDX_SVC,

	BTS_IDX_C1_CHAR,
	BTS_IDX_C1_VAL,
	BTS_IDX_C1_CFG,

	BTS_IDX_C2_CHAR,
	BTS_IDX_C2_VAL,

	BTS_IDX_NB,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

typedef struct{
	esp_gatt_if_t gatts_if;
	uint16_t conn_id;
	uint64_t time_accessed;
}BLE_WCMD_S;

class BLEManager {
private:
	static BLEManager s_instance;
	BLE_WCMD_S ble_cmd_s;
	uint8_t adv_config_done;
	uint16_t svc_app_handle_table[BTS_IDX_NB];
	uint8_t test_manufacturer[3]={'E', 'S', 'P'};
	uint8_t factory_service_uuid[16] = {
		/* LSB <--------------------------------------------------------------------------------> MSB */
		//first uuid, 16bit, [12],[13] is the value
		0x66,0xe1,0x91,0x96,0x6b,0x4d,0xa5,0x8a,0x62,0x4e,0x42,0xbd,0xc3,0x4b,0xed,0x02
	};
	esp_ble_adv_data_t bt_serv_adv_config;
	esp_ble_adv_data_t bt_serv_scan_rsp_config;
	esp_ble_adv_params_t bt_serv_adv_params;
	struct gatts_profile_inst bt_serv_profile_tab[BT_SERV_PROFILE_NUM];
	/// GATT Service
	const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
	const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
	const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
	const uint8_t  char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ;
	const uint8_t  char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	const uint16_t char_1_uuid = BT_SERV_CHAR_1_UUID;
	const uint16_t char_2_uuid = BT_SERV_CHAR_2_UUID;
	uint16_t notification_conn_id;

	esp_gatts_attr_db_t bt_serv_gatt_db[BTS_IDX_NB] =
	{
		// Service Declaration
		[BTS_IDX_SVC]                    =
		{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
		  sizeof(uint16_t), sizeof(factory_service_uuid), (uint8_t *)&factory_service_uuid}},


		// Characteristic-1 Declaration
		[BTS_IDX_C1_CHAR]  =
		{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		  CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

		// Characteristic-1 Value
		[BTS_IDX_C1_VAL]   =
		{{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&char_1_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		  sizeof(uint8_t), 0,NULL}},

		/* Characteristic-1 Configuration Descriptor */
		[BTS_IDX_C1_CFG] =
		{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
				  sizeof(uint16_t), 0,NULL}},

		// Characteristic-2 Declaration
		[BTS_IDX_C2_CHAR]          =
		{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
		  CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

		// Characteristic-2 Value
		[BTS_IDX_C2_VAL]             =
		{{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&char_2_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		  sizeof(uint8_t), 0,NULL}},
	};

	void show_bonded_devices();
	[[maybe_unused]] static void remove_all_bonded_devices();
	uint16_t getAttributeIndexByAppTable(uint16_t attributeHandle);
	void getDeviceID(char *deviceid,size_t max);
	void ReadAndConfigureServiceUUID(void);

	void esp_key_type_to_str(esp_ble_key_type_t key_type,char* name);
	void esp_auth_req_to_str(esp_ble_auth_req_t auth_req,char* name);


public:
	bool isConnected;
	BLEManager();
	virtual ~BLEManager();
	static BLEManager* getInstance();

	static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
	static void gatts_profile_event_handler(esp_gatts_cb_event_t event,esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
	static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

	void InitializeBLE(void);
	void sendNotication(uint8_t *data,uint16_t length);
};

#endif /* MAIN_BLEMANAGER_H_ */

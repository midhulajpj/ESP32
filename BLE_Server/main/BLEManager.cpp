/*
 * BLEManager.cpp
 *
 *  Created on: August 2, 2023
 *      Author: MPJ
 */

#include "includes.h"

const char* TAG = "BLE";

BLEManager BLEManager::s_instance;


BLEManager::BLEManager() {
	isConnected=false;
	adv_config_done=0;
	notification_conn_id=0;
	// config adv data
	bt_serv_adv_config.set_scan_rsp = false;
	bt_serv_adv_config.include_txpower = true;
	bt_serv_adv_config.min_interval = 0x0006; //slave connection min interval; Time = min_interval * 1bt_serv_adv_config.25 msec
	bt_serv_adv_config.max_interval = 0x0010; //slave connection max interval; Time = max_interval * 1bt_serv_adv_config.25 msec
	bt_serv_adv_config.appearance = 0x00;
	bt_serv_adv_config.manufacturer_len = 0; //TEST_MANUFACTURER_DATA_LEN;
	bt_serv_adv_config.p_manufacturer_data =  NULL; //&test_manufacturer[0];
	bt_serv_adv_config.service_data_len = 0;
	bt_serv_adv_config.p_service_data = NULL;
	bt_serv_adv_config.service_uuid_len = ESP_UUID_LEN_128;
	bt_serv_adv_config.p_service_uuid = factory_service_uuid;
	bt_serv_adv_config.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
	// config scan response data
	bt_serv_scan_rsp_config.set_scan_rsp = true;
	bt_serv_scan_rsp_config.include_name = true;
	bt_serv_scan_rsp_config.manufacturer_len = sizeof(test_manufacturer);
	bt_serv_scan_rsp_config.p_manufacturer_data = test_manufacturer;

	bt_serv_adv_params.adv_int_min        = 0x100;
	bt_serv_adv_params.adv_int_max        = 0x100;
	bt_serv_adv_params.adv_type           = ADV_TYPE_IND;
	bt_serv_adv_params.own_addr_type      = BLE_ADDR_TYPE_RANDOM;
	bt_serv_adv_params.channel_map        = ADV_CHNL_ALL;
	bt_serv_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

	memset(&ble_cmd_s,0,sizeof(BLE_WCMD_S ));

}

BLEManager::~BLEManager() {
	// TODO Auto-generated destructor stub
}

BLEManager* BLEManager::getInstance(){
	return &s_instance;
}

void BLEManager::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
	ESP_LOGV(TAG, "GAP_EVT, event %d\n", event);
	switch (event) {
	    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
	        s_instance.adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
	        if (s_instance.adv_config_done == 0){
	            esp_ble_gap_start_advertising(&s_instance.bt_serv_adv_params);
	        }
	        break;
	    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
	    	s_instance.adv_config_done &= (~ADV_CONFIG_FLAG);
	        if (s_instance.adv_config_done == 0){
	            esp_ble_gap_start_advertising(&s_instance.bt_serv_adv_params);
	        }
	        break;
	    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
	        //advertising start complete event to indicate advertising start successfully or failed
	        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
	            ESP_LOGE(TAG, "advertising start failed, error status = %x", param->adv_start_cmpl.status);
	            break;
	        }
	        ESP_LOGI(TAG, "advertising start success");
	        break;
	    case ESP_GAP_BLE_PASSKEY_REQ_EVT:                           /* passkey request event */
	        ESP_LOGI(TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
	        /* Call the following function to input the passkey which is displayed on the remote device */
	        //esp_ble_passkey_reply(bt_serv_profile_tab[HEART_PROFILE_APP_IDX].remote_bda, true, 0x00);
	        break;
	    case ESP_GAP_BLE_OOB_REQ_EVT: {
	        ESP_LOGI(TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
	        uint8_t tk[16] = {1}; //If you paired with OOB, both devices need to use the same tk
	        esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
	        break;
	    }
	    case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
	        ESP_LOGI(TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
	        break;
	    case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
	        ESP_LOGI(TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
	        break;
	    case ESP_GAP_BLE_NC_REQ_EVT:
	        /* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
	        show the passkey number to the user to confirm it with the number displayed by peer device. */
	        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
	        //ESP_LOGI(TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%" PRIu32, param->ble_security.key_notif.passkey);
	        break;
	    case ESP_GAP_BLE_SEC_REQ_EVT:
	        /* send the positive(true) security response to the peer device to accept the security request.
	        If not accept the security request, should send the security response with negative(false) accept value*/
	        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
	        break;
	    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
	        ///show the passkey number to the user to input it in the peer device.
	        //ESP_LOGI(TAG, "The passkey Notify number:%06" PRIu32, param->ble_security.key_notif.passkey);
	        break;
	    case ESP_GAP_BLE_KEY_EVT:{
	        char keytypename[20];
	    	//shows the ble key info share with peer device to the user.
	        s_instance.esp_key_type_to_str(param->ble_security.ble_key.key_type,(char *)keytypename);
	        ESP_LOGI(TAG, "key type = %s",keytypename);
	        break;
	    }
	    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
	        esp_bd_addr_t bd_addr;
	        char authmodename[20];
	        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
	        ESP_LOGI(TAG, "remote BD_ADDR: %08x%04x",\
	                (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
	                (bd_addr[4] << 8) + bd_addr[5]);
	        ESP_LOGI(TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
	        ESP_LOGI(TAG, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
	        if(!param->ble_security.auth_cmpl.success) {
	            ESP_LOGI(TAG, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
	        } else {
	        	s_instance.esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode,(char *)authmodename);
	            ESP_LOGI(TAG, "auth mode = %s",authmodename);
	        }
	        //show_bonded_devices();
	        break;
	    }
	    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT: {
	        ESP_LOGD(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d", param->remove_bond_dev_cmpl.status);
	        ESP_LOGI(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
	        ESP_LOGI(TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
	        esp_log_buffer_hex(TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
	        ESP_LOGI(TAG, "------------------------------------");
	        break;
	    }
	    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:{
	        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
	            ESP_LOGE(TAG, "config local privacy failed, error status = %x", param->local_privacy_cmpl.status);
	            break;
	        }

	        esp_err_t ret = esp_ble_gap_config_adv_data(&s_instance.bt_serv_adv_config);
	        if (ret){
	            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
	        }else{
	        	s_instance.adv_config_done |= ADV_CONFIG_FLAG;
	        }

	        ret = esp_ble_gap_config_adv_data(&s_instance.bt_serv_scan_rsp_config);
	        if (ret){
	            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
	        }else{
	        	s_instance.adv_config_done |= SCAN_RSP_CONFIG_FLAG;
	        }

	        break;
	    }
	    default:
	        break;
	    }
}

void BLEManager::gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
	/* If event is register event, store the gatts_if for each profile */
	if (event == ESP_GATTS_REG_EVT) {
		if (param->reg.status == ESP_GATT_OK) {
			s_instance.bt_serv_profile_tab[BT_SERV_PROFILE_APP_IDX].gatts_if = gatts_if;
		} else {
			ESP_LOGI(TAG, "Reg app failed, app_id %04x, status %d\n",param->reg.app_id,param->reg.status);
			return;
		}
	}

	do {
		int idx;
		for (idx = 0; idx < BT_SERV_PROFILE_NUM  ; idx++) {
			if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
					gatts_if == s_instance.bt_serv_profile_tab[idx].gatts_if) {
				if (s_instance.bt_serv_profile_tab[idx].gatts_cb) {
					s_instance.bt_serv_profile_tab[idx].gatts_cb(event, gatts_if, param);
				}
			}
		}
	} while (0);
}

void BLEManager::gatts_profile_event_handler(esp_gatts_cb_event_t event,esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
	ESP_LOGV(TAG, "event = %x\n",event);
	switch (event) {
		case ESP_GATTS_REG_EVT:{
			char device_ID[20];
			s_instance.getDeviceID(device_ID, sizeof(device_ID));
			esp_ble_gap_set_device_name(device_ID);
			//generate a resolvable random address
			esp_ble_gap_config_local_privacy(true);
			esp_ble_gatts_create_attr_tab(s_instance.bt_serv_gatt_db, gatts_if,BTS_IDX_NB, BT_SERV_SVC_INST_ID);
			}
			break;
		case ESP_GATTS_READ_EVT:{
			int attrIndex=0;
			ESP_LOGI(TAG, "ESP_GATTS_READ_EVT");
			if((attrIndex=s_instance.getAttributeIndexByAppTable(param->read.handle))<BTS_IDX_NB){
				ESP_LOGI(TAG, "BLE Read Event:%s",(attrIndex==BTS_IDX_C1_VAL)?"Admin":((attrIndex==BTS_IDX_C2_VAL)?"User":(attrIndex==BTS_IDX_C1_CFG)?"CFG":"Unknown"));
			}
			esp_gatt_rsp_t rsp;
			memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
			rsp.attr_value.handle = param->read.handle;
			esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,ESP_GATT_OK, &rsp);
		}break;
		case ESP_GATTS_WRITE_EVT:{
				int attrIndex=0;
				if((attrIndex=s_instance.getAttributeIndexByAppTable(param->read.handle))<BTS_IDX_NB){
					ESP_LOGI(TAG, "ESP_GATTS_WRITE_EVT,write value:%s length:%u Event:%s",
							param->write.value,param->write.len,(attrIndex==BTS_IDX_C1_VAL)?"Admin":((attrIndex==BTS_IDX_C2_VAL)?"User":(attrIndex==BTS_IDX_C1_CFG)?"CFG":"Unknown"));
					esp_gatt_rsp_t rsp;
					memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
					rsp.attr_value.handle = param->read.handle;
					if(attrIndex==BTS_IDX_C1_CFG){
						uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
						if (descr_value == 0x0001){
							s_instance.notification_conn_id=param->write.conn_id;
							ESP_LOGI(TAG, "notify enabled connid:%u gattsif:%u",param->write.conn_id,gatts_if);
						}else if (descr_value == 0x0002){
							ESP_LOGI(TAG, "indicate enable");
						}else if (descr_value == 0x0000){
							ESP_LOGI(TAG, "notify/indicate disable connid:%u gattsif:%u",param->write.conn_id,gatts_if);
							s_instance.notification_conn_id=0;
						}else{
							ESP_LOGE(TAG, "unknown descr value");
						}
					}else if(attrIndex==BTS_IDX_C1_VAL){
						ESP_LOGE(TAG, "Characteristics-1 connid:%u gattsif:%u",param->write.conn_id,gatts_if);
					}else if(attrIndex==BTS_IDX_C2_VAL){
						ESP_LOGE(TAG, "Characteristics-2 connid:%u gattsif:%u",param->write.conn_id,gatts_if);
					}
					s_instance.ble_cmd_s.gatts_if=gatts_if;
					s_instance.ble_cmd_s.conn_id=param->write.conn_id;
					esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,ESP_GATT_OK, &rsp);
				}
			}
			break;
	        case ESP_GATTS_EXEC_WRITE_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT");
	            break;
	        case ESP_GATTS_MTU_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT");
	            break;
	        case ESP_GATTS_CONF_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT");
	            break;
	        case ESP_GATTS_UNREG_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_UNREG_EVT");
	            break;
	        case ESP_GATTS_DELETE_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_DELETE_EVT");
	            break;
	        case ESP_GATTS_START_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_START_EVT");
	            break;
	        case ESP_GATTS_STOP_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_STOP_EVT");
	            break;
	        case ESP_GATTS_CONNECT_EVT:
	        	ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT");
	            /* start security connect with peer device when receive the connect event sent by the master */
	            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
	            s_instance.isConnected=true;
	            break;
	        case ESP_GATTS_DISCONNECT_EVT:
	            ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
	            //remove_all_bonded_devices();
	            /* start advertising again when missing the connect */
	            esp_ble_gap_start_advertising(&s_instance.bt_serv_adv_params);
	            s_instance.isConnected=false;
	            break;
	        case ESP_GATTS_OPEN_EVT:
	            break;
	        case ESP_GATTS_CANCEL_OPEN_EVT:
	            break;
	        case ESP_GATTS_CLOSE_EVT:
	            break;
	        case ESP_GATTS_LISTEN_EVT:
	            break;
	        case ESP_GATTS_CONGEST_EVT:
	            break;
	        case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
	            ESP_LOGI(TAG, "The number handle = %x",param->add_attr_tab.num_handle);
	            if (param->create.status == ESP_GATT_OK){
	                if(param->add_attr_tab.num_handle == BTS_IDX_NB) {
	                    memcpy(s_instance.svc_app_handle_table, param->add_attr_tab.handles,sizeof(s_instance.svc_app_handle_table));
	                    esp_ble_gatts_start_service(s_instance.svc_app_handle_table[BTS_IDX_SVC]);
	                }else{
	                    ESP_LOGE(TAG, "Create attribute table abnormally, num_handle (%d) doesn't equal to IIA_IDX_NB(%d)",
	                         param->add_attr_tab.num_handle, BTS_IDX_NB);
	                }
	            }else{
	                ESP_LOGE(TAG, " Create attribute table failed, error code = %x", param->create.status);
	            }
	        break;
	    }

	        default:
	           break;
	    }
}

void BLEManager::InitializeBLE(void){
	esp_err_t ret;
	bt_serv_profile_tab[BT_SERV_PROFILE_APP_IDX].gatts_cb=BLEManager::gatts_profile_event_handler;
	bt_serv_profile_tab[BT_SERV_PROFILE_APP_IDX].gatts_if=ESP_GATT_IF_NONE;       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
	// Service Declaration
	bt_serv_gatt_db[BTS_IDX_SVC]= {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(factory_service_uuid), (uint8_t *)&factory_service_uuid}};
	// Characteristic-1 Declaration
	bt_serv_gatt_db[BTS_IDX_C1_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
	      CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}};
	// Characteristic-1 Value
	bt_serv_gatt_db[BTS_IDX_C1_VAL] =  {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&char_1_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
	      sizeof(uint8_t), 0,NULL}};
	// Characteristic-1 Configuration Descriptor
	bt_serv_gatt_db[BTS_IDX_C1_CFG] =  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
		      sizeof(uint16_t), 0,NULL}};

	// Characteristic-2 Declaration
	bt_serv_gatt_db[BTS_IDX_C2_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
	      CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}};
	// Characteristic-2 Value
	bt_serv_gatt_db[BTS_IDX_C1_VAL] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&char_2_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
	      sizeof(uint8_t), 0,NULL}};
	ReadAndConfigureServiceUUID();
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(TAG, "%s init controller failed: %s", __func__, esp_err_to_name(ret));
		return;
	}
	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
		return;
	}

	ESP_LOGI(TAG, "%s init bluetooth", __func__);
	ret = esp_bluedroid_init();
	if (ret) {
		ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
		return;
	}
	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
		return;
	}

	ret = esp_ble_gatts_register_callback(gatts_event_handler);
	if (ret){
		ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
		return;
	}
	ret = esp_ble_gap_register_callback(gap_event_handler);
	if (ret){
		ESP_LOGE(TAG, "gap register error, error code = %x", ret);
		return;
	}
	ret = esp_ble_gatts_app_register(BT_SERV_RATE_APP_ID);
	if (ret){
		ESP_LOGE(TAG, "gatts app register error, error code = %x", ret);
		return;
	}
	ret = esp_ble_gatt_set_local_mtu(517);
	if (ret)
	{
		ESP_LOGE(TAG,"set local  MTU failed, error code = %x", ret);
	}
	/* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
	esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
	uint8_t key_size = 16;      //the key size should be 7~16 bytes
	uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
	uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
	//set static passkey
	uint32_t passkey = 123456;
	uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
	uint8_t oob_support = ESP_BLE_OOB_DISABLE;
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
	/* If your BLE device acts as a Slave, the init_key means you hope which types of key of the master should distribute to you,
	and the response key means which key you can distribute to the master;
	If your BLE device acts as a master, the response key means you hope which types of key of the slave should distribute to you,
	and the init key means which key you can distribute to the slave. */
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
	/* Just show how to clear all the bonded devices
	 * Delay 30s, clear all the bonded devices
	 *
	 * vTaskDelay(30000 / portTICK_PERIOD_MS);
	 * remove_all_bonded_devices();
	 */

}
void BLEManager::show_bonded_devices(){
	int dev_num = esp_ble_get_bond_device_num();

	esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
	esp_ble_get_bond_device_list(&dev_num, dev_list);
	ESP_LOGI(TAG, "Bonded devices number : %d\n", dev_num);

	ESP_LOGI(TAG, "Bonded devices list : %d\n", dev_num);
	for (int i = 0; i < dev_num; i++) {
		esp_log_buffer_hex(TAG, (void *)dev_list[i].bd_addr, sizeof(esp_bd_addr_t));
	}

	free(dev_list);
}

void BLEManager::remove_all_bonded_devices() {
	int dev_num = esp_ble_get_bond_device_num();

	esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
	esp_ble_get_bond_device_list(&dev_num, dev_list);
	for (int i = 0; i < dev_num; i++) {
		esp_ble_remove_bond_device(dev_list[i].bd_addr);
	}

	free(dev_list);
}

uint16_t BLEManager::getAttributeIndexByAppTable(uint16_t attributeHandle){
	uint16_t attrIndex = BTS_IDX_NB;
	uint16_t i;
	for(i=0;i<BTS_IDX_NB;++i){
		if(svc_app_handle_table[i]==attributeHandle){
			attrIndex=i;
			break;
		}
	}
	return attrIndex;
}

void BLEManager::getDeviceID(char *deviceid,size_t max){
	esp_err_t ret;
	uint8_t eth_mac[6];
	if((ret=esp_read_mac(eth_mac,ESP_MAC_BT))==ESP_OK){
		snprintf(deviceid, max, "%s %02X%02X%02X",
				DEVICE_NAME, eth_mac[3], eth_mac[4], eth_mac[5]);
		ESP_LOGI(TAG, "DeviceID:%s",deviceid);
	}else{
		ESP_LOGE(TAG, "Error:%s while retrieving BLE MAC address",esp_err_to_name(ret));
		snprintf(deviceid, max, "%s",DEVICE_NAME);
	}
}

void BLEManager::ReadAndConfigureServiceUUID(void){
	bt_serv_adv_config.p_service_uuid=factory_service_uuid;
	bt_serv_gatt_db[BTS_IDX_SVC].att_desc.value=(uint8_t *)&factory_service_uuid;
}

void BLEManager::esp_key_type_to_str(esp_ble_key_type_t key_type,char* name){
	   switch(key_type) {
	    case ESP_LE_KEY_NONE:
	       strcpy(name,(const char*)"ESP_LE_KEY_NONE");
	        break;
	    case ESP_LE_KEY_PENC:
	    	strcpy(name,(const char*)"ESP_LE_KEY_PENC");
	        break;
	    case ESP_LE_KEY_PID:
	    	strcpy(name,(const char*)"ESP_LE_KEY_PID");
	        break;
	    case ESP_LE_KEY_PCSRK:
	    	strcpy(name,(const char*)"ESP_LE_KEY_PCSRK");
	        break;
	    case ESP_LE_KEY_PLK:
	    	strcpy(name,(const char*)"ESP_LE_KEY_PLK");
	        break;
	    case ESP_LE_KEY_LLK:
	    	strcpy(name,(const char*)"ESP_LE_KEY_LLK");
	        break;
	    case ESP_LE_KEY_LENC:
	    	strcpy(name,(const char*)"ESP_LE_KEY_LENC");
	        break;
	    case ESP_LE_KEY_LID:
	    	strcpy(name,(const char*)"ESP_LE_KEY_LID");
	        break;
	    case ESP_LE_KEY_LCSRK:
	    	strcpy(name,(const char*)"ESP_LE_KEY_LCSRK");
	        break;
	    default:
	    	strcpy(name,(const char*)"INVALID BLE KEY TYPE");
	        break;

	   }
}

void BLEManager::esp_auth_req_to_str(esp_ble_auth_req_t auth_req,char* name){
	switch(auth_req) {
	    case ESP_LE_AUTH_NO_BOND:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_NO_BOND");
	        break;
	    case ESP_LE_AUTH_BOND:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_BOND");
	        break;
	    case ESP_LE_AUTH_REQ_MITM:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_REQ_MITM");
	        break;
	    case ESP_LE_AUTH_REQ_BOND_MITM:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_REQ_BOND_MITM");
	        break;
	    case ESP_LE_AUTH_REQ_SC_ONLY:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_REQ_SC_ONLY");
	        break;
	    case ESP_LE_AUTH_REQ_SC_BOND:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_REQ_SC_BOND");
	        break;
	    case ESP_LE_AUTH_REQ_SC_MITM:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_REQ_SC_MITM");
	        break;
	    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
	    	strcpy(name,(const char*)"ESP_LE_AUTH_REQ_SC_MITM_BOND");
	        break;
	    default:
	    	strcpy(name,(const char*)"INVALID BLE AUTH REQ");
	        break;
	   }
}

void BLEManager::sendNotication(uint8_t *data,uint16_t length){
	if((isConnected)&&(ble_cmd_s.gatts_if>0)){
		esp_ble_gatts_send_indicate(ble_cmd_s.gatts_if,ble_cmd_s.conn_id, svc_app_handle_table[BTS_IDX_C1_VAL],length, data, false);
	}
}

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "includes.h"


extern "C"{
	void app_main();
}

void app_main(void)
{
	esp_err_t ret;

	/* Initialize NVS. */
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );
	BLEManager *ble;
	ble=BLEManager::getInstance();
	ble->InitializeBLE();
	int counter=0;
    while (true) {
    	char data[200];
    	sprintf((char*)data,(const char*)"count:%d value",counter);
    	ble->sendNotication((uint8_t *)data,(uint16_t)strlen(data));
    	counter++;
        printf("%s\n",data);
        sleep(1);
    }
}

#define TAG "MAIN"

#include <string.h>

#include "device/pt6314.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void
app_main()
{
	pt6314_init();
	ESP_LOGI(TAG, "SYSTEM READY");

	pt6314_print(0x00, "Hello world!");
	pt6314_print(0x40, "XiNGRZ");

	vTaskDelay(1000 / portTICK_PERIOD_MS);

	pt6314_clear();

	vTaskDelay(1000 / portTICK_PERIOD_MS);

	pt6314_print(0x00, "Driver by XiNGRZ");

	vTaskDelay(1000 / portTICK_PERIOD_MS);

	pt6314_print(0x40, "count: ");

	char buf[5] = {0};
	int i = 0;
	while (1) {
		sprintf(buf, "%04d", i);
		pt6314_print(0x40 + 7, buf);
		i = (i + 1) % 1000;
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

#define TAG "PT6314"

#include "pt6314.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pinout.h"

#define INST_CLEAR_DISPLAY (1 << 0)
#define INST_CURSOR_HOME (1 << 1)
#define INST_ENTRY_MODE_SET (1 << 2)
#define ISNT_DISPLAY_ON_OFF (1 << 3)
#define INST_CURSOR_OR_DISPLAY_SHIFT (1 << 4)
#define INST_FUNCTION_SET (1 << 5)
#define INST_CGRAM_ADDRESS_SET (1 << 6)
#define INST_DDRAM_ADDRESS_SET (1 << 7)

#define BIT_SYNC (0x1f << 3)
#define BIT_WRITE (0 << 2)
#define BIT_INST (0 << 1)
#define BIT_DATA (1 << 1)

#define CURSOR_ON 0
#define BLINKING_ON 0

#define BRIGHTNESS 3

static spi_device_handle_t spi;

static void IRAM_ATTR write_inst(uint8_t val);
static void IRAM_ATTR write_data(void *buf, uint32_t len);

void
pt6314_init(void)
{
	ESP_LOGI(TAG, "Init interfaces...");

	gpio_config_t output = {
		.pin_bit_mask = (1UL << PIN_VFD_STB),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
	};

	ESP_ERROR_CHECK(gpio_config(&output));

	spi_bus_config_t bus = {
		.mosi_io_num = PIN_VFD_SIO,
		.miso_io_num = -1,
		.sclk_io_num = PIN_VFD_SCK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 20 * 2 * 8,
		.flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_IOMUX_PINS,
	};

	ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));

	spi_device_interface_config_t dev = {
		.clock_speed_hz = 1000,
		.mode = 0,
		.spics_io_num = -1,
		.queue_size = 7,
	};

	ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev, &spi));

	write_inst(INST_FUNCTION_SET | (1 << 3) | (BRIGHTNESS & 0x03));
	write_inst(INST_CLEAR_DISPLAY);

	write_inst(ISNT_DISPLAY_ON_OFF | (1 << 2) | (CURSOR_ON << 1) | (BLINKING_ON << 0));
	write_inst(INST_FUNCTION_SET | (1 << 3) | (BRIGHTNESS & 0x03));
	write_inst(INST_CLEAR_DISPLAY);
}

void
pt6314_clear(void)
{
	write_inst(INST_CURSOR_HOME);
	write_inst(INST_CLEAR_DISPLAY);
}

void
pt6314_print(uint8_t offset, char *str)
{
	write_inst(INST_CURSOR_HOME);
	write_inst(INST_DDRAM_ADDRESS_SET | offset);
	write_data(str, strlen(str));
}

static void IRAM_ATTR
hspi_write(void *buf, uint32_t len)
{
	spi_transaction_t trans = {
		.tx_buffer = buf,
		.length = len * 8,
	};

	ESP_ERROR_CHECK(gpio_set_level(PIN_VFD_STB, 0));
	ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &trans));
	ESP_ERROR_CHECK(gpio_set_level(PIN_VFD_STB, 1));
}

static void IRAM_ATTR
write_inst(uint8_t val)
{
	uint8_t tmp[2] = {BIT_SYNC | BIT_WRITE | BIT_INST, val};
	hspi_write(tmp, sizeof(tmp));
}

static void IRAM_ATTR
write_data(void *buf, uint32_t len)
{
	uint8_t tmp[2] = {BIT_SYNC | BIT_WRITE | BIT_DATA, 0};
	for (uint32_t i = 0; i < len; i++) {
		tmp[1] = ((uint8_t *)buf)[i];
		hspi_write(tmp, sizeof(tmp));
		hspi_write(tmp, sizeof(tmp));  // I don't know why, but it works
	}
}

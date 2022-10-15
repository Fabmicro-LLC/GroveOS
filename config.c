/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "config.h"
#include "utils.h"
#include "crc16.h"
#include "adc.h"

int baud_rates[BAUD_RATES] = { 9600, 19200, 38400, 57600, 115200, 250000 };
char *stop_bits[STOP_BITS] = { "1", "0.5", "2", "1.5" };

int flash_write(unsigned int sector_number, unsigned int offset,  unsigned int *src, unsigned int size);

const static unsigned int FLASH_SECTOR2ADDR[12] = {0x08000000, 0x08004000, 0x08008000, 0x0800C000, 0x08010000, 0x08020000, 0x08040000, 0x08060000, 0x08080000, 0x080A0000, 0x080C0000, 0x080E0000};
const static unsigned int FLASH_SECTOR2NUMBER[12] = {FLASH_Sector_0, FLASH_Sector_1, FLASH_Sector_2, FLASH_Sector_3, FLASH_Sector_4, FLASH_Sector_5, FLASH_Sector_6, FLASH_Sector_7, FLASH_Sector_8, FLASH_Sector_9, FLASH_Sector_10, FLASH_Sector_11};

#define ADC_DC_VOLTS	(12.0/4096.0)
#define	ADC_TEMPERATURE_SENSOR_SLOPE	(SENSORS_ADC_VREF / SENSORS_ADC_MAX / 0.0025) 
#define	ADC_TEMPERATURE_SENSOR_OFFSET	((0.76 / 0.0025 - 25) / (SENSORS_ADC_VREF / 4096 / 0.0025))

const CONFIG config_default = {
	.lcd = {
		.KX1=0.499662, .KX2=0.001713, .KX3=-18.589819, .KY1=0.005836, .KY2=-0.288795, .KY3=278.657990,
		.enabled = 1,
	},

	.adc = {
		.enabled = 0,
		.mode = ADC_MODE,
		.tim_period = 500,//2000 kilo-samples per second (measuring 50Hz mains requires at least 50*32=1600 samples/sec)
		.samples = 1000,// trigger ADC event two times a second
		.coeff = { 	ADC_DC_VOLTS, ADC_DC_VOLTS, ADC_DC_VOLTS, ADC_DC_VOLTS, ADC_DC_VOLTS,
				ADC_DC_VOLTS, ADC_DC_VOLTS, ADC_DC_VOLTS, ADC_DC_VOLTS, ADC_DC_VOLTS,
				ADC_TEMPERATURE_SENSOR_SLOPE, ADC_TEMPERATURE_SENSOR_SLOPE, ADC_TEMPERATURE_SENSOR_SLOPE},
		.offset = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, ADC_TEMPERATURE_SENSOR_OFFSET, ADC_TEMPERATURE_SENSOR_OFFSET, ADC_TEMPERATURE_SENSOR_OFFSET},
	},


	.spi = {
		.enabled = 0,
		.prescaler = 0x10, // SPI_BaudRatePrescaler_8
		.mode = 0, // 0 - Single, 1 - Continuous
		.flags = 0, // 0 - LSB/MSB
	},


	#ifdef DALI1_MODE
	.dali1 = {
		.mode = DALI1_MODE,
		.short_addr = DALI1_SHORT_ADDR,
		.txrx_pol = DALI1_TXRX_POL,
		.manchester_pol = DALI1_MANCHESTER_POL,
	},
	#endif

	#ifdef DALI2_MODE
	.dali1 = {
		.mode = DALI2_MODE,
		.short_addr = DALI2_SHORT_ADDR,
		.txrx_pol = DALI2_TXRX_POL,
		.manchester_pol = DALI2_MANCHESTER_POL,
	},
	#endif

	.gpios_enabled = 1,
	.logger_enabled = 0,
	.microsec_timer_enabled = 0,

	.default_application_file_name = "default.elf",
	.ext1_port_baud_rate = 0, // 9600 is default, please see baud_rates[] 
	.ext1_port_parity = 0,
	.ext1_port_stop_bits = 0,
	.ext1_port_modbus_mode = 1, // 0 - off, 1 - master, 2 - slave
	.ext1_port_modbus_addr = 254,
	.ext1_port_modbus_key = 0x01234567,
	.ext2_port_baud_rate = 0, // 9600 is default, please see baud_rates[] 
	.ext2_port_parity = 0,
	.ext2_port_stop_bits = 0,
	.ext2_port_modbus_mode = 0, // 0 - off, 1 - master, 2 - slave
	.ext2_port_modbus_addr = 254,
	.ext2_port_modbus_key = 0x01234567,
	.use_rtc = 0, // RTC is disabled by default

};

CONFIG *config_flash = (CONFIG *) 0x08004000; // address of FLASH_CONFIG section




CONF_ITEM config_map_spi[] = {
	{CONF_BYTE, "enabled", "Enable/Disable", &config_active.spi.enabled },
	{CONF_BYTE, "mode", "0 - send by request, 1 - send continuously", &config_active.spi.mode },
	{CONF_BYTE, "flags", "bit 0: MSB/LSB, bit 1:  CPOL_HIGH/CPOL_LOW, bit 3: CPHA_2Edge/CPHA_1Edge", &config_active.spi.flags },
	{CONF_BYTE, "prescaler", "SPI bus clock, 8: 1.3125 Mhz, 16: 2.625 MHz, 24: 5.25 MHz, 32: 10.5 MHz, 40: 21 MHz, 48: 42 MHz, 56: 84 MHz", &config_active.spi.prescaler },
	{CONF_END,  NULL, NULL, NULL },
};

CONF_ITEM config_map_lcd[] = {
	{CONF_BYTE, "enabled", "Enable/Disable", &config_active.lcd.enabled },
	{CONF_FLOAT, "KX1", "Float value", &config_active.lcd.KX1 },
	{CONF_FLOAT, "KX2", "Float value", &config_active.lcd.KX2 },
	{CONF_FLOAT, "KX3", "Float value", &config_active.lcd.KX3 },
	{CONF_FLOAT, "KY1", "Float value", &config_active.lcd.KY1 },
	{CONF_FLOAT, "KY2", "Float value", &config_active.lcd.KY2 },
	{CONF_FLOAT, "KY3", "Float value", &config_active.lcd.KY3 },
	{CONF_END,  NULL, NULL, NULL },
};

CONF_ITEM config_map_adc_offsets[] = {
	{CONF_FLOAT, "adc0", "Float value", &config_active.adc.offset[0] },
	{CONF_FLOAT, "adc1", "Float value", &config_active.adc.offset[1] },
	{CONF_FLOAT, "adc2", "Float value", &config_active.adc.offset[2] },
	{CONF_FLOAT, "adc3", "Float value", &config_active.adc.offset[3] },
	{CONF_FLOAT, "adc4", "Float value", &config_active.adc.offset[4] },
	{CONF_FLOAT, "adc5", "Float value", &config_active.adc.offset[5] },
	{CONF_FLOAT, "adc6", "Float value", &config_active.adc.offset[6] },
	{CONF_FLOAT, "adc7", "Float value", &config_active.adc.offset[7] },
	{CONF_FLOAT, "adc8", "Float value", &config_active.adc.offset[8] },
	{CONF_FLOAT, "adc9", "Float value", &config_active.adc.offset[9] },
	{CONF_FLOAT, "adc10", "Float value", &config_active.adc.offset[10] },
	{CONF_FLOAT, "adc11", "Float value", &config_active.adc.offset[11] },
	{CONF_FLOAT, "adc12", "Float value", &config_active.adc.offset[12] },
	{CONF_FLOAT, "adc13", "Float value", &config_active.adc.offset[13] },
	{CONF_FLOAT, "adc14", "Float value", &config_active.adc.offset[14] },
	{CONF_FLOAT, "adc15", "Float value", &config_active.adc.offset[15] },
	{CONF_END,  NULL, NULL, NULL },
};

CONF_ITEM config_map_adc_coeffs[] = {
	{CONF_FLOAT, "adc0", "Float value", &config_active.adc.coeff[0] },
	{CONF_FLOAT, "adc1", "Float value", &config_active.adc.coeff[1] },
	{CONF_FLOAT, "adc2", "Float value", &config_active.adc.coeff[2] },
	{CONF_FLOAT, "adc3", "Float value", &config_active.adc.coeff[3] },
	{CONF_FLOAT, "adc4", "Float value", &config_active.adc.coeff[4] },
	{CONF_FLOAT, "adc5", "Float value", &config_active.adc.coeff[5] },
	{CONF_FLOAT, "adc6", "Float value", &config_active.adc.coeff[6] },
	{CONF_FLOAT, "adc7", "Float value", &config_active.adc.coeff[7] },
	{CONF_FLOAT, "adc8", "Float value", &config_active.adc.coeff[8] },
	{CONF_FLOAT, "adc9", "Float value", &config_active.adc.coeff[9] },
	{CONF_FLOAT, "adc10", "Float value", &config_active.adc.coeff[10] },
	{CONF_FLOAT, "adc11", "Float value", &config_active.adc.coeff[11] },
	{CONF_FLOAT, "adc12", "Float value", &config_active.adc.coeff[12] },
	{CONF_FLOAT, "adc13", "Float value", &config_active.adc.coeff[13] },
	{CONF_FLOAT, "adc14", "Float value", &config_active.adc.coeff[14] },
	{CONF_FLOAT, "adc15", "Float value", &config_active.adc.coeff[15] },
	{CONF_END,  NULL, NULL, NULL },
};

CONF_ITEM config_map_adc[] = {
	{CONF_BYTE, "enabled", "Enable/Disable", &config_active.adc.enabled },
	{CONF_BYTE, "mode", "0 - scan continuously, 1 - scan by request, 2 - scan between LCD updates", &config_active.adc.mode },
	{CONF_DWORD, "period", "Period in usecs between scans", &config_active.adc.tim_period },
	{CONF_DWORD, "samples", "Num of samples to average", &config_active.adc.samples },
	{CONF_SUBITEM, "offsets", "Calibration offsets", &config_map_adc_offsets },
	{CONF_SUBITEM, "coeffs", "Calibration coefficients", &config_map_adc_coeffs },
	{CONF_END,  NULL, NULL, NULL },
};

// Root config map item
CONF_ITEM config_map[] = { 
	{CONF_SUBITEM, "adc", "ADC configuration", &config_map_adc },
	{CONF_SUBITEM, "lcd", "LCD configuration", &config_map_lcd },
	{CONF_SUBITEM, "spi", "SPI configuration", &config_map_spi },
	{CONF_BYTE, "gpios_enabled", "Enable/Disable external GPIOs", &config_active.gpios_enabled },
	{CONF_BYTE, "logger_enabled", "Enable/Disable event logger", &config_active.logger_enabled },
	{CONF_BYTE, "microsec_timer_enabled", "Enable/Disable micro-second timer", &config_active.microsec_timer_enabled },
	{255, "default_application", "Default application file name", &config_active.default_application_file_name},
	{CONF_BYTE, "ext1_port_baud_rate", "EXT1 baud rate: 0 - 9600, 1 - 19200, 2 - 38400, 3 - 57600, 4 - 115200, 5 - 250000", &config_active.ext1_port_baud_rate },
	{CONF_BYTE, "ext1_port_parity", "EXT1 port parity: 0 - N, 1 - E, 2 - O", &config_active.ext1_port_parity },
	{CONF_BYTE, "ext1_port_stop_bits", "EXT1 port stop bits:  0 - 1 bit, 1 - 1.5bits, 2 - 2 bits", &config_active.ext1_port_stop_bits},
	{CONF_BYTE, "ext1_port_modbus_mode", "EXT1 port Modbus mode:  0 - off, 1 - master, 2 - slave", &config_active.ext1_port_modbus_mode},
	{CONF_BYTE, "ext1_port_modbus_addr", "EXT1 port Modbus address", &config_active.ext1_port_modbus_addr},
	{CONF_DWORD, "ext1_port_modbus_key", "EXT1 port Modbus auth key", &config_active.ext1_port_modbus_key},
	{CONF_BYTE, "ext2_port_baud_rate", "EXT2 baud rate: 0 - 9600, 1 - 19200, 2 - 38400, 3 - 57600, 4 - 115200, 5 - 250000", &config_active.ext2_port_baud_rate },
	{CONF_BYTE, "ext2_port_parity", "EXT2 port parity: 0 - N, 1 - E, 2 - O", &config_active.ext2_port_parity },
	{CONF_BYTE, "ext2_port_stop_bits", "EXT2 port stop bits:  0 - 1 bit, 1 - 1.5bits, 2 - 2 bits", &config_active.ext2_port_stop_bits},
	{CONF_BYTE, "ext2_port_modbus_mode", "EXT2 port Modbus mode:  0 - off, 1 - master, 2 - slave", &config_active.ext2_port_modbus_mode},
	{CONF_BYTE, "ext2_port_modbus_addr", "EXT2 port Modbus address", &config_active.ext2_port_modbus_addr},
	{CONF_DWORD, "ext2_port_modbus_key", "EXT2 port Modbus auth key", &config_active.ext2_port_modbus_key},
	{CONF_BYTE, "use_rtc", "Enable/Disable RTC", &config_active.use_rtc},
	{CONF_END,  NULL, NULL, NULL },
};







int verify_config() {
	unsigned char* p1=(unsigned char*) &config_active;
	unsigned char* p2=(unsigned char*) config_flash;
	
	for(int i=0; i<sizeof(CONFIG); i++) {
		if(*p1++ != *p2++) {
			print("verify_config: failed\r\n");
			return -1;
		}
	}

	return 0;
}

int load_config() {
	if(sizeof(struct __CONFIG) > 16048) {
                print("load_config: failed, sizeof(struct __CONFIG) too big, should be <= 16048 bytes\r\n");
                return -1;
        }

        print("load_config: loading config size = %d...\r\n", sizeof(struct __CONFIG));
        config_active = *config_flash;
        unsigned short crc16=crc16_ccitt((unsigned char*)&config_active, sizeof(CONFIG)-2);

        if(crc16 != config_active.crc16) {
                print("load_config: config error: CRC16 mismatch! calculated crc16= 0x%04X, saved crc16 = 0x%04X\r\n", crc16, config_active.crc16);
                print("load_config: default config loaded!\r\n");
                config_active = config_default;
		return -1;
        }
        print("load_config: config read ok: crc16=%04X\r\n", crc16);
	return 0;
}

int save_config() {
	if(sizeof(struct __CONFIG) > 16048) {
                print("save_config: failed, sizeof(struct __CONFIG) too big, should be <= 16048 bytes\r\n");
                return -1;
        }
        config_active.crc16 = crc16_ccitt((unsigned char*)&config_active, sizeof(CONFIG)-2);
        print("save_config: saving config_active, crc16 = 0x%04X\r\n", config_active.crc16);

	int rc=0;

	for(int i=0; i<3; i++) {
        	rc=flash_write(CONFIG_SECTOR_NUMBER, 0, (unsigned int*) &config_active, sizeof(config_active));
		print("save_config: flash_write rc=%d, i=%d\r\n", rc, i);
		if(rc==0) {
			rc=verify_config();	
			print("save_config: verify_config rc=%d\r\n", rc);
			if(rc == 0) break;
		}
	}
		
	if(rc) {
		print("save_config: failed\r\n");
		return rc;
	}

	//print("save_config: succeded\r\n");
        return 0;
}


int flash_write(unsigned int sector_number, unsigned int offset,  unsigned int *src, unsigned int size) {
        int rc;
        int i;

        if(sector_number>=SIZE(FLASH_SECTOR2ADDR)) {
                print("flash_write: sector number %d is too big!\r\n");
                return -999;
        }

	rc=FLASH_GetStatus();
        if(rc!=FLASH_COMPLETE) {
                print("flash_write() flash not ready, status=%d\r\n", rc);
                return -333;
        }

        unsigned int *dst = (unsigned int *)FLASH_SECTOR2ADDR[sector_number]+offset;

        //print("flash_write: unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        //print("flash_write: erasing flash, sector_number=%d\r\n", sector_number);
        rc = FLASH_EraseSector(FLASH_SECTOR2NUMBER[sector_number], VoltageRange_3);

        if(rc!=FLASH_COMPLETE) {
                print("flash_write: FLASH_EraseSector error, rc=%d\r\n", rc);
                FLASH_Lock();
                return rc;
        }

        print("flash_write: writing flash, src=%p, dst=%p, size=%d bytes\r\n", src, dst, size);

        int num_of_32_blocks = size / 4 + (size % 4 ? 1 : 0);

        for(i=0; i < num_of_32_blocks; i++) {
                rc = FLASH_ProgramWord((unsigned int) dst++, *src++);
                if(rc!=FLASH_COMPLETE) {
                        print("flash_write: FLASH_ProgramWord error, rc=%d\r\n", rc);
                        FLASH_Lock();
                        return rc;
                }
        }

        //print("flash_write: writing complete!\r\n");

        FLASH_Lock();//lock flash at the end
        //print("flash_write: locking flash\r\n");
        return 0;
}


int config_map_find(char* conf_str, int* item_type, void** item_data)
{
	//print("config_map_find: %s\r\n", conf_str);

	if(conf_str == NULL) {
		//print("config_map_find: conf_str is NULL, no config path provided!\r\n");
		return -1;
	}


	char str[128];
	strncpy(str, conf_str, 128); 

	int str_len = strnlen(str, 128);

	int depth = 1;
        char *c = str;

        while( (c = strpbrkn(c, ".", str + str_len)) ) { *c = 0; c++; depth++; }

	CONF_ITEM *conf_item = config_map;

	//print("config_map_find: depth = %d\r\n", depth);

	for(int i = 0; i < depth; i++) {
		char *str_item = strnarg(str, i, str_len);

		//print("config_map_find: i = %d, str_item = %s\r\n", i, str_item);

		if(str_item == NULL) {
			return -2; // End of tree reached, not found
		}
		

		// Search for item
		while(conf_item) {
			//print("config_map_find: conf_item = %p\r\n", conf_item);
			if(conf_item->item_type == 0) 
				return -3; // End of list, not found

			//print("config_map_find: trncasecmp(%s, %s)\r\n", conf_item->item_name, str_item);
        		if(strncasecmp(conf_item->item_name, str_item, 128) == 0)
				break;

			conf_item++;
		}

		*item_type = conf_item->item_type;
		*item_data = conf_item->data;

		if(conf_item->item_type == CONF_SUBITEM) 
			conf_item = (CONF_ITEM*)(conf_item->data);

	}

	//print("config_map_find: found, item_type = %d, item_data = %p\r\n", *item_type, *item_data);

	return 0;
} 

int config_map_enum(CONF_ITEM* conf_item, char *prefix)
{
	if(conf_item == NULL)
		return -1;

	while(conf_item->item_type != 0) {

		if(conf_item->item_type == 1 || conf_item->item_type == 2 || conf_item->item_type == 4 || conf_item->item_type == 5) {
			if(prefix[0] != 0)
				_print("SET %s.%s ", prefix, conf_item->item_name);
			else
				_print("SET %s ", conf_item->item_name);
		}

		switch(conf_item->item_type) {
			case 1: _print("%u\r\n", *(uint8_t*)conf_item->data);
				break;
			case 2: _print("%u\r\n", *(uint16_t*)conf_item->data);
				break;
			case 4: _print("%u\r\n", *(uint32_t*)conf_item->data);
				break;
			case 5: _print("%f\r\n", *(float*)conf_item->data);
				break;
			default:
				break;
		}


		if(conf_item->item_type == CONF_SUBITEM) {
			// Go deeper

			char old_prefix[256];
			
			strncpy(old_prefix, prefix, 256);

			if(prefix[0] != 0)
				strncat(prefix, ".", 256);

			strncat(prefix, conf_item->item_name, 256);

			config_map_enum((CONF_ITEM*) conf_item->data, prefix);

			strncpy(prefix, old_prefix, 256);
		}

		conf_item++; // next item in the array of same depth
	}

	return 0;
} 


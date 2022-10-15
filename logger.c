/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "logger.h"
#include "config.h"
#include "utils.h"
#include "crc16.h"
#include "msg.h"

extern int event_logging;

int flash_write(unsigned int sector_number, unsigned int offset,  unsigned int *src, unsigned int size);


const unsigned int FLASH_SECTOR2ADDR[25] = {	0x08000000, 0x08004000, 0x08008000, 0x0800C000, 0x08010000, 0x08020000, 0x08040000, 0x08060000, 
						0x08080000, 0x080A0000, 0x080C0000, 0x080E0000, 0x08100000, 0x08104000, 0x08108000, 0x0810C000, 
						0x08110000, 0x08120000, 0x08140000, 0x08160000, 0x08180000, 0x081A0000, 0x081C0000, 0x081E0000,
						0x081E0000 + 128*1024 };
const unsigned int FLASH_SECTOR2NUMBER[25] = {FLASH_Sector_0, FLASH_Sector_1, FLASH_Sector_2, FLASH_Sector_3, FLASH_Sector_4, FLASH_Sector_5, FLASH_Sector_6, FLASH_Sector_7, FLASH_Sector_8, FLASH_Sector_9, FLASH_Sector_10, FLASH_Sector_11, FLASH_Sector_12, FLASH_Sector_13, FLASH_Sector_14, FLASH_Sector_15, FLASH_Sector_16, FLASH_Sector_17, FLASH_Sector_18, FLASH_Sector_19, FLASH_Sector_20, FLASH_Sector_21, FLASH_Sector_22, FLASH_Sector_23, NULL};


//char __attribute__((section (".flash_logger"))) data = 0;

struct LOGGER_ITEM *current_logger_item = (struct LOGGER_ITEM *) 0;
static struct LOGGER_ITEM temp_item;

unsigned int current_logger_sector = LOGGER_START_SECTOR;
unsigned int current_logger_serial = 1;

int flash_erase(unsigned int sector_number);
int flash_erase_async(unsigned int sector_number);
int flash_write_item(struct LOGGER_ITEM* item);


void logger_init(void)
{

	NVIC_InitTypeDef   NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = FLASH_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	current_logger_item = (struct LOGGER_ITEM *) FLASH_SECTOR2ADDR[LOGGER_START_SECTOR];

	// search for record with the top most serial number

	struct LOGGER_ITEM *item;

	print("logger_init() looking for a record with topmost serial number...\r\n");

	for(item =  (struct LOGGER_ITEM*)FLASH_SECTOR2ADDR[LOGGER_START_SECTOR]; item < (struct LOGGER_ITEM*)(FLASH_SECTOR2ADDR[LOGGER_END_SECTOR] + 128 * 1024); item++) {
		//print("logger_init() item = %p\r\n", item);
		if(item->crc16 == crc16_ccitt((const unsigned char*)item, sizeof(*item) - 2)) { // check CRC16 first
			if(item->serial >= current_logger_serial) {
				current_logger_item = item;
				current_logger_serial = item->serial;
			}			
		}
	}


	// Shift to next record
	current_logger_item++;
	current_logger_serial++;

	// Check for flash memory overlapping
	if(current_logger_item >= (struct LOGGER_ITEM*)(FLASH_SECTOR2ADDR[LOGGER_END_SECTOR] + 128 * 1024))
		current_logger_item = (struct LOGGER_ITEM*)FLASH_SECTOR2ADDR[LOGGER_START_SECTOR];

	current_logger_sector = logger_get_sector(current_logger_item);

	print("logger_init() current_logger_serial = %p, current_logger_item = %p, current_logger_sector = %d\r\n", current_logger_serial, current_logger_item, current_logger_sector);

	// Erase sector if current_logger_item points to the very beginning the sector

	for(int i = 0; i < 24; i++) {
		if((unsigned int)current_logger_item == FLASH_SECTOR2ADDR[i]) {
			print("logger_init() Erasing sector %d because current_logger_item points to very beginning of the sector!\r\n", i);
			flash_erase(i);
			break;
		} 
	}

}


int logger_erase_current_sector(void) 
{
	if(!config_active.logger_enabled)
		return -1;

	return flash_erase_async(current_logger_sector);
}


int logger_get_sector(void* addr)
{
	if(!config_active.logger_enabled)
		return -1;

	 unsigned int a = ( unsigned int) addr;

	for(int i = 0; i < 23; i++) {
		if(a >= FLASH_SECTOR2ADDR[i] && a < FLASH_SECTOR2ADDR[i+1]) {
			return i;
		}
	}

	if(a >= FLASH_SECTOR2ADDR[23] &&  a < FLASH_SECTOR2ADDR[23]+128*1024)
		return 23;

	return -2;
}


struct LOGGER_ITEM* logger_get_previous_item(struct LOGGER_ITEM* item)
{
	if(!config_active.logger_enabled)
		return NULL;

	if(item >= (struct LOGGER_ITEM*) (FLASH_SECTOR2ADDR[LOGGER_END_SECTOR] + 128*1024))
		item = (struct LOGGER_ITEM*) FLASH_SECTOR2ADDR[LOGGER_START_SECTOR];

	while(1) {
		item--;

		if(item <= (struct LOGGER_ITEM*) FLASH_SECTOR2ADDR[LOGGER_START_SECTOR]) {
			item = (struct LOGGER_ITEM*) (FLASH_SECTOR2ADDR[LOGGER_END_SECTOR+1]);
			item--;
			// last record in last sector
		} 


		if(item == current_logger_item) 
			return NULL; // no more records found

		if(item->crc16 != crc16_ccitt((const unsigned char*)item, sizeof(*item) - 2))
			continue; // bogus record, continue searching

		break;
	}

	return item;
}


int logger_write_data(uint8_t *data, int num)
{
	int i, rc, ret = 0;

	if(!config_active.logger_enabled)
		return -1;

	if(data == NULL) {
		print("logger_write_data() null pointer provided: data = %p\r\n", data);
		return -2;
	}

	if(num > LOGGER_MAX_BYTES) 
		num = LOGGER_MAX_BYTES;


	if(event_logging)
		print("logger_write_data() Storing logger record, current_logger_serial = %p, current_logger_item = %p\r\n", current_logger_serial, current_logger_item);

	// Copy sensor data to logger item structure

	for(i = 0; i < num; i++) 
		temp_item.data[i] = data[i];

	
        RTC_GetDate(RTC_Format_BIN, &temp_item.rtc_date);
        RTC_GetTime(RTC_Format_BIN, &temp_item.rtc_time);

	temp_item.serial = current_logger_serial;
	temp_item.crc16 = crc16_ccitt((const unsigned char*)&temp_item, sizeof(temp_item) - 2);


	// Check that current memory record is empty 

	unsigned int* p = (unsigned int*)current_logger_item;

	for(i = 0; i < sizeof(struct LOGGER_ITEM) / 4; i++) {
		if(*p != 0xffffffff) {
			// Memory is not empty
			print("logger_write_data() Record is not empty, writing cancelled!\r\n");
			current_logger_item++;
			ret = 888;
			goto logger_write_aps_data_end;
		}
	}

	// Everything is alright, can write

	rc = flash_write_item(&temp_item);

	if(rc) {
		print("logger_write_data() write error, rc = %d\r\n", rc);
	}

	current_logger_item++;
	current_logger_serial++;

	if((unsigned int)current_logger_item >= FLASH_SECTOR2ADDR[LOGGER_END_SECTOR] + 128*1024) {
		print("logger_write_data() flash memory is completed, overlapping!\r\n");
		current_logger_item = (struct LOGGER_ITEM*) FLASH_SECTOR2ADDR[LOGGER_START_SECTOR];
	}

	int new_logger_sector = logger_get_sector(current_logger_item);


	if(new_logger_sector == -1) {
		print("logger_write_data() Fatal error, current_logger_item = %p is outside of flash memory scope!\r\n", current_logger_item);
		ret = -3;
		goto logger_write_aps_data_end;
	}

	if(new_logger_sector != current_logger_sector) {
		print("logger_write_data() End of sector reached, old sector = %d, new sector = %d, current_logger_item = %p, current_logger_serial = %p\r\n", current_logger_sector, new_logger_sector, current_logger_item, current_logger_serial);
		current_logger_sector = new_logger_sector;
		ret = 999; // signal upper leverl tha flash sector needs to be erased
		goto logger_write_aps_data_end;
	}

	logger_write_aps_data_end:

	//print("logger_write_data() END\r\n");

	return ret;
}


int flash_erase_async(unsigned int sector_number)
{
	int rc;
	uint32_t tmp_psize = 0x0;
	uint32_t VoltageRange = VoltageRange_3;

	print("flash_erase_async\r\n");

        if(sector_number>=SIZE(FLASH_SECTOR2ADDR)) {
                print("flash_erase_async() sector number %d is too big!\r\n");
                return -1;
        }

	rc=FLASH_GetStatus();
        if(rc!=FLASH_COMPLETE) {
                print("flash_erase_async() flash not ready, status=%d\r\n", rc);
                return -333;
        }

        print("flash_erase_async() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        print("flash_erase_async() Erasing flash, sector_number=%d\r\n", sector_number);


	switch(VoltageRange) {
		case VoltageRange_1:	
					tmp_psize = FLASH_PSIZE_BYTE;
					break;
		case VoltageRange_2:	
					tmp_psize = FLASH_PSIZE_HALF_WORD;
					break;
		case VoltageRange_3:
					tmp_psize = FLASH_PSIZE_WORD;
					break;
		default:
					tmp_psize = FLASH_PSIZE_DOUBLE_WORD;
					break;
	}

  	/* Wait for last operation to be completed */
	rc= FLASH_WaitForLastOperation();

	if(rc == FLASH_COMPLETE) {
		/* if the previous operation is completed, proceed to erase the sector */

		#define SECTOR_MASK               ((uint32_t)0xFFFFFF07)

		FLASH->CR &= CR_PSIZE_MASK;
		FLASH->CR |= tmp_psize;
		FLASH->CR &= SECTOR_MASK;
		FLASH->CR |= FLASH_CR_SER | FLASH_SECTOR2NUMBER[sector_number];
		FLASH->CR |= FLASH_CR_STRT;
		FLASH->CR |= FLASH_CR_EOPIE;

		/* if the erase operation is completed, disable the SER Bit */
		//FLASH->CR &= (~FLASH_CR_SER);
		//FLASH->CR &= SECTOR_MASK;
	} else {
		print("flash_erase_async() FLASH_WaitForLastOperation() has failed!\r\n");
	}


	return rc;
}


int flash_erase(unsigned int sector_number)
{
	int rc;

        if(sector_number>=SIZE(FLASH_SECTOR2ADDR)) {
                print("flash_erase() sector number %d is too big!\r\n");
                return -1;
        }

	rc=FLASH_GetStatus();
        if(rc!=FLASH_COMPLETE) {
                print("flash_erase() flash not ready, status=%d\r\n", rc);
                return -333;
        }

	__disable_irq();

        print("flash_erase() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        print("flash_erase() Erasing flash, sector_number=%d\r\n", sector_number);
        rc = FLASH_EraseSector(FLASH_SECTOR2NUMBER[sector_number], VoltageRange_3);

        FLASH_Lock();//lock flash anyway


        if(rc!=FLASH_COMPLETE) {
                print("flash_erase() FLASH_EraseSector error, rc=%d\r\n", rc);
        }

	print("flash_erase() COMPLETE!\r\n");

	__enable_irq();

	return rc;
}

int flash_write_item(struct LOGGER_ITEM* item)
{
	int rc, i;

	unsigned int *src = (unsigned int *) item;
	unsigned int *dst = (unsigned int *) current_logger_item;
	int size = sizeof(struct LOGGER_ITEM);

	rc=FLASH_GetStatus();
	if(rc!=FLASH_COMPLETE) {
		print("flash_write_item() flash not ready, status=%d\r\n", rc);
		return -333;
	}

        //print("flash_write_item() Unlocking flash\r\n");
        FLASH_Unlock();

        //print("flash_write_item() Writing flash sector %d @ %p, src=%p, dst=%p, size=%d bytes\r\n", logger_get_sector(dst), current_logger_item, src, dst, size);

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        int num_of_32_blocks = size / 4 + (size % 4 ? 1 : 0);

        for(i=0; i < num_of_32_blocks; i++) {
                rc = FLASH_ProgramWord((unsigned int) dst++, *src++);
                if(rc!=FLASH_COMPLETE) {
                        print("flash_write_item() FLASH_ProgramWord error, rc=%d\r\n", rc);
                        FLASH_Lock();
                        return rc;
                }
        }

        //print("flash_write_item() Writing complete!\r\n");
	FLASH_Lock();


	return 0;
}


void FLASH_IRQHandler(void) {

        __disable_irq();


/*
    @arg FLASH_FLAG_EOP: FLASH End of Operation flag
  *            @arg FLASH_FLAG_OPERR: FLASH operation Error flag
  *            @arg FLASH_FLAG_WRPERR: FLASH Write protected error flag
  *            @arg FLASH_FLAG_PGAERR: FLASH Programming Alignment error flag
  *            @arg FLASH_FLAG_PGPERR: FLASH Programming Parallelism error flag
  *            @arg FLASH_FLAG_PGSERR: FLASH Programming Sequence error flag
  *            @arg FLASH_FLAG_BSY: FLASH Busy flag
*/


	int flags = FLASH_GetFlagStatus(FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	print("FLASH_IRQHandler() flags = %p\r\n", flags);

	if(flags & FLASH_FLAG_EOP) {

		print("FLASH_IRQHandler() Operation complete\r\n");

		if(flags & FLASH_FLAG_WRPERR || flags & FLASH_FLAG_PGAERR || flags & FLASH_FLAG_PGPERR || flags & FLASH_FLAG_PGSERR) {
			print("FLASH_IRQHandler() one of error flags set: FLASH_FLAG_WRPERR = %d, FLASH_FLAG_PGAERR = %d, FLASH_FLAG_PGPERR = %d, FLASH_FLAG_PGSERR = %d\r\n",
				flags & FLASH_FLAG_WRPERR, flags & FLASH_FLAG_PGAERR, flags & FLASH_FLAG_PGPERR, flags & FLASH_FLAG_PGSERR);

			PostMessageIRQ(FLASH_ERASE_ERROR, 1, 0, 0);

		} else {
			PostMessageIRQ(FLASH_ERASE_OK, 1, 0, 0);
		}

		FLASH->CR &= ~FLASH_CR_EOPIE; // Disable interrupts from Flash EOP

	        /* Clear All pending flags */
        	FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

		FLASH_Lock();
	}

        __enable_irq();
}



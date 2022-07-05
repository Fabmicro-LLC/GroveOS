#include "tfs.h"
#include "utils.h"
#include "crc16.h"


// Below are define in logger.c and depend on hardware
extern const unsigned int FLASH_SECTOR2ADDR[];
extern const unsigned int FLASH_SECTOR2NUMBER[];

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))


void tfs_flash_memcpy(char* dst, char* src, int len)
{
	for(int i = 0; i < len; i++) {
		FLASH_ProgramByte((int)dst++, *src++);
	}
}


// creates new file, return pointer to new TFS or NULL if no free space available
TFS_HEADER* tfs_create_file(char *name, int name_len, int file_size)
{
	// -1. check for existance and delete 
	// 0. find end of chain 
	// 1. find free space
	// 2. check space for consistency
	// 3. prepare TFS structure: copy name, date/time, set size and flags to 0xffff
	// 4. Update prev TFS header to point to new one
	// 5. write TFS header to flash
	
	TFS_HEADER* tfs_prev = NULL;
	TFS_HEADER* tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR]; 
	TFS_HEADER* end_of_tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]; 


	TFS_HEADER* tfs_tmp;
	if(tfs_tmp = tfs_find(name)) {
		print("tfs_create_file() File %s already exists @ %p, deleting...\r\n", name, tfs_tmp);	
		tfs_delete(tfs_tmp);
	}

	// Look for end of chain

	while((int)tfs < (int)end_of_tfs) {

		if(tfs->magic != 0x00534654) 
			break; // looks like we reached and of chain

		if(tfs->next == (TFS_HEADER*)0xffffffff) {
			tfs_prev = tfs;
			tfs = (TFS_HEADER*)((unsigned char*)tfs + sizeof(TFS_HEADER) + tfs->size); // point right next to the data block
			break; 
			
		}

		tfs = tfs->next;
	}

	// 32 bit allignment
	if((unsigned int)tfs & 0x00000003)
		tfs = (TFS_HEADER*)(((unsigned int)tfs & ~0x00000003) + 4); 

	print("tfs_create_file() end of chain at %p, prev tfs: %p\r\n", tfs, tfs_prev);

	// Check for consistancy and capacity

	unsigned int required_space = file_size + sizeof(TFS_HEADER);

	while((int)end_of_tfs - (int)tfs >= required_space) {
		int i;
		for(i = 0; i < required_space; i+=4) {
			if(*((unsigned int*)((unsigned char*)tfs +i)) != 0xffffffff) {
				// garbled space
				break;
			}
		}

		if(i >= required_space) { // Good space found
			break;
		} else {
			// garbled space found
			tfs = (TFS_HEADER*) ((unsigned int)tfs + 4); // shift 4 bytes
		}

		// and try again
	}
	 
	if((int)end_of_tfs - (int)tfs < required_space) {
		print("tfs_create_file() No space left on device, tfs: %p, enf_of_tfs: %p, file size + tfs size: %d\r\n", tfs, end_of_tfs, file_size + sizeof(TFS_HEADER));
		return NULL; 
	}

	print("tfs_create_file() Good space at: %p\r\n", tfs);

        //print("tfs_create_file() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);


	if(tfs_prev)
		FLASH_ProgramWord((int)&tfs_prev->next, (int)tfs); // tfs_prev->next = tfs; // update previous TFS to point to new one

	int rc = FLASH_ProgramWord((int)&(tfs->magic), TFS_MAGIC); //tfs->magic = TFS_MAGIC;

	FLASH_ProgramWord((int)&tfs->size, file_size); //tfs->size = file_size;
	FLASH_ProgramWord((int)&tfs->next, 0xffffffff); //tfs->next = 0xffffffff;
	FLASH_ProgramHalfWord((int)&tfs->flags, 0xffff); //tfs->flags = 0xffff;
	FLASH_ProgramHalfWord((int)&tfs->crc16, 0xffff); //tfs->crc16 = 0xffff;

	RTC_TimeTypeDef t;
	RTC_DateTypeDef d;

        RTC_GetTime(RTC_Format_BIN, &t);
        RTC_GetDate(RTC_Format_BIN, &d);

        FLASH_ProgramWord((int)&tfs->time, *(int*)(&t));
        FLASH_ProgramWord((int)&tfs->date, *(int*)(&d));

	// memcpy(tfs->name, name, MIN(name_len, sizeof(name)-1));
	tfs_flash_memcpy(tfs->name, name, MIN(name_len+1, 256));

       	//print("tfs_create_file() Locking flash\r\n");
      	FLASH_Lock();

	return tfs;
}

// writes a piece of data to this file, offset must be tracked by user.
// CAUTION: writing to same offset twice will get data corrupted!
int tfs_write_block(TFS_HEADER* tfs, char* block, int block_size, int offset)
{
	if((unsigned int)tfs < FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR] || (unsigned int)tfs >= FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]) {
		print("tfs_write_block() Bogus tfs: %p\r\n", tfs);
		return -1;
	}
 
	if(tfs->magic != TFS_MAGIC) {
		print("tfs_write_block() Provided tfs pointer %p is not a TFS!\r\n", tfs);
		return -2;
	}

	if(block_size + offset > tfs->size) {
		print("tfs_write_block() Block size is out of file extents, tfs: %p, size: %d, block_size: %d, offset: %d\r\n", tfs, tfs->size, block_size, offset);
		return -3;
	}

        //print("tfs_write_block() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	//memcpy((unsigned char*)tfs + sizeof(TFS_HEADER) + offset, block, block_size); 
	tfs_flash_memcpy((unsigned char*)tfs + sizeof(TFS_HEADER) + offset, block, block_size);

       	//print("tfs_write_block() Locking flash\r\n");
      	FLASH_Lock();

	if(crc16_ccitt(block, block_size) != crc16_ccitt((unsigned char*)tfs + sizeof(TFS_HEADER) + offset, block_size)) {
		print("tfs_write_block() Writtent data CRC mismatch, corrupted media?\r\n");
		return -4; 
	}

       	print("tfs_write_block() Written %d bytes\r\n", block_size);
	return block_size;
}


int tfs_read_block(TFS_HEADER* tfs, int offset, char* dst, int block_size)
{

	if((unsigned int)tfs < FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR] || (unsigned int)tfs >= FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]) {
		print("tfs_read_block() Bogus tfs: %p\r\n", tfs);
		return -1;
	}
 
	if(tfs->magic != TFS_MAGIC) {
		print("tfs_read_block() Provided tfs pointer %p is not a TFS!\r\n", tfs);
		return -2;
	}

	if(block_size + offset > tfs->size) {
		print("tfs_read_block() Block size is out of file extents, tfs: %p, size: %d, block_size: %d, offset: %d\r\n", tfs, tfs->size, block_size, offset);
		return -3;
	}

	memcpy(dst, (unsigned char*)tfs + sizeof(TFS_HEADER) + offset,  block_size);

	return block_size;
}

// calculates and sets CRC16 for this file, called after file is fully written. Returns CRC16.
int tfs_close(TFS_HEADER* tfs) 
{
	if((unsigned int)tfs < FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR] || (unsigned int)tfs >= FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]) {
		print("tfs_close() Bogus tfs: %p\r\n", tfs);
		return -1;
	}

	if(tfs->magic != TFS_MAGIC) {
		print("tfs_close() Provided tfs pointer %p is not a TFS!\r\n", tfs);
		return -2;
	}

        print("tfs_close() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	//tfs->crc16 = crc16_ccitt((char*)tfs + sizeof(TFS_HEADER), tfs->size);
	FLASH_ProgramHalfWord((int)&tfs->crc16, crc16_ccitt((char*)tfs + sizeof(TFS_HEADER), tfs->size));

       	print("tfs_close() Locking flash\r\n");
      	FLASH_Lock();

	return tfs->crc16;
}

// mark this file as deleted
int tfs_delete(TFS_HEADER* tfs)
{
	if((unsigned int)tfs < FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR] || (unsigned int)tfs >= FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]) {
		print("tfs_delete() Bogus tfs: %p\r\n", tfs);
		return -1;
	}

	if(tfs->magic != TFS_MAGIC) {
		print("tfs_delete() Provided tfs pointer %p is not a TFS!\r\n", tfs);
		return -2;
	}

        print("tfs_delete() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	//tfs->flags = tfs->flags & 0x7fff; // reset DEL flag indicating file is deleted
	FLASH_ProgramHalfWord((int)&tfs->flags, tfs->flags & 0x7fff);

       	print("tfs_delete() Locking flash\r\n");
      	FLASH_Lock();

	return 0;
}


// finds and returns TFS of an existing file by null terminated string.
TFS_HEADER* tfs_find(char *name)
{
	if(name) {
		TFS_HEADER* tfs_prev = NULL;
		TFS_HEADER* start_of_tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR]; 
		TFS_HEADER* end_of_tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]; 
		TFS_HEADER* tfs = start_of_tfs;

		while((uint32_t)tfs >= (uint32_t)start_of_tfs && (uint32_t)tfs+sizeof(TFS_HEADER) < (uint32_t)end_of_tfs) {
			if((tfs->flags & TFS_FLAGS_DEL) && strncmp(tfs->name, name, 256) == 0) {
				return tfs; // file found
			}

			tfs_prev = tfs;
			tfs = tfs->next;

			if((uint32_t)tfs == 0xffffffff)
				return NULL; // end of chain
			
		}

		print("tfs_find() File system is corrupted, tfs %p (next: %p) extends beyond file system boundary! \r\n", tfs_prev, tfs_prev->next);

		return NULL;
		
	} else {
		print("tfs_find() Bogus name: %p\r\n", name);
		return NULL;
	}
}

TFS_HEADER* tfs_get_begin(void)
{
	return (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR];
}

TFS_HEADER* tfs_get_end(void)
{
	return (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1];
}

unsigned int tfs_get_capacitance(void)
{
	return (unsigned int)tfs_get_end() - (unsigned int)tfs_get_begin(); 
}


// finds next file following provided TFS. To find first file use tfs=NULL
TFS_HEADER* tfs_find_next(TFS_HEADER* tfs)
{

	if(tfs == NULL) {
		tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR];

		if(tfs->magic != TFS_MAGIC) {
			print("tfs_find_next() Provided tfs pointer %p is not a TFS, file system is corrupted!\r\n", tfs);
			return NULL;
		}

		return tfs;
	}

	if((unsigned int)tfs < FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR] || (unsigned int)tfs >= FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]) {
		print("tfs_find_next() Bogus tfs: %p\r\n", tfs);
		return NULL;
	}




	if(tfs->magic != TFS_MAGIC) {
		print("tfs_find_next() Provided tfs pointer %p is not a TFS, file system is corrupted!\r\n", tfs);
		return NULL;
	}

	if((unsigned int)tfs->next == 0xffffffff)
		return NULL; // this is last TFS

	if(tfs->next->magic != TFS_MAGIC) {
		print("tfs_find_next() Corrupted file system, tfs %p (next: %p) points to something not a TFS!\r\n", tfs, tfs->next);
		return NULL;
	}

	return tfs->next;
}

// returns max size of file that is possible to write to file system
int tfs_get_free_space(void)
{
	TFS_HEADER* tfs_prev = NULL;
	TFS_HEADER* tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR]; 
	TFS_HEADER* end_of_tfs = (TFS_HEADER*) FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]; 


	// Look for end of chain

	while(tfs < end_of_tfs) {

		if(tfs->magic != 0x00534654) 
			break; // looks like we reached and of chain

		if(tfs->next == (TFS_HEADER*)0xffffffff) {
			tfs_prev = tfs;
			tfs = (TFS_HEADER*)((unsigned char*)tfs + sizeof(TFS_HEADER) + tfs->size); // point right next to the data block
			break; 
			
		}

		tfs = tfs->next;
	}

	// 32 bit allignment
	if((unsigned int)tfs & 0x00000003)
		tfs = (TFS_HEADER*)(((unsigned int)tfs & ~0x00000003) + 4); 


	//print("XXXXXXX: tfs: %p, end_of_tfs: %p\r\n", tfs, end_of_tfs);

	int free_space = (int)end_of_tfs - (int)tfs;

	if(free_space < 0) {
		print("tfs_get_free_space() Corrupted file system, tfs %p (next: %p) points beyond file system space\r\n", tfs_prev, tfs_prev->next);
		return 0;
	}

	if(free_space - sizeof(TFS_HEADER) > 0)
		return free_space - sizeof(TFS_HEADER);

	return 0; // available space is not sufficient to store TFS header
}

// erase flash, all files will be lost.
int tfs_format(void)
{
        int rc;


        print("tfs_format() Unlocking flash\r\n");
        FLASH_Unlock();

        /* Clear All pending flags */
        FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR |  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);


	for(int i = TFS_FIRST_SECTOR; i < TFS_LAST_SECTOR+1; i++) {
        	print("tfs_format() Erasing flash, sector_number=%d at %p\r\n", i, FLASH_SECTOR2ADDR[i]);
        	//__disable_irq();
        	rc = FLASH_EraseSector(FLASH_SECTOR2NUMBER[i], VoltageRange_3);
       		//__enable_irq();



        	if(rc!=FLASH_COMPLETE) {
        		print("tfs_format() Locking flash\r\n");
        		FLASH_Lock();//lock flash anyway
                	print("tfs_format() FLASH_EraseSector error, rc=%d\r\n", rc);
			return rc;
        	}
	}

       	print("tfs_format() Locking flash\r\n");
      	FLASH_Lock();//lock flash anyway

        print("tfs_format() COMPLETE!\r\n");

        return 0;
}




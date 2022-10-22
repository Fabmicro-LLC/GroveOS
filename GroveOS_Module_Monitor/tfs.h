/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef __TFS_H__
#define __TFS_H__

// Below API provides a simple file sysmtem ot be deployed on NAND flash
// 1. Files a contiguous, means there is on one large data block for each file of a variable size
// 2. Files cannot be removed, but can be marked as deleted
// 3. File size is defined upon creation.
// 4. Files cannot be appended. To append data one has to copy file to a new one with a bigger creation size

typedef struct _TFS_HEADER TFS_HEADER;

struct _TFS_HEADER {
	unsigned int magic;		// "TFS\0" = 0x54 0x46 0x53 0x00 = 0x00534654
	char name[256];			// null terminated string of file name
	unsigned short flags;		// 0xffff - normal file, 0x0000 - deleted file
	unsigned short crc16;		// CRC16 of the file data block - 0xffff if ffile has not been written yet
	unsigned int date;		// creation date - RTC BCD format
	unsigned int time;		// creation time - RTC BCD format
	unsigned int size;		// file size in bytes, 0xffffff - means file has not been written in full yet
	TFS_HEADER*  next;		// Points to the next TFS header, reletive to file system start address
};
// File data blocks follow immidiately
// Data block is 32 bit padded at the end, thus TFS headers are 32 bit alligned

TFS_HEADER* tfs_create_file(char *name, int name_len, int file_size); // creates new file, return pointer to new TFS or NULL if no free space available
int tfs_write_block(TFS_HEADER* tfs, char* block, int block_size, int offset); // writes a piece of data to this file, offset must be tracked by user. 
									   // CAUTION: writing to same offset twice will get data corrupted!
int tfs_close(TFS_HEADER* tfs); // calculates and sets CRC16 for this file, called after file is fully written. Returns CRC16.
int tfs_delete(TFS_HEADER* tfs); // mark this file as deleted, 0 - success, -1 - bogus TFS
TFS_HEADER* tfs_find(char *name); // finds and returns TFS of an existing file by null terminated string.
TFS_HEADER* tfs_find_next(TFS_HEADER* tfs); // finds next file following provided TFS. To find first file use tfs=NULL
int tfs_get_free_space(void); // returns max size of file that is possible to write to file system
int tfs_format(void); // erase flash, all files will be lost.
TFS_HEADER* tfs_get_begin(void); // returns beginning address of file system  
TFS_HEADER* tfs_get_end(void); // returns endinging address of file system plus one
unsigned int tfs_get_capacitance(void);

#define	TFS_MAGIC		0x00534654
#define	TFS_FIRST_SECTOR	9	// first sector of Tiny File System 
#define	TFS_LAST_SECTOR		16	// last sector of Tiny File System 

#define	TFS_FLAGS_DEL		0x8000	// setting this bit to zero marks file as deleted


#endif // end of __TFS_H__

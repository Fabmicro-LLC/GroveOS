#ifndef _TFS_H_
#define _TFS_H_

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

#ifndef SVC_CLIENT

TFS_HEADER* tfs_create_file(char *name, int name_len, int file_size); // creates new file, return pointer to new TFS or NULL if no free space available
int tfs_write_block(TFS_HEADER* tfs, char* block, int block_size, int offset); // writes a piece of data to this file, offset must be tracked by user. 
									   // CAUTION: writing to same offset twice will get data corrupted!
int tfs_read_block(TFS_HEADER* tfs, int offset, char* dst, int block_size);
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


#else

// Below API provides a simple file system to be deployed on NAND flash
// 1. Files a contiguous, means there is on one large data block for each file of a variable size
// 2. Files cannot be removed, but can be marked as deleted
// 3. File size is defined upon creation.
// 4. Files cannot be appended. To append data one has to copy file to a new one with a bigger creation size


TFS_HEADER* svc_tfs_create(char *name, int name_len, int file_size); // creates new file, return pointer to new TFS or NULL if no free space available
                                                                     // CAUTION: If file exists it will be deleted and a new file with same name created
int svc_tfs_write(TFS_HEADER* tfs, char* block, int block_size, int offset); // writes a piece of data to this file, offset must be tracked by user.
                                                                             // CAUTION: writing to same offset twice will get data corrupted!
int svc_tfs_close(TFS_HEADER* tfs); // calculates and sets CRC16 for this file, called after file is fully written. Returns CRC16.
TFS_HEADER* svc_tfs_find(char *name); // finds and returns TFS of an existing file by null terminated string.
TFS_HEADER* svc_tfs_find_next(TFS_HEADER* tfs); // finds next file following provided TFS. To find first file use tfs=NULL
TFS_HEADER* svc_tfs_get_begin(void); // Returns TFS pointing to beginning of file system 
int svc_tfs_get_free(void); // returns max size of file that is possible to write to file system
void svc_tfs_format(void); // Erases all data on Flash including application's own code. The only feasible call after this is svc_close(), otherwise Exception!
int svc_tfs_aplay(char *name); // Play PCMU audio file.

#endif //SVC_CLIENT
#endif //_TFS_H_

#ifdef SVC_CLIENT_IMPL

#ifndef _TFS_H_IMPL_
#define _TFS_H_IMPL_

#include "svc.h"

// creates new file, return pointer to new TFS or NULL if no free space available
// CAUTION: If file exists it will be deleted and a new file with same name created
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_create(char *name, int name_len, int file_size)
{
        svc(SVC_TFS_CREATE);
}

// writes a piece of data to this file, offset must be tracked by user.
// CAUTION: writing to same offset twice will get data corrupted!
__attribute__ ((noinline)) int svc_tfs_write(TFS_HEADER* tfs, char* block, int block_size, int offset)
{
        svc(SVC_TFS_WRITE);
}

// calculates and sets CRC16 for this file, called after file is fully written. Returns CRC16.
__attribute__ ((noinline)) int svc_tfs_close(TFS_HEADER* tfs)
{
        svc(SVC_TFS_CLOSE);
}

// finds and returns TFS of an existing file by null terminated string.
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_find(char *name)
{
        svc(SVC_TFS_FIND);
}

// finds next file following provided TFS. To find first file use tfs=NULL
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_find_next(TFS_HEADER* tfs)
{
        svc(SVC_TFS_FIND_NEXT);
}

// returns max size of file that is possible to write to file system
__attribute__ ((noinline)) int svc_tfs_get_free(void)
{
        svc(SVC_TFS_GET_FREE);
}

// returns tfs pointing to beginnign of system
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_get_begin(void)
{
        svc(SVC_TFS_GET_BEGIN);
}

// Erase all data on flash, including application own text. After this call the only feasible call is svc_ctop()
__attribute__ ((noinline)) void svc_tfs_format(void)
{
        svc(SVC_TFS_FORMAT);
}

// Play PCMU audio file.
__attribute__ ((noinline)) int svc_tfs_aplay(char *name)
{
        svc(SVC_TFS_APLAY);
}

#endif //_TFS_H_IMPL_
#endif //SVC_CLIENT_IMPL

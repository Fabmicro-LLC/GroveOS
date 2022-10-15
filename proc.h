#ifndef __PROC_H__
#define __PROC_H__


#define PROC_MAX	32

typedef PROC struct _PROC {
        TFS_HEADER* application_tfs = NULL;
        void* application_elf = NULL;
        uint32_t application_elf_size = 0;
        int (*application_start_addr)(void) = (int(*)(void))NULL;
        void (*application_process_msg_addr)(MSG*) = (void(*)(MSG*))NULL;
        uint8_t* application_data_addr = NULL;
        uint32_t application_text_addr = 0;
        uint32_t application_data_size = 0;
}


extern PROC proc[];

void proc_list(void); // enlist currently running processes
PROC* proc_find_by_name(char* name); // returns pointer to PROC of a running process by its file name
PROC* proc_find_by_addr(void* addr); // returns pointer to PTOC of a running process by its text and data address


#endif // __PROC_H__
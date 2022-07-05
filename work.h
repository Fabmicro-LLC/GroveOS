#ifndef ___WORK_H___
#define ___WORK_H___

void init();
void prompt(void);
void process_message(MSG* msg);
void stop_application(void* application_text);

#define	MONITOR_INVOKE		5	// microsecons to hold UP and DOWN keys to run zmodem
#define	APPLICATION_MAX_MEM	16	// Max numbe rof memory blocks available for application. Each block is one malloc()
#define	MODBUS_SECURITY_KEY	0x01234567	// Use this key when initiating Modbus TFS requests


#endif

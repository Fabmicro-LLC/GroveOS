#ifndef _MSG_H_
#define _MSG_H_

typedef struct {
	int message;
	int p1;
	int p2;
} MSG;

#define AUDIO_RECORD_DMA_HT			1
#define AUDIO_RECORD_DMA_TC			2
#define AUDIO_PLAY_DMA_HT			3
#define AUDIO_PLAY_DMA_TC			4

#define	MICRO_TIMER				5
#define INFO_TIMER_INT				6	
#define LCD_DMA_TC				7	
#define SPI_DMA_TX_TC				8	
#define LCD_INT_ACTIVE				9	
#define LCD_INT_RELEASED			10	
#define	AUDIO_PLAY_STOP				11	

#define LEFT_ENCODER_READY                      12      
#define LEFT_ENCODER_ERROR                      13      
#define RIGHT_ENCODER_READY                     14      
#define RIGHT_ENCODER_ERROR                     15      
#define WHEEL_ENCODER_READY                     16      
#define WHEEL_ENCODER_ERROR                     17


#define	ADC1_EVENT				20
#define CHECK_FOR_AUTOEXEC			21

// Controls
#define	KEY_UP_IRQ				31	
#define	KEY_DOWN_IRQ				32	
#define	KEY_LEFT_IRQ				33	
#define	KEY_RIGHT_IRQ				34	


// LCD touch
#define	LCD_TOUCH				40
#define	LCD_UNTOUCH				41
#define LCD_TOUCH_MOVED				42

// PWM DMA
#define	DC_PWM_1_DMA_TC				43
#define	DC_PWM_2_DMA_TC				44
#define	DC_PWM_3_DMA_TC				45
#define	DC_PWM_4_DMA_TC				46

#define APS_RX_CMD				51

#define	EXTI1_IRQ				52

//
#define	EXCEPTION_APPLICATION			65
#define	EXCEPTION_SUPERVISOR			66	
#define	SVC_EXEC_PENDING			67
#define	SVC_FORMAT_PENDING			68

//FLASH memory
#define FLASH_ERASE_OK                          70
#define FLASH_ERASE_ERROR                       71


#define	RTC_ALARM				80
#define	USART1_RX_DATA				81
#define	USART2_RX_DATA				82
#define	MODBUS1_RX_COMPLETE			83
#define	MODBUS1_WRITE_HOLD_REGS			84
#define	MODBUS1_READ_HOLD_REGS			85
#define	USART3_RX_DATA				86
#define	MODBUS2_RX_COMPLETE			87
#define	MODBUS2_WRITE_HOLD_REGS			88
#define	MODBUS2_READ_HOLD_REGS			89

#define	DALI1_TX_COMPLETE			90
#define	DALI1_TX_DELAY_COMPLETE			91	
#define	DALI1_RX_COMPLETE			92	
#define DALI1_RESPONSE_RECEIVED			93
#define DALI1_RESPONSE_NOT_RECEIVED		94

#define	DALI2_TX_COMPLETE			95
#define	DALI2_TX_DELAY_COMPLETE			96	
#define	DALI2_RX_COMPLETE			97	
#define DALI2_RESPONSE_RECEIVED			98
#define DALI2_RESPONSE_NOT_RECEIVED		99



#ifndef SVC_CLIENT

extern int main_queue_enabled;
unsigned int main_queue_changed;
unsigned int main_queue_wait_counter;
unsigned int error_main_queue_isfull;

void main_queue_init();
int main_queue_len();
void main_queue_clear();

int PostMessage(int message,  int unique, int p1, int p2);
int PostMessageIRQ(int message,  int unique, int p1, int p2);
int GetMessage(MSG* msg);

#endif //SVC_CLIENT

#endif //_MSG_H_

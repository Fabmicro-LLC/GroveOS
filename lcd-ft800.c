#include "hardware.h"

#include <stm32f4xx_usart.h>
#include "lcd-ft800.h"
#include "utils.h"
#include "ft_gpu.h"
#include "msg.h"

static int lcd_speaker_enable = 0;
static int lcd_new_brightness; //= 64;
static int lcd_brightness;// = 64;

int lcd_irq_requested = 0;
int lcd_touched = 0;
int lcd_touch_x = 0;
int lcd_touch_y = 0;

#ifdef LCD_SPI

void lcd_init() {
	print("lcd_init() Initializing LCD\n");
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_SPI_CS_GPIO_PERIPH, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_SPI_PD_GPIO_PERIPH, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_SPI_INT_GPIO_PERIPH, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_SPI_SCK_GPIO_PERIPH, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_SPI_MISO_GPIO_PERIPH, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_SPI_MOSI_GPIO_PERIPH, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_DMA_PERIPH, ENABLE);
	
	if(LCD_SPI_PERIPH == RCC_APB1Periph_SPI2)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	if(LCD_SPI_PERIPH == RCC_APB2Periph_SPI1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	gpio_init(LCD_SPI_CS_GPIO_PERIPH, GPIO_SPEED, LCD_SPI_CS_GPIO_PORT, LCD_SPI_CS_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(LCD_SPI_PD_GPIO_PERIPH, GPIO_SPEED, LCD_SPI_PD_GPIO_PORT, LCD_SPI_PD_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(LCD_SPI_INT_GPIO_PERIPH, GPIO_SPEED, LCD_SPI_INT_GPIO_PORT, LCD_SPI_INT_GPIO_PIN, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP);
	gpio_irq(LCD_SPI_INT_IRQ_PORT_SRC, LCD_SPI_INT_IRQ_PIN_SRC, LCD_SPI_INT_IRQ_LINE, LCD_SPI_INT_IRQ_CHAN, LCD_SPI_INT_IRQ_RAISFALL);

	gpio_init(LCD_SPI_SCK_GPIO_PERIPH, LCD_GPIO_SPEED, LCD_SPI_SCK_GPIO_PORT, LCD_SPI_SCK_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP);
	gpio_init(LCD_SPI_MISO_GPIO_PERIPH, LCD_GPIO_SPEED, LCD_SPI_MISO_GPIO_PORT, LCD_SPI_MISO_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP);
	gpio_init(LCD_SPI_MOSI_GPIO_PERIPH, LCD_GPIO_SPEED, LCD_SPI_MOSI_GPIO_PORT, LCD_SPI_MOSI_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP);

  	// Connect SPI pins to AF5 
  	GPIO_PinAFConfig(LCD_SPI_SCK_GPIO_PORT, LCD_SPI_SCK_GPIO_PIN_SRC, LCD_SPI_AF);//SCK
  	GPIO_PinAFConfig(LCD_SPI_MISO_GPIO_PORT, LCD_SPI_MISO_GPIO_PIN_SRC, LCD_SPI_AF);//MISO
  	GPIO_PinAFConfig(LCD_SPI_MOSI_GPIO_PORT, LCD_SPI_MOSI_GPIO_PIN_SRC, LCD_SPI_AF);//MOSI

  	/* SPI configuration -------------------------------------------------------*/
  	SPI_DeInit(LCD_SPI);
  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  	//SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;// SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//SPI_CPHA_2Edge;
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;// SPI_CPOL_Low;
  	//SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//SPI_CPHA_1Edge;
  	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  	SPI_InitStructure.SPI_BaudRatePrescaler = LCD_SPI_BAUDRATE_PRESCALER; //SPI_BaudRatePrescaler_2; //SPI_BaudRatePrescaler_8;//SPI_BaudRatePrescaler_4;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(LCD_SPI, &SPI_InitStructure);

	lcd_setup();

	#ifndef STM32F42X 
		RCC_PCLK2Config(RCC_HCLK_Div1); // This doubles all bus clocks. Default is Div2
	#endif

	//snd_play_c8();

	lcd_speaker_enable = 0;

	print("lcd_ini() Initializing LCD done\n");

}

void LCD_DMA_TX_STREAM_IRQ_HANDLER(void) {
	__disable_irq();
	int tc = DMA_GetITStatus(LCD_DMA_TX_STREAM, LCD_DMA_IT);
	if(tc) {
	        DMA_ClearITPendingBit(LCD_DMA_TX_STREAM, LCD_DMA_IT);
		lcd_spi_cs(1);

	        PostMessageIRQ(LCD_DMA_TC, 1, 0, 0);
	}
	__enable_irq();
}




uint8_t lcd_write_memory_dma_irq(uint32_t addr, uint8_t* aTxBuffer, uint32_t buffer_size) {

	if(lcd_speaker_enable) {
	        lcd_write_mem8(REG_GPIO,0x82);
	} else {
	        lcd_write_mem8(REG_GPIO,0x80);
	}

	if(lcd_new_brightness != lcd_brightness) {
	        lcd_brightness = lcd_new_brightness;
		int safe_br = (lcd_new_brightness ==0 ? 5 : lcd_new_brightness);
	        lcd_write_mem8(REG_PWM_DUTY, safe_br);
	}

	lcd_spi_cs(0);

	spi_write_byte(LCD_SPI, ((addr >> 16) & 0x3F) | 0x80 );
	spi_write_byte(LCD_SPI, ((addr >> 8) & 0xFF) );
	spi_write_byte(LCD_SPI, (addr & 0xFF) );

	DMA_InitTypeDef DMA_InitStructure;

	/* DMA configuration -------------------------------------------------------*/

	/* Configure DMA Initialization Structure */
	DMA_InitStructure.DMA_BufferSize = buffer_size ;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(LCD_SPI->DR)) ;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	/* Configure TX DMA */
	DMA_InitStructure.DMA_Channel = LCD_DMA_CHANNEL ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aTxBuffer ;
	DMA_Init(LCD_DMA_TX_STREAM, &DMA_InitStructure);

	DMA_ITConfig(LCD_DMA_TX_STREAM, DMA_IT_TC , ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = LCD_DMA_NVIC_IRQCHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

       	/* Enable DMA SPI TX Stream */
	DMA_Cmd(LCD_DMA_TX_STREAM,ENABLE);

       	/* Enable SPI DMA TX Requsts */
	SPI_I2S_DMACmd(LCD_SPI, SPI_I2S_DMAReq_Tx, ENABLE);

       	SPI_Cmd(LCD_SPI, ENABLE);

	return 0;
}


uint8_t spi_dma_xfer(SPI_TypeDef* SPIx, uint8_t* aTxBuffer, uint32_t buffer_size) {

	DMA_InitTypeDef	DMA_InitStructure;

	/* DMA configuration -------------------------------------------------------*/
	/* Deinitialize DMA Streams */
	DMA_DeInit(LCD_DMA_TX_STREAM); // TX

	/* Configure DMA Initialization Structure */
	DMA_InitStructure.DMA_BufferSize = buffer_size ;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPIx->DR)) ;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	if(aTxBuffer) {
		/* Configure TX DMA */
		DMA_InitStructure.DMA_Channel = LCD_DMA_CHANNEL ;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
		DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aTxBuffer ;
		DMA_Init(LCD_DMA_TX_STREAM, &DMA_InitStructure);

		/* Enable DMA SPI TX Stream */
		DMA_Cmd(LCD_DMA_TX_STREAM,ENABLE);

		/* Enable SPI DMA TX Requsts */
		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, ENABLE);
	}


	SPI_Cmd(LCD_SPI, ENABLE);


	/* Waiting the end of Data transfer */
	if(aTxBuffer) while (DMA_GetFlagStatus(LCD_DMA_TX_STREAM,LCD_DMA_TX_TCFLAG)==RESET);

  	/* Clear DMA Transfer Complete Flags */
	DMA_ClearFlag(LCD_DMA_TX_STREAM, LCD_DMA_TX_TCFLAG);

	/* Disable DMA SPI TX Stream */
	DMA_Cmd(LCD_DMA_TX_STREAM,DISABLE);

	/* Disable SPI DMA TX Requsts */
	SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, DISABLE);

	return 0;
}


int spi_write_byte(SPI_TypeDef* SPIx, uint8_t tx_data) {
	int i;
	//while (SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_TXE) == RESET);
	for(i = 0; i < 9999; i++) {
		if(!(SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_TXE) == RESET))
			break;
	}
	if(i == 9999)
		return -1;

	SPI_I2S_SendData(SPIx , tx_data);

	//while (SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_RXNE) == RESET);
	for(i = 0; i < 9999; i++) {
		if(!(SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_RXNE) == RESET))
			break;
	}
	if(i == 9999)
		return -2;

	// dummy read
	SPI_I2S_ReceiveData(SPIx);
	return 0;
}

int spi_write_read_byte(SPI_TypeDef* SPIx, uint8_t tx_data, uint8_t* rx_data) {

	int i;

	if(!rx_data)
		return -1;

	//while (SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_TXE) == RESET);
	for(i = 0; i < 9999; i++) {
		if(!(SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_TXE) == RESET))
			break;
	}
	if(i == 9999)
		return -2;

	SPI_I2S_SendData(SPIx , tx_data);

	//while (SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_RXNE) == RESET);
	for(i = 0; i < 9999; i++) {
		if(!(SPI_I2S_GetFlagStatus(SPIx , SPI_I2S_FLAG_RXNE) == RESET))
			break;
	}
	if(i == 9999)
		return -3;

	*rx_data = SPI_I2S_ReceiveData(SPIx);
	return 0;
}

void lcd_spi_cs(BitAction val) {
	GPIO_WriteBit(LCD_SPI_CS_GPIO_PORT,  LCD_SPI_CS_GPIO_PIN, val);
}

uint8_t lcd_write_host_cmd(uint8_t cmd) {


	lcd_spi_cs(0);
	uint8_t txbuf[4];

	txbuf[0] = cmd;
	txbuf[1] = 0x00;
	txbuf[2] = 0x00;
	
	uint8_t status = spi_dma_xfer(LCD_SPI, txbuf, 3);

	lcd_spi_cs(1);

	return status;
}

uint8_t lcd_write_mem8(uint32_t addr, uint8_t data) {

	lcd_spi_cs(0);
	uint8_t txbuf[4];

	txbuf[0] = (((addr >> 16) & 0x3F) | 0x80 );
	txbuf[1] = (((addr >> 8) & 0xFF) );
	txbuf[2] = ((addr & 0xFF) );
	txbuf[3] = data;
	
	uint8_t status = spi_dma_xfer(LCD_SPI, txbuf, 4);

	lcd_spi_cs(1);

	return status;
}

uint8_t lcd_write_mem16(uint32_t addr, uint16_t data) {

	lcd_spi_cs(0);
	uint8_t txbuf[5];

	txbuf[0] = (((addr >> 16) & 0x3F) | 0x80 );
	txbuf[1] = (((addr >> 8) & 0xFF) );
	txbuf[2] = ((addr & 0xFF) );
	txbuf[3] = data & 0xFF;
	txbuf[4] = (data >> 8) & 0xFF ;
	
	uint8_t status = spi_dma_xfer(LCD_SPI, txbuf, 5);

	lcd_spi_cs(1);

	return status;
}

uint8_t lcd_write_mem32(uint32_t addr, uint32_t data) {

	lcd_spi_cs(0);
	uint8_t txbuf[7];

	txbuf[0] = (((addr >> 16) & 0x3F) | 0x80 );
	txbuf[1] = (((addr >> 8) & 0xFF) );
	txbuf[2] = ((addr & 0xFF) );
	txbuf[3] = data & 0xFF;
	txbuf[4] = (data >> 8) & 0xFF ;
	txbuf[5] = (data >> 16) & 0xFF ;
	txbuf[6] = (data >> 24) & 0xFF ;
	
	uint8_t status = spi_dma_xfer(LCD_SPI, txbuf, 7);

	lcd_spi_cs(1);

	return status;
}

uint8_t lcd_write_memory(uint32_t addr, uint8_t* buffer, uint32_t length) {

	lcd_spi_cs(0);

	uint8_t status = 
	spi_write_byte(LCD_SPI, ((addr >> 16) & 0x3F) | 0x80 );
	spi_write_byte(LCD_SPI, ((addr >> 8) & 0xFF) );
	spi_write_byte(LCD_SPI, (addr & 0xFF) );

	spi_dma_xfer(LCD_SPI, buffer, length);

	lcd_spi_cs(1);

	return status;
}

static uint32_t lcd_dli = 0; 

void lcd_dl_flush(void) { 
	lcd_write_mem8(REG_DLSWAP,DLSWAP_FRAME);//display list swap
	lcd_dli = 0;
	DelayLoopMicro(1000);
}

void lcd_dl(unsigned long cmd) { 
	lcd_write_mem32(RAM_DL + lcd_dli, cmd); 
	lcd_dli += 4; 
}


void snd_play_c8(void) {

	lcd_write_mem8(REG_VOL_SOUND,0xFF); //set the volume to maximum 
	lcd_write_mem16(REG_SOUND, (0x6C<< 8) | 0x41); // C8 MIDI note on xylophone 
	lcd_write_mem8(REG_PLAY, 1); // play the sound

}


void lcd_setup(void) {
	uint32_t val;

	// Reset power on LCD
	GPIO_ResetBits(LCD_SPI_PD_GPIO_PORT,  LCD_SPI_PD_GPIO_PIN);
	DelayLoopMicro(100000);
	GPIO_SetBits(LCD_SPI_PD_GPIO_PORT,  LCD_SPI_PD_GPIO_PIN);
	DelayLoopMicro(100000);

	lcd_write_host_cmd(0x00); // ACTIVE - switch from stand-by mode 
	DelayLoopMicro(40000);
	lcd_write_host_cmd(0x68); // Send reset pulse to FT800 core
	DelayLoopMicro(40000);
	lcd_write_host_cmd(0x44); // CLKEXT
	DelayLoopMicro(20000);
	lcd_write_host_cmd(0x62); // Switch PLL output clock to 48MHz
	DelayLoopMicro(20000);
	 

	// setup RGB bus
	lcd_write_mem16(REG_HCYCLE, 548);
	lcd_write_mem16(REG_HOFFSET, 43);
	lcd_write_mem16(REG_HSYNC0, 0);
	lcd_write_mem16(REG_HSYNC1, 41);
	lcd_write_mem16(REG_VCYCLE, 292);
	lcd_write_mem16(REG_VOFFSET, 12);
	lcd_write_mem16(REG_VSYNC0, 0);
	lcd_write_mem16(REG_VSYNC1, 10);
	lcd_write_mem8(REG_SWIZZLE, 0);
	lcd_write_mem8(REG_PCLK_POL, 1);
	lcd_write_mem8(REG_CSPREAD, 1);
	lcd_write_mem16(REG_HSIZE, 480);
	lcd_write_mem16(REG_VSIZE, 272);

	/* write first display list */ 
	lcd_write_mem32(RAM_DL+0,CLEAR_COLOR_RGB(0xff,0,0)); 
	lcd_write_mem32(RAM_DL+4,CLEAR(1,1,1)); 
	lcd_write_mem32(RAM_DL+8,DISPLAY());

	lcd_write_mem8(REG_DLSWAP,DLSWAP_FRAME);//display list swap

	/*Set DISP_EN and SOUND_EN to 1*/
	lcd_write_mem8(REG_GPIO_DIR,0x82); 
	//lcd_write_mem8(REG_GPIO,0x82);//enable display bit
	lcd_write_mem8(REG_GPIO,0x80);//disable audio amp bit

	lcd_write_mem8(REG_PCLK,5);//after this display is visible on the LCD

	// backlight
	lcd_brightness = lcd_new_brightness = 100; //64;
	lcd_write_mem16(REG_PWM_HZ, 500);
	lcd_write_mem8(REG_PWM_DUTY, lcd_brightness);

	/* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
	lcd_write_mem16(REG_TOUCH_RZTHRESH, 4800);

	lcd_write_mem32(REG_INT_MASK, 0x82); // allow interrupts from touch screen only 
	lcd_write_mem32(REG_INT_EN, 1);	// enable interrupts from LCD
	//lcd_write_mem32(REG_TOUCH_MODE, 1);	// sense once 
	lcd_write_mem32(REG_TOUCH_MODE, 2);	// sense each frame
	lcd_read_mem32(REG_INT_FLAGS, &val); // clear flags by reading them

}

void lcd_backlight_on(int on_off) {
	if(on_off) ;
	else ;
}

int lcd_read_mem32(uint32_t addr, uint32_t *data) {
	int i,j;
	uint8_t rxbuf[8]={0};
	uint8_t txbuf[8]={0};

	if(data==NULL) {
	        return -1;
	}


	txbuf[0] = (((addr >> 16) & 0xFF)) & 0x3F;
	txbuf[1] = (((addr >> 8) & 0xFF));
	txbuf[2] = (((addr >> 0) & 0xFF));
	txbuf[3] = 0; // dummy bytes

	__disable_irq();

	for(i = 0; i < 9999; i++) {
		if(SPI_I2S_GetFlagStatus(LCD_SPI , SPI_I2S_FLAG_TXE))
			break;
	}
	if(i == 9999) {
		__enable_irq();
		return -2;
	}

	lcd_spi_cs(0);

	for(j = 0; j < 8; j++) {


		SPI_I2S_SendData(LCD_SPI, txbuf[j]);

		for(i = 0; i < 9999; i++) {
			if(SPI_I2S_GetFlagStatus(LCD_SPI , SPI_I2S_FLAG_RXNE))
				break;
			DelayLoopMicro(1);
		}
		if(i == 9999) {
			__enable_irq();
			return -3;
		}

	        rxbuf[j] = SPI_I2S_ReceiveData(LCD_SPI);

		//print("MEM32: j = %d, Tx: 0x%02x, Rx: 0x%02x\r\n", j, txbuf[j], rxbuf[j]);
	}
	
	lcd_spi_cs(1);

	__enable_irq();


	*data = rxbuf[4] | (rxbuf[5] << 8) | (rxbuf[6] << 16) | (rxbuf[7] << 24) ;

	return 0;
}

int lcd_check() {
	uint32_t val=0, rc=0, result=0;
	rc = lcd_read_mem32(REG_ID, &val);

	if(rc) {
	        print("lcd_check lcd_read_mem32(%p) FAILED, rc = %d, val = 0x%08X\r\n", REG_ID, rc, val);
	        return 1; //lcd is not working or not connected
	} else {

	        switch(val) {
	        case 0x7c42:
	        case 0x7c:
	                result = 0;//lcd is working
	                break;
	        case 0xffffffff:
	                result = 1;//lcd is not working or not connected
	                break;
	        default:
	                result = -1;//lcd is not inited or wrongly initilized
	                break;
	        }

	        if(result) print("lcd_check lcd_read_mem32(%p) val = 0x%08X, result = %d\r\n", REG_ID, val, result);

	        return result;
	}
}

void lcd_enable_speaker (int enable) {
	lcd_speaker_enable = enable;
}

void lcd_set_brightness(int percent) {
	lcd_new_brightness = percent;
}

int lcd_read_touch(int *X, int *Y, int *Preasure) {
	uint32_t rc, val;
	rc = lcd_read_mem32(REG_TOUCH_SCREEN_XY, &val);
	*X = (val >> 16) & 0x03FF;
	*Y = val & 0x03FF;
	rc = lcd_read_mem32(REG_TOUCH_RZ, &val);
	*Preasure = val & 0xFFFF;

	lcd_read_mem32(REG_INT_FLAGS, &val); // clear flags by reading them

	return rc;
}



#endif



#ifndef ___LCD_H___
#define ___LCD_H___

#include "stm32f4xx.h"

/////////////////////////////////////

#define LCD_WIDTH       480
#define LCD_HEIGHT      272

extern int lcd_irq_requested;
extern int lcd_touched;
extern int lcd_touch_x;
extern int lcd_touch_y;

void lcd_init();

int spi_write_byte(SPI_TypeDef* SPIx, uint8_t tx_data);
int spi_write_read_byte(SPI_TypeDef* SPIx, uint8_t tx_data, uint8_t* rx_data);

void lcd_spi_cs(BitAction val);
uint8_t spi_dma_xfer(SPI_TypeDef* SPIx, uint8_t* aTxBuffer, uint32_t buffer_size);
uint8_t lcd_write_memory_dma_irq(uint32_t addr, uint8_t* aTxBuffer, uint32_t buffer_size);
uint8_t lcd_write_host_cmd(uint8_t value);
uint8_t lcd_write_mem8(uint32_t addr, uint8_t value);
uint8_t lcd_write_mem16(uint32_t addr, uint16_t value);
uint8_t lcd_write_mem32(uint32_t addr, uint32_t value);
int lcd_read_mem32(uint32_t addr, uint32_t *data);
uint8_t lcd_write_memory(uint32_t addr, uint8_t* buffer, uint32_t length);
void lcd_dl(unsigned long cmd);
void lcd_dl_flush(void);
void lcd_backlight_on(int on_off);

void snd_play_c8(void);
void lcd_setup(void);
int lcd_check();

void lcd_enable_speaker(int enable);
void lcd_set_brightness(int percent);

int lcd_read_touch(int *X, int *Y, int *Preasure);



#endif

CROSS=/opt/STM32/gcc-arm-none-eabi-4_7-2013q3/bin/arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
AR=$(CROSS)ar
CP=$(CROSS)objcopy
OD=$(CROSS)objdump
GDB=$(CROSS)gdb
STRIP=$(CROSS)strip
READELF=$(CROSS)readelf
STM32Libraries=../../STM32Libraries

MCFLAGS = -O2 -mcpu=cortex-m4 -mthumb -mthumb-interwork -std=gnu99 -nostartfiles -mfloat-abi=hard  -mfpu=fpv4-sp-d16  -mfp16-format=ieee 

VPATH	= ./

STM32_INCLUDES = -I$(STM32Libraries)/CMSIS/Device/ST/STM32F4xx/Include/ \
	-I$(STM32Libraries)/CMSIS/Include/ \
	-I$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/inc/
	

INCLUDES = $(STM32_INCLUDES) -I./ -I./uilib/

STATIC_LIBS = stm32f4.a 

DEFS = 	-DUSE_STDPERIPH_DRIVER -DSTM32F4XX \
	-DHAVE_CONFIG_H \
	-DHSE_VALUE=8000000 \
	-DUSE_USB_OTG_FS

CFLAGS	= $(MCFLAGS) $(DEFS) $(INCLUDES)
LDFLAGS = -lnosys -Wl,-T,groveos_module.ld -Wl,-nmagic 

SRC = monitor.c 

STM32_SRC = svc_user.c printf.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/misc.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c  \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
	$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c

	#../$(STM32Libraries)/STM32_USB_Device_Library/Class/cdc/src/usbd_cdc_core.c \
	#../$(STM32Libraries)/STM32_USB_Device_Library/Core/src/usbd_req.c \
	#../$(STM32Libraries)/STM32_USB_Device_Library/Core/src/usbd_core.c \
	#../$(STM32Libraries)/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
	#../$(STM32Libraries)/STM32_USB_OTG_Driver/src/usb_dcd.c \
	#../$(STM32Libraries)/STM32_USB_OTG_Driver/src/usb_core.c \
	#../$(STM32Libraries)/STM32_USB_OTG_Driver/src/usb_dcd_int.c \

	#../$(STM32Libraries)/STM32_USB_OTG_Driver/src/usb_core.c \
	#../$(STM32Libraries)/STM32_USB_OTG_Driver/src/usb_otg.c 
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c 
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c 
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
	#../$(STM32Libraries)/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c 

STM32_OBJ     =$(STM32_SRC:.c=.o)


all: monitor.elf

monitor.hex: monitor.elf
	$(CP) -O ihex $^ $@

monitor.elf: $(SRC) $(STATIC_LIBS)  
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	$(STRIP) $@

strip: monitor.elf
	$(STRIP) $^

$(STM32_OBJ): %.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@


stm32f4.a: $(STM32_OBJ)
	$(AR) rcs $@ $^ 


clean:
	rm -f *.su *.hex *.o *.elf stm32f4.a 

disassm: monitor.elf
	$(OD) -s -l -S  $^

readelf: monitor.elf
	$(READELF) -a $^



CROSS=../../../gcc-arm-none-eabi-4_7-2013q3/bin/arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
AR=$(CROSS)ar
CP=$(CROSS)objcopy
OD=$(CROSS)objdump
GDB=$(CROSS)gdb
STRIP=$(CROSS)strip
READELF=$(CROSS)readelf

BUILD_NUMBER=`cat build_number`
DEVICE_NAME='"GroveOS"'

MCFLAGS = -g -O1 -mcpu=cortex-m4 -mthumb -mno-thumb-interwork -std=gnu99 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mfp16-format=ieee
#MCFLAGS = -O1 -mcpu=cortex-m4 -mthumb -mno-thumb-interwork -std=gnu99 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mfp16-format=ieee

STARTUP = ../../Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.s

STM32_INCLUDES = -I../../Libraries/CMSIS/Device/ST/STM32F4xx/Include/ \
	-I../../Libraries/CMSIS/Include/ \
	-I../../Libraries/STM32F4xx_StdPeriph_Driver/inc/ \
	-I../../Libraries/STM32_USB_OTG_Driver/inc \
	-I../../Libraries/STM32_USB_Device_Library/Core/inc \
	-I../../Libraries/STM32_USB_Device_Library/Class/cdc/inc
	

SPEEX_INCLUDES = 

INCLUDES = $(STM32_INCLUDES) $(SPEEX_INCLUDES) -I./ -I./gui -I./gui/wnd/  -I./gui/uilib/ -I./gui/svg/

STATIC_LIBS = 

DEFS = 	-DUSE_STDPERIPH_DRIVER -DSTM32F4XX \
	-DHAVE_CONFIG_H \
	-DHSE_VALUE=8000000 \
	-DUSE_USB_OTG_FS \
	-DBUILD_NUMBER=$(BUILD_NUMBER)+1\
	-D_DEVICE_NAME_=$(DEVICE_NAME)

CFLAGS	= $(MCFLAGS) $(DEFS) $(INCLUDES)
LDFLAGS =  -lc_s -lnosys -Wl,-T,grovex_c1_core.ld -Wl,-nmagic -u _printf_float
#LDFLAGS =  -lc -lnosys -Wl,-T,grovex_c1_core.ld -Wl,-nmagic 

FONTS = fonts/font_oled_6x9.c fonts/calibri_small.c fonts/calibri_medium.c fonts/calibri_large.c

BITMAPS = bitmaps/arrow_up_black.c bitmaps/arrow_down_black.c bitmaps/key_caps_white.c bitmaps/key_caps_black.c bitmaps/key_caps_green.c

SRC = 	ext_pwm.c ext_gpio.c g711.c audio.c vault.c dali1.c modbus_ext1.c modbus_ext2.c softtimer.c oled.c tfs.c svc.c main.c \
	zmodem.c utils.c stm32f4xx_it.c system_stm32f4xx.c ext_spi.c lcd-ft800.c msg.c work.c fbqueue.c adc.c crc.c \
	config.c crc16.c logger.c usart_dma.c elf.c ext_irq.c listener.c \
	gui/wnd/wnd.c gui/wnd/label.c gui/wnd/image.c gui/wnd/button.c gui/wnd/scroll.c gui/wnd/form.c gui/wnd/menu_cell.c \
	gui/alert.c gui/calibrate_touch.c gui/input_text.c gui/input_num.c gui/select_list.c gui/set_time.c gui/adc_setup.c \
	gui/cal_adc_channel.c \
        gui/svg/nsvg_paint.c  gui/svg/svgimage.c

STM32_SRC = ../../Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c  \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c

	#../../Libraries/STM32_USB_Device_Library/Class/cdc/src/usbd_cdc_core.c \
	#../../Libraries/STM32_USB_Device_Library/Core/src/usbd_req.c \
	#../../Libraries/STM32_USB_Device_Library/Core/src/usbd_core.c \
	#../../Libraries/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
	#../../Libraries/STM32_USB_OTG_Driver/src/usb_dcd.c \
	#../../Libraries/STM32_USB_OTG_Driver/src/usb_core.c \
	#../../Libraries/STM32_USB_OTG_Driver/src/usb_dcd_int.c \

	#../../Libraries/STM32_USB_OTG_Driver/src/usb_core.c \
	#../../Libraries/STM32_USB_OTG_Driver/src/usb_otg.c 
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c 
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c 
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
	#../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c 



C_SRC = $(SRC) $(FONTS) $(BITMAPS) $(SOUNDS) $(STM32_SRC)

all: 
	@echo 'Select target: aps | grovex | sputnik | smartpwr6'

grovex: PRODUCT = -DPRODUCT_GROVEX
grovex: main.hex

aps: PRODUCT = -DPRODUCT_APS
aps: main.hex

sputnik: PRODUCT = -DPRODUCT_SPUTNIK
sputnik: main.hex

smartpwr6: PRODUCT = -DPRODUCT_SMARTPWR6
smartpwr6: main.hex



main.hex: main.elf
	@expr $(BUILD_NUMBER) + 1 > build_number
	$(CP) -O ihex $^ $@

main.elf: $(C_SRC) $(STARTUP) monitor.o 
	$(CC) $(PRODUCT) $(CFLAGS) $^ $(LDFLAGS) $(STATIC_LIBS) -o $@
	cp main.elf main_debug.elf
	$(STRIP) $@


dict.h: dict.txt
	 ./parsedict.pl dict.txt dict.h

install: main.hex
	../../../stm32flash/stm32flash -w main.hex -v -b 115200 /dev/ttyUSB0

clean:
	rm -f *.su *.o main.hex main.elf 

readconfig:
	../../../stm32flash/stm32flash -r config.dump -S 0x8004000:2048 -b 57600 /dev/ttyUSB0
writeconfig:
	../../../stm32flash/stm32flash -w config.dump -S 0x8004000:2048 -b 57600 /dev/ttyUSB0

disassm: main_debug.elf
	$(OD) -l -S $^

debug_work: main.elf
	$(GDB) $^ -ex "target extended-remote 192.168.169.179:4242"

debug_home: main.elf
	$(GDB) $^ -ex "target extended-remote 192.168.168.124:4242"

debug_ugin: main_debug.elf
	$(GDB) $^ -ex "target extended-remote 10.211.55.3:4242"

readelf: main.elf
	$(READELF) -a $^

monitor.o: monitor.elf
	$(STRIP) $^
	$(CP) -I binary -O elf32-littlearm -B arm monitor.elf monitor.o	




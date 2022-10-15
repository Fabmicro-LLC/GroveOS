/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "hardware.h"
#include "ext_gpio.h"
#include "utils.h"

#ifdef GPIO_CHANNELS
EXT_GPIO ext_gpios[GPIO_CHANNELS];

int ext_gpio_init(void)
{
	#ifdef GPIO1_PORT
	ext_gpios[0].periph = GPIO1_PERIPH;
	ext_gpios[0].port = GPIO1_PORT;
        ext_gpios[0].pin = GPIO1_PIN;
	ext_gpios[0].mode = GPIO1_MODE;
	ext_gpios[0].descr = GPIO1_DESCR;
        gpio_init(ext_gpios[0].periph, GPIO_SPEED, ext_gpios[0].port, ext_gpios[0].pin, ext_gpios[0].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO1_IRQ_LINE
		ext_gpios[0].irq_port_src = GPIO1_IRQ_PORT_SRC;
		ext_gpios[0].irq_pin_src = GPIO1_IRQ_PIN_SRC;
		ext_gpios[0].irq_line = GPIO1_IRQ_LINE;
		ext_gpios[0].irq_chan = GPIO1_IRQ_CHAN;
		ext_gpios[0].irq_raisfall = GPIO1_IRQ_RAISFALL;
		gpio_irq(ext_gpios[0].irq_port_src, ext_gpios[0].irq_pin_src, ext_gpios[0].irq_line, ext_gpios[0].irq_chan, ext_gpios[0].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO2_PORT
	ext_gpios[1].periph = GPIO2_PERIPH;
	ext_gpios[1].port = GPIO2_PORT;
        ext_gpios[1].pin = GPIO2_PIN;
	ext_gpios[1].mode = GPIO2_MODE;
	ext_gpios[1].descr = GPIO2_DESCR;
        gpio_init(ext_gpios[1].periph, GPIO_SPEED, ext_gpios[1].port, ext_gpios[1].pin, ext_gpios[1].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO2_IRQ_LINE
		ext_gpios[1].irq_port_src = GPIO2_IRQ_PORT_SRC;
		ext_gpios[1].irq_pin_src = GPIO2_IRQ_PIN_SRC;
		ext_gpios[1].irq_line = GPIO2_IRQ_LINE;
		ext_gpios[1].irq_chan = GPIO2_IRQ_CHAN;
		ext_gpios[1].irq_raisfall = GPIO2_IRQ_RAISFALL;
		gpio_irq(ext_gpios[1].irq_port_src, ext_gpios[1].irq_pin_src, ext_gpios[1].irq_line, ext_gpios[1].irq_chan, ext_gpios[1].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO3_PORT
	ext_gpios[2].periph = GPIO3_PERIPH;
	ext_gpios[2].port = GPIO3_PORT;
        ext_gpios[2].pin = GPIO3_PIN;
	ext_gpios[2].mode = GPIO3_MODE;
	ext_gpios[2].descr = GPIO3_DESCR;
        gpio_init(ext_gpios[2].periph, GPIO_SPEED, ext_gpios[2].port, ext_gpios[2].pin, ext_gpios[2].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO3_IRQ_LINE
		ext_gpios[2].irq_port_src = GPIO3_IRQ_PORT_SRC;
		ext_gpios[2].irq_pin_src = GPIO3_IRQ_PIN_SRC;
		ext_gpios[2].irq_line = GPIO3_IRQ_LINE;
		ext_gpios[2].irq_chan = GPIO3_IRQ_CHAN;
		ext_gpios[2].irq_raisfall = GPIO3_IRQ_RAISFALL;
		gpio_irq(ext_gpios[2].irq_port_src, ext_gpios[2].irq_pin_src, ext_gpios[2].irq_line, ext_gpios[2].irq_chan, ext_gpios[2].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO4_PORT
	ext_gpios[3].periph = GPIO4_PERIPH;
	ext_gpios[3].port = GPIO4_PORT;
        ext_gpios[3].pin = GPIO4_PIN;
	ext_gpios[3].mode = GPIO4_MODE;
	ext_gpios[3].descr = GPIO4_DESCR;
        gpio_init(ext_gpios[3].periph, GPIO_SPEED, ext_gpios[3].port, ext_gpios[3].pin, ext_gpios[3].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO4_IRQ_LINE
		ext_gpios[3].irq_port_src = GPIO4_IRQ_PORT_SRC;
		ext_gpios[3].irq_pin_src = GPIO4_IRQ_PIN_SRC;
		ext_gpios[3].irq_line = GPIO4_IRQ_LINE;
		ext_gpios[3].irq_chan = GPIO4_IRQ_CHAN;
		ext_gpios[3].irq_raisfall = GPIO4_IRQ_RAISFALL;
		gpio_irq(ext_gpios[3].irq_port_src, ext_gpios[3].irq_pin_src, ext_gpios[3].irq_line, ext_gpios[3].irq_chan, ext_gpios[3].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO5_PORT
	ext_gpios[4].periph = GPIO5_PERIPH;
	ext_gpios[4].port = GPIO5_PORT;
        ext_gpios[4].pin = GPIO5_PIN;
	ext_gpios[4].mode = GPIO5_MODE;
	ext_gpios[4].descr = GPIO5_DESCR;
        gpio_init(ext_gpios[4].periph, GPIO_SPEED, ext_gpios[4].port, ext_gpios[4].pin, ext_gpios[4].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO5_IRQ_LINE
		ext_gpios[4].irq_port_src = GPIO5_IRQ_PORT_SRC;
		ext_gpios[4].irq_pin_src = GPIO5_IRQ_PIN_SRC;
		ext_gpios[4].irq_line = GPIO5_IRQ_LINE;
		ext_gpios[4].irq_chan = GPIO5_IRQ_CHAN;
		ext_gpios[4].irq_raisfall = GPIO5_IRQ_RAISFALL;
		gpio_irq(ext_gpios[4].irq_port_src, ext_gpios[4].irq_pin_src, ext_gpios[4].irq_line, ext_gpios[4].irq_chan, ext_gpios[4].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO6_PORT
	ext_gpios[5].periph = GPIO6_PERIPH;
	ext_gpios[5].port = GPIO6_PORT;
        ext_gpios[5].pin = GPIO6_PIN;
	ext_gpios[5].mode = GPIO6_MODE;
	ext_gpios[5].descr = GPIO6_DESCR;
        gpio_init(ext_gpios[5].periph, GPIO_SPEED, ext_gpios[5].port, ext_gpios[5].pin, ext_gpios[5].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO6_IRQ_LINE
		ext_gpios[5].irq_port_src = GPIO6_IRQ_PORT_SRC;
		ext_gpios[5].irq_pin_src = GPIO6_IRQ_PIN_SRC;
		ext_gpios[5].irq_line = GPIO6_IRQ_LINE;
		ext_gpios[5].irq_chan = GPIO6_IRQ_CHAN;
		ext_gpios[5].irq_raisfall = GPIO6_IRQ_RAISFALL;
		gpio_irq(ext_gpios[5].irq_port_src, ext_gpios[5].irq_pin_src, ext_gpios[5].irq_line, ext_gpios[5].irq_chan, ext_gpios[5].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO7_PORT
	ext_gpios[6].periph = GPIO7_PERIPH;
	ext_gpios[6].port = GPIO7_PORT;
        ext_gpios[6].pin = GPIO7_PIN;
	ext_gpios[6].mode = GPIO7_MODE;
	ext_gpios[6].descr = GPIO7_DESCR;
        gpio_init(ext_gpios[6].periph, GPIO_SPEED, ext_gpios[6].port, ext_gpios[6].pin, ext_gpios[6].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO7_IRQ_LINE
		ext_gpios[6].irq_port_src = GPIO7_IRQ_PORT_SRC;
		ext_gpios[6].irq_pin_src = GPIO7_IRQ_PIN_SRC;
		ext_gpios[6].irq_line = GPIO7_IRQ_LINE;
		ext_gpios[6].irq_chan = GPIO7_IRQ_CHAN;
		ext_gpios[6].irq_raisfall = GPIO7_IRQ_RAISFALL;
		gpio_irq(ext_gpios[6].irq_port_src, ext_gpios[6].irq_pin_src, ext_gpios[6].irq_line, ext_gpios[6].irq_chan, ext_gpios[6].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO8_PORT
	ext_gpios[7].periph = GPIO8_PERIPH;
	ext_gpios[7].port = GPIO8_PORT;
        ext_gpios[7].pin = GPIO8_PIN;
	ext_gpios[7].mode = GPIO8_MODE;
	ext_gpios[7].descr = GPIO8_DESCR;
        gpio_init(ext_gpios[7].periph, GPIO_SPEED, ext_gpios[7].port, ext_gpios[7].pin, ext_gpios[7].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO8_IRQ_LINE
		ext_gpios[7].irq_port_src = GPIO8_IRQ_PORT_SRC;
		ext_gpios[7].irq_pin_src = GPIO8_IRQ_PIN_SRC;
		ext_gpios[7].irq_line = GPIO8_IRQ_LINE;
		ext_gpios[7].irq_chan = GPIO8_IRQ_CHAN;
		ext_gpios[7].irq_raisfall = GPIO8_IRQ_RAISFALL;
		gpio_irq(ext_gpios[7].irq_port_src, ext_gpios[7].irq_pin_src, ext_gpios[7].irq_line, ext_gpios[7].irq_chan, ext_gpios[7].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO9_PORT
	ext_gpios[8].periph = GPIO9_PERIPH;
	ext_gpios[8].port = GPIO9_PORT;
        ext_gpios[8].pin = GPIO9_PIN;
	ext_gpios[8].mode = GPIO9_MODE;
	ext_gpios[8].descr = GPIO9_DESCR;
        gpio_init(ext_gpios[8].periph, GPIO_SPEED, ext_gpios[8].port, ext_gpios[8].pin, ext_gpios[8].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO9_IRQ_LINE
		ext_gpios[8].irq_port_src = GPIO9_IRQ_PORT_SRC;
		ext_gpios[8].irq_pin_src = GPIO9_IRQ_PIN_SRC;
		ext_gpios[8].irq_line = GPIO9_IRQ_LINE;
		ext_gpios[8].irq_chan = GPIO9_IRQ_CHAN;
		ext_gpios[8].irq_raisfall = GPIO9_IRQ_RAISFALL;
		gpio_irq(ext_gpios[8].irq_port_src, ext_gpios[8].irq_pin_src, ext_gpios[8].irq_line, ext_gpios[8].irq_chan, ext_gpios[8].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO10_PORT
	ext_gpios[9].periph = GPIO10_PERIPH;
	ext_gpios[9].port = GPIO10_PORT;
        ext_gpios[9].pin = GPIO10_PIN;
	ext_gpios[9].mode = GPIO10_MODE;
	ext_gpios[9].descr = GPIO10_DESCR;
        gpio_init(ext_gpios[9].periph, GPIO_SPEED, ext_gpios[9].port, ext_gpios[9].pin, ext_gpios[9].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO10_IRQ_LINE
		ext_gpios[9].irq_port_src = GPIO10_IRQ_PORT_SRC;
		ext_gpios[9].irq_pin_src = GPIO10_IRQ_PIN_SRC;
		ext_gpios[9].irq_line = GPIO10_IRQ_LINE;
		ext_gpios[9].irq_chan = GPIO10_IRQ_CHAN;
		ext_gpios[9].irq_raisfall = GPIO10_IRQ_RAISFALL;
		gpio_irq(ext_gpios[9].irq_port_src, ext_gpios[9].irq_pin_src, ext_gpios[9].irq_line, ext_gpios[9].irq_chan, ext_gpios[9].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO11_PORT
	ext_gpios[10].periph = GPIO11_PERIPH;
	ext_gpios[10].port = GPIO11_PORT;
        ext_gpios[10].pin = GPIO11_PIN;
	ext_gpios[10].mode = GPIO11_MODE;
	ext_gpios[10].descr = GPIO11_DESCR;
        gpio_init(ext_gpios[10].periph, GPIO_SPEED, ext_gpios[10].port, ext_gpios[10].pin, ext_gpios[10].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO11_IRQ_LINE
		ext_gpios[10].irq_port_src = GPIO11_IRQ_PORT_SRC;
		ext_gpios[10].irq_pin_src = GPIO11_IRQ_PIN_SRC;
		ext_gpios[10].irq_line = GPIO11_IRQ_LINE;
		ext_gpios[10].irq_chan = GPIO11_IRQ_CHAN;
		ext_gpios[10].irq_raisfall = GPIO11_IRQ_RAISFALL;
		gpio_irq(ext_gpios[10].irq_port_src, ext_gpios[10].irq_pin_src, ext_gpios[10].irq_line, ext_gpios[10].irq_chan, ext_gpios[10].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO12_PORT
	ext_gpios[11].periph = GPIO12_PERIPH;
	ext_gpios[11].port = GPIO12_PORT;
        ext_gpios[11].pin = GPIO12_PIN;
	ext_gpios[11].mode = GPIO12_MODE;
	ext_gpios[11].descr = GPIO12_DESCR;
        gpio_init(ext_gpios[11].periph, GPIO_SPEED, ext_gpios[11].port, ext_gpios[11].pin, ext_gpios[11].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO12_IRQ_LINE
		ext_gpios[11].irq_port_src = GPIO12_IRQ_PORT_SRC;
		ext_gpios[11].irq_pin_src = GPIO12_IRQ_PIN_SRC;
		ext_gpios[11].irq_line = GPIO12_IRQ_LINE;
		ext_gpios[11].irq_chan = GPIO12_IRQ_CHAN;
		ext_gpios[11].irq_raisfall = GPIO12_IRQ_RAISFALL;
		gpio_irq(ext_gpios[11].irq_port_src, ext_gpios[11].irq_pin_src, ext_gpios[11].irq_line, ext_gpios[11].irq_chan, ext_gpios[11].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO13_PORT
	ext_gpios[12].periph = GPIO13_PERIPH;
	ext_gpios[12].port = GPIO13_PORT;
        ext_gpios[12].pin = GPIO13_PIN;
	ext_gpios[12].mode = GPIO13_MODE;
	ext_gpios[12].descr = GPIO13_DESCR;
        gpio_init(ext_gpios[12].periph, GPIO_SPEED, ext_gpios[12].port, ext_gpios[12].pin, ext_gpios[12].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO13_IRQ_LINE
		ext_gpios[12].irq_port_src = GPIO13_IRQ_PORT_SRC;
		ext_gpios[12].irq_pin_src = GPIO13_IRQ_PIN_SRC;
		ext_gpios[12].irq_line = GPIO13_IRQ_LINE;
		ext_gpios[12].irq_chan = GPIO13_IRQ_CHAN;
		ext_gpios[12].irq_raisfall = GPIO13_IRQ_RAISFALL;
		gpio_irq(ext_gpios[12].irq_port_src, ext_gpios[12].irq_pin_src, ext_gpios[12].irq_line, ext_gpios[12].irq_chan, ext_gpios[12].irq_raisfall);
		#endif
	#endif

	#ifdef GPIO14_PORT
	ext_gpios[13].periph = GPIO14_PERIPH;
	ext_gpios[13].port = GPIO14_PORT;
        ext_gpios[13].pin = GPIO14_PIN;
	ext_gpios[13].mode = GPIO14_MODE;
	ext_gpios[13].descr = GPIO14_DESCR;
        gpio_init(ext_gpios[13].periph, GPIO_SPEED, ext_gpios[13].port, ext_gpios[13].pin, ext_gpios[13].mode, GPIO_OType_PP, GPIO_PuPd_UP);
		#ifdef GPIO14_IRQ_LINE
		ext_gpios[13].irq_port_src = GPIO14_IRQ_PORT_SRC;
		ext_gpios[13].irq_pin_src = GPIO14_IRQ_PIN_SRC;
		ext_gpios[13].irq_line = GPIO14_IRQ_LINE;
		ext_gpios[13].irq_chan = GPIO14_IRQ_CHAN;
		ext_gpios[13].irq_raisfall = GPIO14_IRQ_RAISFALL;
		gpio_irq(ext_gpios[13].irq_port_src, ext_gpios[13].irq_pin_src, ext_gpios[13].irq_line, ext_gpios[13].irq_chan, ext_gpios[13].irq_raisfall);
		#endif
	#endif

	print("ext_gpio_init() Intialized %d GPIOs\r\n", GPIO_CHANNELS);

	return 0;
}


int ext_gpio_get(unsigned int gpio_num)
{

	if(gpio_num >= GPIO_CHANNELS)
		return -1;	// Wrong GPIO channel number

	if(ext_gpios[gpio_num].mode == GPIO_Mode_IN)
		return GPIO_ReadInputDataBit(ext_gpios[gpio_num].port, ext_gpios[gpio_num].pin);
	else if(ext_gpios[gpio_num].mode == GPIO_Mode_OUT)
		return GPIO_ReadOutputDataBit(ext_gpios[gpio_num].port, ext_gpios[gpio_num].pin);
	else return -2; // Port is neither input nor output!

}


int ext_gpio_set(unsigned int gpio_num, int val)
{

	if(gpio_num >= GPIO_CHANNELS)
		return -1;	// Wrong GPIO channel number

	gpio_set(ext_gpios[gpio_num].port, ext_gpios[gpio_num].pin, val);

	return val;
}


int ext_gpio_irq(unsigned int gpio_num, int onoff)
{
	if(gpio_num >= GPIO_CHANNELS)
		return -1;	// Wrong GPIO channel number

	if(onoff) {
		gpio_irq(ext_gpios[gpio_num].irq_port_src, ext_gpios[gpio_num].irq_pin_src, ext_gpios[gpio_num].irq_line, ext_gpios[gpio_num].irq_chan, ext_gpios[gpio_num].irq_raisfall);
	} else {
		gpio_irq_disable(ext_gpios[gpio_num].irq_port_src, ext_gpios[gpio_num].irq_pin_src, ext_gpios[gpio_num].irq_line, ext_gpios[gpio_num].irq_chan);
	}

	return 0;
}



  
#endif
           

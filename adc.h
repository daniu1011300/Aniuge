#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#define CONV_NUMBER     (50)
#define CHANNEL_NUMBER  (16)


void adc_rx_proc( void (*cb)(void *, int len) );
void Voltage_ADC_Config(void);

#endif

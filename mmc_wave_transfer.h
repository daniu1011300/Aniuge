#ifndef _MMC_WAVE_TRANSFER_H
#define _MMC_WAVE_TRANSFER_H

#include <stdint.h>


#define PIN_OUT_NUM   (5)



int generate_spwm_table(int nout);
void clk_output_init(void);
void SIN_Test(uint8_t K);



#endif

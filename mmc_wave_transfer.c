#include "stm32f10x.h"
#include <math.h>
#include "mmc_wave_transfer.h"

#define DECIMATION   (30000*10)   //300khz
#define BASE_FREQ     (600)       //600hz
#define SIN_FREQ      (50)        //50hz
#define  PIN_VAL_SIZE  (DECIMATION/SIN_FREQ)
uint32_t PIN_VAL[PIN_VAL_SIZE] = {0};

float SIN_FLAG=0.99;
uint32_t PIN_IO[PIN_OUT_NUM*2] = { GPIO_Pin_6,  GPIO_Pin_7,  GPIO_Pin_8,  GPIO_Pin_9,   GPIO_Pin_10,
                                   GPIO_Pin_11, GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14,  GPIO_Pin_15};


void SIN_Test(uint8_t K){
	SIN_FLAG=(K*0.3+0.09);
}	
														   

void clk_output_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = 0x0000;
    for(int i=0; i<PIN_OUT_NUM*2; i++) {
        GPIO_InitStructure.GPIO_Pin |= PIN_IO[i];
    }
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    

    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit( DMA1_Channel2 );
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(GPIOB->BSRR);

    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)PIN_VAL;

    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = PIN_VAL_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init( DMA1_Channel2 , &DMA_InitStructure);
    DMA_Cmd( DMA1_Channel2 , ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Prescaler = 1200/10 - 1;
    TIM_TimeBaseStructure.TIM_Period = (60000*10/DECIMATION) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM3, ENABLE);
    TIM_DMACmd(TIM3, TIM_DMA_CC3, ENABLE);
}


#define DEAD_TIME   1

#define CONST_PI  (3.14159)
uint32_t generate_mmc_output(uint16_t nout, float second)
{
    float freq = SIN_FREQ;
    float base_freq = BASE_FREQ;
    static uint8_t last_out_level[PIN_OUT_NUM] = {0};
    
    uint32_t output_value = 0;
    for(int i=0; i<nout; i++) {
        /* sawtooth ware */
        //phase
        float sawtooth = (2*CONST_PI)*i/(nout);
        //tv
        sawtooth = 2*CONST_PI*second*base_freq + sawtooth;
        //mod 2*pi
        sawtooth = fmod(sawtooth, 2*CONST_PI);
        if(sawtooth < CONST_PI) {
            sawtooth = sawtooth*2/CONST_PI-1;
        } else {
            sawtooth = sawtooth*-2/CONST_PI+3;
        }
        
        /* sin ware */
        float sinwave = sin(2*CONST_PI*second*freq);
        uint8_t out_level = ( SIN_FLAG * sinwave > sawtooth) ? 1 : 0;
        uint8_t cur_out_level = out_level;
        
#if DEAD_TIME
        if(out_level - last_out_level[i] > 0) {
            cur_out_level = 0; //delay unit
        }
#endif
        if(cur_out_level) {
            output_value |= (PIN_IO[i]); //Set bit
        } else {
            output_value |= (PIN_IO[i])<<16; //Reset bit
        }
        
#if DEAD_TIME
        uint8_t cur_n_out_level = !out_level;
        if(cur_n_out_level - !last_out_level[i] > 0) {
            cur_n_out_level = 0; //delay unit
        }
        if(cur_n_out_level) {
            output_value |= (PIN_IO[nout + i]); //Set bit
        } else {
            output_value |= (PIN_IO[nout + i])<<16; //Reset bit
        }
#endif
        
        last_out_level[i] = out_level;
    }
    return output_value;
}


int generate_spwm_table(int nout)
{
	
    for(int i=0; i<PIN_VAL_SIZE; i++) {
        PIN_VAL[i] = generate_mmc_output(nout, i*1.0/DECIMATION);
    }
    return 0;
}

#ifdef _MMC_TEST_DEF

int main(void)
{
    generate_spwm_table(PIN_OUT_NUM);
    clk_output_init();
    while(1);
}

#endif

#include "adc.h"
#include <stdio.h>
#include <string.h>


uint16_t ADC_ConvertedValue[CONV_NUMBER*CHANNEL_NUMBER];
__IO uint16_t DMA_FLAG = 0;

struct sAdc
{
    uint8_t ADC_Channel_x;
    uint16_t GPIO_Pin;
    GPIO_TypeDef* GPIOx;
};

struct sAdc ADC_PARAM[CHANNEL_NUMBER] = {
    {
        .ADC_Channel_x = ADC_Channel_0,
        .GPIO_Pin      = GPIO_Pin_0,
        .GPIOx         = GPIOA,
    },
    {
        .ADC_Channel_x = ADC_Channel_1,
        .GPIO_Pin      = GPIO_Pin_1,
        .GPIOx         = GPIOA,
    },
    {
        .ADC_Channel_x = ADC_Channel_2,
        .GPIO_Pin      = GPIO_Pin_2,
        .GPIOx         = GPIOA,
    },
    {
        .ADC_Channel_x = ADC_Channel_3,
        .GPIO_Pin      = GPIO_Pin_3,
        .GPIOx         = GPIOA,
    },
    
    {
        .ADC_Channel_x = ADC_Channel_4,
        .GPIO_Pin      = GPIO_Pin_4,
        .GPIOx         = GPIOA,
    },
    {
        .ADC_Channel_x = ADC_Channel_5,
        .GPIO_Pin      = GPIO_Pin_5,
        .GPIOx         = GPIOA,
    },
    {
        .ADC_Channel_x = ADC_Channel_6,
        .GPIO_Pin      = GPIO_Pin_6,
        .GPIOx         = GPIOA,
    },
    {
        .ADC_Channel_x = ADC_Channel_7,
        .GPIO_Pin      = GPIO_Pin_7,
        .GPIOx         = GPIOA,
    },
    
//    {
//        .ADC_Channel_x = ADC_Channel_8,
//        .GPIO_Pin      = GPIO_Pin_0,
//        .GPIOx         = GPIOB,
//    },
    {
        .ADC_Channel_x = ADC_Channel_9,
        .GPIO_Pin      = GPIO_Pin_1,
        .GPIOx         = GPIOB,
    },
    {
        .ADC_Channel_x = ADC_Channel_10,
        .GPIO_Pin      = GPIO_Pin_0,
        .GPIOx         = GPIOC,
    },
    {
        .ADC_Channel_x = ADC_Channel_11,
        .GPIO_Pin      = GPIO_Pin_1,
        .GPIOx         = GPIOC,
    },
    
    {
        .ADC_Channel_x = ADC_Channel_12,
        .GPIO_Pin      = GPIO_Pin_2,
        .GPIOx         = GPIOC,
    },
    {
        .ADC_Channel_x = ADC_Channel_13,
        .GPIO_Pin      = GPIO_Pin_3,
        .GPIOx         = GPIOC,
    },
    {
        .ADC_Channel_x = ADC_Channel_14,
        .GPIO_Pin      = GPIO_Pin_4,
        .GPIOx         = GPIOC,
    },
    {
        .ADC_Channel_x = ADC_Channel_15,
        .GPIO_Pin      = GPIO_Pin_5,
        .GPIOx         = GPIOC,
    },
};


void Voltage_ADC_Config(void)
{
    DMA_InitTypeDef            DMA_InitStructure;
    ADC_InitTypeDef            ADC_InitStructure;
    GPIO_InitTypeDef           GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
    TIM_OCInitTypeDef          TIM_OCInitStructure;
    NVIC_InitTypeDef           NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //............................................................
    //Config.........
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    for(int i=0; i<CHANNEL_NUMBER; i++) {
        GPIO_InitStructure.GPIO_Pin = ADC_PARAM[i].GPIO_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(ADC_PARAM[i].GPIOx, &GPIO_InitStructure);
    }
    
    
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    //每秒触发500次ADC转换，每20us中断一次
    //每50个计算一次平均值，用PID算法改变PWM值
    TIM_TimeBaseStructure.TIM_Period = 40-1; //25kHz, 40us
    TIM_TimeBaseStructure.TIM_Prescaler = 71-1;   //1MHz, 1us
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    //计数器模式
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 10;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
    TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_OC4Ref);

    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR));
    // 存储器地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValue[0];
    // 数据源来自外设
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = CONV_NUMBER*CHANNEL_NUMBER;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    // DMA 传输通道优先级为高
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADC_InitStructure.ADC_Mode                = ADC_Mode_Independent;
    // 禁止扫描模式，多通道才要
    ADC_InitStructure.ADC_ScanConvMode        = ENABLE;
    // 连续转换模式,定时器触发不能开启
    ADC_InitStructure.ADC_ContinuousConvMode  = DISABLE;
    // TIM4触发转换
    ADC_InitStructure.ADC_ExternalTrigConv    = ADC_ExternalTrigConv_T4_CC4;
    ADC_InitStructure.ADC_DataAlign           = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel        = CHANNEL_NUMBER;
    ADC_Init(ADC1, &ADC_InitStructure);

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    //...........................................
    //Config...
    for(unsigned char i=0; i<CHANNEL_NUMBER; i++) {
        ADC_RegularChannelConfig(ADC1, ADC_PARAM[i].ADC_Channel_x , i + 1, ADC_SampleTime_55Cycles5);
    }
    
    ADC_DMACmd(ADC1, ENABLE);
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
    TIM_Cmd(TIM4, ENABLE);
    //TIM_CtrlPWMOutputs(TIM1, ENABLE);  //TIM1 & TIM8 only
}


void adc_rx_proc( void (*func_cb)(void *, int len) )
{
    if(DMA_FLAG) {
        func_cb(ADC_ConvertedValue, sizeof(ADC_ConvertedValue));
        DMA_FLAG = 0;
        TIM_Cmd(TIM4, ENABLE);
    }
}


void DMA1_Channel1_IRQHandler(void)
{
    if( DMA_GetITStatus(DMA1_IT_TC1) == SET){
        DMA_ClearITPendingBit(DMA1_IT_GL1);
        TIM_Cmd(TIM4, DISABLE);
        DMA_FLAG = 1;
    }
}


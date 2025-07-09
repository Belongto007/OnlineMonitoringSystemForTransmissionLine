#include "adc.h"
#include "delay.h"

ADC_HandleTypeDef ADC1_Handler;//ADC句柄

DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim1;

u8 g_adc_ready  = 0;
/**
 * @brief	初始化ADC函数
 *
 * @param   void
 *
 * @return  void
 */
void MY_ADC_Init(void)
{
	HAL_ADC_MspInit(&ADC1_Handler);
    ADC1_Handler.Instance = ADC1;
    ADC1_Handler.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4; //2分频，ADCCLK=SYSCLK/2=80/2=40MHZ	
    ADC1_Handler.Init.Resolution = ADC_RESOLUTION_12B;           //12位模式
    ADC1_Handler.Init.DataAlign = ADC_DATAALIGN_RIGHT;           //右对齐
    ADC1_Handler.Init.ScanConvMode = ENABLE;                    //非扫描模式
    ADC1_Handler.Init.EOCSelection = ADC_EOC_SEQ_CONV;                    //关闭EOC中断
    ADC1_Handler.Init.ContinuousConvMode = DISABLE;              //关闭连续转换
    ADC1_Handler.Init.NbrOfConversion = 2;                       //1个转换在规则序列中 也就是只转换规则序列1
    ADC1_Handler.Init.DiscontinuousConvMode = DISABLE;           //禁止不连续采样模式
    ADC1_Handler.Init.NbrOfDiscConversion = 0;                   //不连续采样通道数为0
    ADC1_Handler.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO;     //软件触发
    ADC1_Handler.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING; //使用软件触发

    ADC1_Handler.Init.LowPowerAutoWait = DISABLE;
    ADC1_Handler.Init.DMAContinuousRequests = ENABLE;
    ADC1_Handler.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    ADC1_Handler.Init.OversamplingMode = DISABLE;

    ADC1_Handler.Init.DMAContinuousRequests = DISABLE;           //关闭DMA请求
    HAL_ADC_Init(&ADC1_Handler);                                 //初始化
	
	HAL_Delay(1);	//初始化
	
	if(HAL_ADCEx_Calibration_Start(&ADC1_Handler, ADC_SINGLE_ENDED) != HAL_OK) {
    printf("ADC Calibration Failed!\n");
    Error_Handler();
  }
	ADC_ChannelConfTypeDef ADC1_ChanConf;
  
	ADC1_ChanConf.Channel = ADC_CHANNEL_5;                                 //通道
    ADC1_ChanConf.Rank = ADC_REGULAR_RANK_1;										//第1个序列，序列1
    ADC1_ChanConf.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;    //采样时间
    ADC1_ChanConf.SingleDiff = ADC_SINGLE_ENDED;								//ADC单端输入
    ADC1_ChanConf.OffsetNumber = ADC_OFFSET_NONE;								//偏移号选择
    ADC1_ChanConf.Offset = 0;
    HAL_ADC_ConfigChannel(&ADC1_Handler, &ADC1_ChanConf);       //通道配置  
  
    ADC1_ChanConf.Channel = ADC_CHANNEL_6;                                 //通道
    ADC1_ChanConf.Rank = ADC_REGULAR_RANK_2;										//第1个序列，序列1
    ADC1_ChanConf.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;    //采样时间
    ADC1_ChanConf.SingleDiff = ADC_SINGLE_ENDED;								//ADC单端输入
    ADC1_ChanConf.OffsetNumber = ADC_OFFSET_NONE;								//偏移号选择
    ADC1_ChanConf.Offset = 0;
    HAL_ADC_ConfigChannel(&ADC1_Handler, &ADC1_ChanConf);       //通道配置

	
//	uint32_t adc_clock = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_ADC);
//		printf("ADC Clock: %lu Hz\r\n", adc_clock);
}
/**
 * @brief	ADC底层驱动，引脚配置，时钟使能，此函数会被HAL_ADC_Init()调用
 *
 * @param   hadc	ADC句柄
 *
 * @return  void
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    GPIO_InitTypeDef GPIO_Initure;

    //选择ADC时钟源为SYSCLK(80Mhz)
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    __HAL_RCC_ADC_CLK_ENABLE();            //使能ADC1时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();					//开启GPIOC时钟

    GPIO_Initure.Pin =GPIO_PIN_0 | GPIO_PIN_1 ;          	//PA
    GPIO_Initure.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;   							//模拟
    GPIO_Initure.Pull = GPIO_NOPULL;        													//不带上下拉
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
	
	/* ADC1 DMA Init */
	/* ADC1 Init */
	hdma_adc1.Instance = DMA1_Channel1;
	hdma_adc1.Init.Request = DMA_REQUEST_0;
	hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
	hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma_adc1.Init.Mode = DMA_NORMAL;
	hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
	if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
	{
	  Error_Handler();
	}

	__HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);
}
/**
 * @brief	获得ADC值
 *
 * @param   ch		通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
 *
 * @return  u16		转换结果
 */
u16 Get_Adc(u32 ch)
{
    ADC_ChannelConfTypeDef ADC1_ChanConf;

    ADC1_ChanConf.Channel = ch;                                 //通道
    ADC1_ChanConf.Rank = ADC_REGULAR_RANK_1;										//第1个序列，序列1
    ADC1_ChanConf.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;    //采样时间
    ADC1_ChanConf.SingleDiff = ADC_SINGLE_ENDED;								//ADC单端输入
    ADC1_ChanConf.OffsetNumber = ADC_OFFSET_NONE;								//偏移号选择
    ADC1_ChanConf.Offset = 0;
    HAL_ADC_ConfigChannel(&ADC1_Handler, &ADC1_ChanConf);       //通道配置

    HAL_ADC_Start(&ADC1_Handler);                               //开启ADC

    HAL_ADC_PollForConversion(&ADC1_Handler, 10);               //轮询转换

    return (u16)HAL_ADC_GetValue(&ADC1_Handler);	        			//返回最近一次ADC1规则组的转换结果
}

/**
 * @brief	获取指定通道的转换值，取times次,然后平均
 *
 * @param   ch		通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
 * @param   times	获取次数
 *
 * @return  u16		通道ch的times次转换结果平均值
 */
u16 Get_Adc_Average(u32 ch, u8 times)
{
    u32 temp_val = 0;
    u8 t;

    for(t = 0; t < times; t++)
    {
        temp_val += Get_Adc(ch);
        delay_ms(1);
    }

    return temp_val / times;
}

// 在adc.c中添加回调函数
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if(hadc->Instance == ADC1) {
        HAL_TIM_Base_Stop(&htim1);     // 停止定时器触发
        HAL_ADC_Stop_DMA(&ADC1_Handler); // 停止ADC&DMA
        // 设置标志位通知主程序处理数据
        g_adc_ready = 1;
//		printf("Sampling stopped\r\n");
    }
}


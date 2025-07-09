#include "spi2.h"
#include "delay.h"
#include "gpio.h"
SPI_HandleTypeDef SPI2_Handler;  //SPI2句柄
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;

/**
 * @brief	SPI2初始化代码，配置成主机模式
 *
 * @param   void
 *
 * @return  void
 */
 void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
		
  }
  /* USER CODE END Error_Handler_Debug */
}
void SPI2_Init(void)
{
	  
    SPI2_Handler.Instance=SPI2;                         //SPI2
    SPI2_Handler.Init.Mode=SPI_MODE_MASTER;             //设置SPI工作模式，设置为主模式
    SPI2_Handler.Init.Direction=SPI_DIRECTION_2LINES;   //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    SPI2_Handler.Init.DataSize=SPI_DATASIZE_8BIT;       //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI2_Handler.Init.CLKPolarity=SPI_POLARITY_LOW;     //串行同步时钟的空闲状态为高电平
    SPI2_Handler.Init.CLKPhase=SPI_PHASE_1EDGE;         //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI2_Handler.Init.NSS=SPI_NSS_SOFT;                 //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI2_Handler.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_256;//SPI_BAUDRATEPRESCALER_2;//定义波特率预分频的值:波特率预分频值为256
    SPI2_Handler.Init.FirstBit=SPI_FIRSTBIT_MSB;        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI2_Handler.Init.TIMode=SPI_TIMODE_DISABLE;        //关闭TI模式
    SPI2_Handler.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    SPI2_Handler.Init.CRCPolynomial=7;                  //CRC值计算的多项式
	  SPI2_Handler.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
		SPI2_Handler.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	  HAL_SPI_MspDeInit(&SPI2_Handler);
		HAL_SPI_MspInit(&SPI2_Handler);
    HAL_SPI_Init(&SPI2_Handler);												//初始化SPI2
    __HAL_SPI_ENABLE(&SPI2_Handler);                  //使能SPI2
		__HAL_DMA_ENABLE(&hdma_spi2_rx);
		__HAL_DMA_ENABLE(&hdma_spi2_tx);
		
	
}

/**
 * @brief	SPI2底层驱动，时钟使能，引脚配置
 *
 * @param   hspi	SPI句柄
 *
 * @return  void
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure= {0};
    
    __HAL_RCC_GPIOB_CLK_ENABLE();       //使能GPIOB时钟
    __HAL_RCC_SPI2_CLK_ENABLE();        //使能SPI2时钟

    //PB13.14.15
    GPIO_Initure.Pin=GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //复用推挽输出
    GPIO_Initure.Pull=GPIO_NOPULL;                  //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;   		//         
    GPIO_Initure.Alternate=GPIO_AF5_SPI2;           //PB12.PB15复用为SPI2
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
    /* SPI2 DMA Init */
    /* SPI2_RX Init */
    hdma_spi2_rx.Instance = DMA1_Channel4;
    hdma_spi2_rx.Init.Request = DMA_REQUEST_1;
    hdma_spi2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_rx.Init.Mode = DMA_NORMAL;
    hdma_spi2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_spi2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hspi,hdmarx,hdma_spi2_rx);

    /* SPI2_TX Init */
    hdma_spi2_tx.Instance = DMA1_Channel5;
    hdma_spi2_tx.Init.Request = DMA_REQUEST_1;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hspi,hdmatx,hdma_spi2_tx);

    /* SPI2 interrupt Init */
   // HAL_NVIC_SetPriority(SPI2_IRQn, 1, 2);
 //   HAL_NVIC_EnableIRQ(SPI2_IRQn);
}
/**
 * @brief	SPI2解绑
 *
 * @param   hspi	SPI句柄
 *
 * @return  void
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    /* SPI2 DMA DeInit */
    HAL_DMA_DeInit(spiHandle->hdmarx);
    HAL_DMA_DeInit(spiHandle->hdmatx);

    /* SPI2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/**
 * @brief	SPI2 读写一个字节
 *
 * @param   TxData	要写入的字节
 *
 * @return  u8		读取到的字节
 */
u8 SPI2_ReadWriteByte(u8 TxData)
{
  u8 Rxdata;
	HAL_SPI_TransmitReceive_DMA(&SPI2_Handler,&TxData,&Rxdata,1);    
 	return Rxdata;          		    //返回收到的数据		
}




//typedef enum
//{
//  HAL_SPI_STATE_RESET      = 0x00U,    /*!< Peripheral not Initialized                         */
//  HAL_SPI_STATE_READY      = 0x01U,    /*!< Peripheral Initialized and ready for use           */
//  HAL_SPI_STATE_BUSY       = 0x02U,    /*!< an internal process is ongoing                     */
//  HAL_SPI_STATE_BUSY_TX    = 0x03U,    /*!< Data Transmission process is ongoing               */
//  HAL_SPI_STATE_BUSY_RX    = 0x04U,    /*!< Data Reception process is ongoing                  */
//  HAL_SPI_STATE_BUSY_TX_RX = 0x05U,    /*!< Data Transmission and Reception process is ongoing */
//  HAL_SPI_STATE_ERROR      = 0x06U,    /*!< SPI error state                                    */
//  HAL_SPI_STATE_ABORT      = 0x07U     /*!< SPI abort is ongoing                               */
//} HAL_SPI_StateTypeDef;

/**
 * @brief	SPI2 写入一个字节
 *
 * @param   TxData	要写入的字节
 * @param   size	写入字节大小
 *
 * @return  u8		0:写入成功,其他:写入失败
 */
u8 SPI2_WriteByte(u8 *TxData,u16 size)
{ 
	u8  state;
	state=HAL_SPI_Transmit_DMA(&SPI2_Handler,TxData,size);
	return state;
}

HAL_SPI_StateTypeDef SPI2STATE;

uint8_t SpiInOut( uint8_t txBuffer)
{
	 /*
      while( SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);//当发送buffer为空时(说明上一次数据已复制到移位寄存器中)退出,这时可以往buffer里面写数据
      SPI_SendData8(SPI2, txBuffer);
    
      while( SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);//当接收buffer为非空时退出
      return SPI_ReceiveData8(SPI2);
   */
	
//	  SPI2STATE= HAL_SPI_GetState(&SPI2_Handler);
//	  while(HAL_SPI_GetState(&SPI2_Handler) == HAL_SPI_STATE_BUSY_TX )
//		;
		u8 Rxdata;
		HAL_SPI_TransmitReceive(&SPI2_Handler,&txBuffer,&Rxdata,1,1000);
		return Rxdata;          		    //返回收到的数据
}

void SpiIn( uint8_t *txBuffer, uint16_t size )
{
    uint16_t i;
		u8 Rxdata;
    /*
    for(i=0;i<size;i++)
    {
      while( SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);//当发送buffer为空时(说明上一次数据已复制到移位寄存器中)退出,这时可以往buffer里面写数据
      SPI_SendData8(SPI2, txBuffer[i]);
      
      while( SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);//当接收buffer为非空时退出
      SPI_ReceiveData8(SPI2);
    }
		*/
	  for(i=0;i<size;i++)
    {
			HAL_SPI_TransmitReceive_DMA(&SPI2_Handler,&txBuffer[i],&Rxdata,1); 
			delay_ms(1);
			
    }	
}













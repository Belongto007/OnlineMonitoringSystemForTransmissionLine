#include "usart.h"
#include "delay.h"
#include "tjc_usart_hmi.h"
#include <string.h>
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};

FILE __stdout;
/**
 * @brief	定义_sys_exit()以避免使用半主机模式
 *
 * @param	void
 *
 * @return  void
 */
void _sys_exit(int x)
{
    x = x;
}
/**
 * @brief	重定义fputc函数
 *
 * @param	ch		输出字符量
 * @param	f		文件指针
 *
 * @return  void
 */
int fputc(int ch, FILE *f)
{
	//屏幕
    while((UART4->ISR & 0X40) == 0); //循环发送,直到发送完毕
	UART4->TDR = (u8) ch;
	//串口调试
//	while((USART1->ISR & 0X40) == 0); //循环发送,直到发送完毕
//	USART1->TDR = (u8) ch;
    
    return ch;

}
#endif


#if EN_UART4_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0;     //接收状态标记
u8 rxData;
UART_HandleTypeDef UART3_Handler; //UART句柄
UART_HandleTypeDef UART1_Handler;
UART_HandleTypeDef UART4_Handler;
/**
 * @brief	初始化串口1函数
 *
 * @param	bound	串口波特率
 *
 * @return  void
 */
void uart_init(u32 bound1,u32 bound3)
{
    //UART 初始化设置
    UART3_Handler.Instance = USART3;					  					//USART3
    UART3_Handler.Init.BaudRate = bound3;				  				//波特率
    UART3_Handler.Init.WordLength = UART_WORDLENGTH_8B; 	//字长为8位数据格式
    UART3_Handler.Init.StopBits = UART_STOPBITS_1;	  		//一个停止位
    UART3_Handler.Init.Parity = UART_PARITY_NONE;		  		//无奇偶校验位
    UART3_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; 	//无硬件流控
    UART3_Handler.Init.Mode = UART_MODE_TX_RX;		  			//收发模式
    HAL_UART_Init(&UART3_Handler);					    					//HAL_UART_Init()会使能UART1

    __HAL_UART_ENABLE_IT(&UART3_Handler, UART_IT_RXNE); 	//开启接收中断
    HAL_NVIC_EnableIRQ(USART3_IRQn);											//使能USART1中断通道
    HAL_NVIC_SetPriority(USART3_IRQn, 2, 2);							//抢占优先级3，子优先级3
	
    //UART 初始化设置
    UART1_Handler.Instance = USART1;					  					//USART1
	  UART1_Handler.Init.BaudRate = bound1;				  				//波特率
    UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B; 	//字长为8位数据格式
    UART1_Handler.Init.StopBits = UART_STOPBITS_1;	  		//一个停止位
    UART1_Handler.Init.Parity = UART_PARITY_NONE;		  		//无奇偶校验位
    UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; 	//无硬件流控
    UART1_Handler.Init.Mode = UART_MODE_TX_RX;		  			//收发模式
    HAL_UART_Init(&UART1_Handler);					    					//HAL_UART_Init()会使能UART1

    __HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_RXNE); 	//开启接收中断
    HAL_NVIC_EnableIRQ(USART1_IRQn);											//使能USART1中断通道
    HAL_NVIC_SetPriority(USART1_IRQn, 2, 1);							//抢占优先级3，子优先级3
		
		
    //UART 初始化设置
    UART4_Handler.Instance = UART4;					  					//USART4
    UART4_Handler.Init.BaudRate = 19200;				  				//波特率
    UART4_Handler.Init.WordLength = UART_WORDLENGTH_8B; 	//字长为8位数据格式
    UART4_Handler.Init.StopBits = UART_STOPBITS_1;	  		//一个停止位
    UART4_Handler.Init.Parity = UART_PARITY_NONE;		  		//无奇偶校验位
    UART4_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; 	//无硬件流控
    UART4_Handler.Init.Mode = UART_MODE_TX_RX;		  			//收发模式
    HAL_UART_Init(&UART4_Handler);					    					//HAL_UART_Init()会使能UART1

    __HAL_UART_ENABLE_IT(&UART4_Handler, UART_IT_RXNE); 	//开启接收中断
    HAL_NVIC_EnableIRQ(UART4_IRQn);											//使能USART1中断通道
    HAL_NVIC_SetPriority(UART4_IRQn, 2, 3);							//抢占优先级3，子优先级3
		
	


}


/**
 * @brief	HAL库串口底层初始化，时钟使能，引脚配置，中断配置
 *
 * @param	huart	串口句柄
 *
 * @return  void
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
	
{
		initRingBuff();
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_Initure;

    if(huart->Instance == USART3) //如果是串口1，进行串口1 MSP初始化
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();				//使能GPIOA时钟
        __HAL_RCC_USART3_CLK_ENABLE();				//使能USART1时钟

        GPIO_Initure.Pin = GPIO_PIN_4;				//Pc4
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
        GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
        GPIO_Initure.Speed = GPIO_SPEED_FAST;		//高速
        GPIO_Initure.Alternate = GPIO_AF7_USART3;	//复用为USART3
        HAL_GPIO_Init(GPIOC, &GPIO_Initure);	   	//初始化Pc4

        GPIO_Initure.Pin = GPIO_PIN_5;				//Pc5
        HAL_GPIO_Init(GPIOC, &GPIO_Initure);	   	//初始化Pc5
    }
		    if(huart->Instance == USART1) //如果是串口1，进行串口1 MSP初始化
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();				//使能GPIOA时钟
        __HAL_RCC_USART1_CLK_ENABLE();				//使能USART1时钟

        GPIO_Initure.Pin = GPIO_PIN_9;				//Pa8
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
        GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
        GPIO_Initure.Speed = GPIO_SPEED_FAST;		//高速
        GPIO_Initure.Alternate = GPIO_AF7_USART1;	//复用为USART3
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//初始化Pc4

        GPIO_Initure.Pin = GPIO_PIN_10;				//Pa9
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//初始化Pc5
    }
 if(huart->Instance == UART4) //如果是串口1，进行串口1 MSP初始化
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();				//使能GPIOC时钟
        __HAL_RCC_UART4_CLK_ENABLE();				//使能UART4时钟

        GPIO_Initure.Pin = GPIO_PIN_10;				//Pc10
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
        GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
        GPIO_Initure.Speed = GPIO_SPEED_FAST;		//高速
        GPIO_Initure.Alternate = GPIO_AF8_UART4;	//复用为USART4
        HAL_GPIO_Init(GPIOC, &GPIO_Initure);	   	//初始化Pc4

        GPIO_Initure.Pin = GPIO_PIN_11;				//Pc11
        HAL_GPIO_Init(GPIOC, &GPIO_Initure);	   	//初始化Pc5
    }
}


/**
 * @brief	串口1中断服务程序
 *
 * @remark	下面代码我们直接把中断控制逻辑写在中断服务函数内部
 * 			说明：采用HAL库处理逻辑，效率不高。
 *
 * @param   void
 *
 * @return  void
 */
void USART3_IRQHandler(void)
{
    u8 Res;

    if((__HAL_UART_GET_FLAG(&UART3_Handler, UART_FLAG_RXNE) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)]
			
    {
        HAL_UART_Receive(&UART3_Handler, &Res, 1, 1000);

        if((USART_RX_STA & 0x8000) == 0) //接收未完成
        {
            if(USART_RX_STA & 0x4000) //接收到了0x0d
            {
                if(Res != 0x0a)USART_RX_STA = 0; //接收错误,重新开始

                else USART_RX_STA |= 0x8000;	//接收完成了
            }
            else //还没收到0X0D
            {
                if(Res == 0x0d)USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res ;
                    USART_RX_STA++;

                    if(USART_RX_STA > (USART_REC_LEN - 1))USART_RX_STA = 0; //接收数据错误,重新开始接收
                }
            }
        }
    }
    HAL_UART_IRQHandler(&UART3_Handler);
}

void USART1_IRQHandler(void)
{
	 HAL_UART_IRQHandler(&UART1_Handler);		
}

void UART4_IRQHandler(void)
{
	 HAL_UART_IRQHandler(&UART4_Handler);		
}

//	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{

//		  if(rxData[0] == 0x65 && rxData[4] == 0xff && rxData[5] == 0xff && rxData[6] == 0xff)
//	  {
//		  TJCPrintf("msg.txt=\"led %d is %s\"", u(1), u(2) ? "on" : "off");
//	
//	  }		
//	memset(rxData, 0, sizeof(rxData));
//		  for (int i = 0; i < 16; i++) {
//        printf("%d ", rxData[i]);
//    }
//        HAL_UART_Receive_IT(&UART1_Handler, &rxData[0], 7);

//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	u8	tjcstate1=tjcstate;
	//writeRingBuff(rxData[0]);
	switch(rxData)
	{
		case 0x70:
			tjcstate=1;
			break;			
		case 0x71:
			tjcstate=2;
			break;		
		case 0x88:
			tjcstate=0;
			break;
		default:
			tjcstate=0;
			break;
	}
//	printf("rxData = %x \r\n",rxData);
//	printf("tjcstate = %d \r\n",tjcstate);
	screenstate(tjcstate);
	HAL_UART_Receive_IT(&UART4_Handler, &rxData, 1);
}		
//void USART1_IRQHandler(void)
//{
//    u8 Res;

//    if((__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_RXNE) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)]
//			
//    {
//        HAL_UART_Receive(&UART1_Handler, &Res, 1, 1000);

//        if((USART_RX_STA & 0x8000) == 0) //接收未完成
//        {
//            if(USART_RX_STA & 0x4000) //接收到了0x0d
//            {
//                if(Res != 0x0a)USART_RX_STA = 0; //接收错误,重新开始

//                else USART_RX_STA |= 0x8000;	//接收完成了
//            }
//            else //还没收到0X0D
//            {
//                if(Res == 0x0d)USART_RX_STA |= 0x4000;
//                else
//                {
//                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res ;
//                    USART_RX_STA++;

//                    if(USART_RX_STA > (USART_REC_LEN - 1))USART_RX_STA = 0; //接收数据错误,重新开始接收
//                }
//            }
//        }
//    }
//    HAL_UART_IRQHandler(&UART1_Handler);
//}

#endif







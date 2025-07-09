#include "usart.h"
#include "delay.h"
#include <stdint.h>
#include <stdbool.h>
char rxdatabufer;
u16 point1 = 0;
uint8_t command1[] = {0x5A, 0x5A, 0x06, 0x0B, 0x00, 0x09, 0x00, 0xCE};
uint8_t command2[] = {0x5A, 0x5A, 0x06, 0x0C, 0x00, 0x03, 0x00, 0xC9};
uint8_t command3[] = {0x5A, 0x5A, 0x06, 0x0E, 0x00, 0x02, 0x00, 0xCA};
_SaveData Save_Data;
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
    while((USART3->ISR & 0X40) == 0); //循环发送,直到发送完毕

    USART3->TDR = (u8) ch;
    return ch;
}
#endif


#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0;     //接收状态标记

UART_HandleTypeDef UART3_Handler; //UART句柄


/**
 * @brief	初始化串口1函数
 *
 * @param	bound	串口波特率
 *
 * @return  void
 */
void uart_initT(u32 bound)
{
    //UART 初始化设置
    UART3_Handler.Instance = USART3;					  					//USART1
    UART3_Handler.Init.BaudRate = bound;				  				//波特率
    UART3_Handler.Init.WordLength = UART_WORDLENGTH_8B; 	//字长为8位数据格式
    UART3_Handler.Init.StopBits = UART_STOPBITS_1;	  		//一个停止位
    UART3_Handler.Init.Parity = UART_PARITY_NONE;		  		//无奇偶校验位
    UART3_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; 	//无硬件流控
    UART3_Handler.Init.Mode = UART_MODE_TX;		  			//收发模式
    HAL_UART_Init(&UART3_Handler);					    					//HAL_UART_Init()会使能UART1

 //  __HAL_UART_ENABLE_IT(&UART3_Handler, UART_IT_RXNE); 	//开启接收中断
    HAL_NVIC_EnableIRQ(USART3_IRQn);											//使能USART1中断通道
  //  HAL_NVIC_SetPriority(USART3_IRQn, 3, 3);							//抢占优先级3，子优先级3
}

void uart_init(u32 bound)
{
    //UART 初始化设置
    UART3_Handler.Instance = USART3;					  					//USART1
    UART3_Handler.Init.BaudRate = bound;				  				//波特率
    UART3_Handler.Init.WordLength = UART_WORDLENGTH_8B; 	//字长为8位数据格式
    UART3_Handler.Init.StopBits = UART_STOPBITS_1;	  		//一个停止位
    UART3_Handler.Init.Parity = UART_PARITY_NONE;		  		//无奇偶校验位
    UART3_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; 	//无硬件流控
    UART3_Handler.Init.Mode = UART_MODE_TX_RX;		  			//收发模式
    HAL_UART_Init(&UART3_Handler);					    					//HAL_UART_Init()会使能UART1

   __HAL_UART_ENABLE_IT(&UART3_Handler, UART_IT_RXNE); 	//开启接收中断
    HAL_NVIC_EnableIRQ(USART3_IRQn);											//使能USART1中断通道
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 3);							//抢占优先级3，子优先级3
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
	

	
    if((__HAL_UART_GET_FLAG(&UART3_Handler, UART_FLAG_RXNE) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
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

void send_command(uint8_t* command, uint8_t length) {
    HAL_UART_Transmit(&UART3_Handler, command, length,100);
}
void mlx90640_init()
{

//				send_command(command1, sizeof(command1));
//       delay_ms(100);

        send_command(command2, sizeof(command2));
        delay_ms(1000);
send_command(command2, sizeof(command2));
	    delay_ms(1000);
        send_command(command3, sizeof(command3));
        delay_ms(500);
	HAL_UART_AbortReceive(&UART3_Handler);
}

#endif







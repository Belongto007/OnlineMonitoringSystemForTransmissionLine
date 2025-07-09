/**
使用注意事项:
    1.将tjc_usart_hmi.c和tjc_usart_hmi.h 分别导入工程
    2.在需要使用的函数所在的头文件中添加 #include "tjc_usart_hmi.h"
    3.使用前请将 HAL_UART_Transmit_IT() 这个函数改为你的单片机的串口发送单字节函数
    3.TJCPrintf和printf用法一样

*/


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "tjc_usart_hmi.h"
#include <stddef.h>
#include "usart.h"

#define STR_LENGTH 100

typedef struct
{
    uint16_t Head;
    uint16_t Tail;
    uint16_t Lenght;
    uint8_t  Ring_data[RINGBUFF_LEN];
}RingBuff_t;

RingBuff_t ringBuff;	//创建一个ringBuff的缓冲区
uint8_t RxBuff[1];



/********************************************************
函数名：  	TJCPrintf
作者：    	wwd
日期：    	2022.10.08
功能：    	向串口打印数据,使用前请将USART_SCREEN_write这个函数改为你的单片机的串口发送单字节函数
输入参数：		参考printf
返回值： 		打印到串口的数量
修改记录：
**********************************************************/
//void USART1_printf (char *fmt, ...){ 
//	char buffer[STR_LENGTH+1];  // 数据长度
//	u8 i = 0;	
//	va_list arg_ptr;
//	va_start(arg_ptr, fmt);  
//	vsnprintf(buffer, STR_LENGTH+1, fmt, arg_ptr);
//	while ((i < STR_LENGTH) && (i < strlen(buffer))){
//        USART_SendData(USART1, (u8) buffer[i++]);
//        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); 
//	}
//	va_end(arg_ptr);
//	
//}


//void TJCPrintf (const char *str, ...){ 
//	char buffer[STR_LENGTH+1];  // 数据长度
//	u8 tdata,cdata=0xff;
//	u8 i = 0;	
//	va_list arg_ptr;
//	va_start(arg_ptr, str);  
//	vsnprintf(buffer, STR_LENGTH+1, str, arg_ptr);
//	va_end(arg_ptr);
//	while ((i < STR_LENGTH) && (i < strlen(buffer)))
//	{
//   tdata=(u8) buffer[i++];
//		HAL_UART_Transmit(&UART1_Handler, &tdata, 1, 1000);
//        while (__HAL_UART_GET_FLAG(&UART1_Handler,  UART_FLAG_TXE) == RESET); 
//	}
//	HAL_UART_Transmit(&UART1_Handler,&cdata, 1, 1000);	//这个函数改为你的单片机的串口发送单字节函数
//	while (__HAL_UART_GET_FLAG(&UART1_Handler,  UART_FLAG_TXE) == RESET);	
//	HAL_UART_Transmit(&UART1_Handler,&cdata, 1, 1000);		//这个函数改为你的单片机的串口发送单字节函数
//	while (__HAL_UART_GET_FLAG(&UART1_Handler,  UART_FLAG_TXE) == RESET);	
//	HAL_UART_Transmit(&UART1_Handler,&cdata, 1, 1000);		//这个函数改为你的单片机的串口发送单字节函数
//	while (__HAL_UART_GET_FLAG(&UART1_Handler,  UART_FLAG_TXE) == RESET);	

//}






/********************************************************
函数名：  	USART1_IRQHandler
作者：
日期：    	2022.10.08
功能：    	串口接收中断,将接收到的数据写入环形缓冲区
输入参数：
返回值： 		void
修改记录：
**********************************************************/
//void USART1_IRQHandler(void)
//{ u8 tdata;

//	if (__HAL_USART_GET_FLAG(&UART1_Handler, USART_FLAG_RXNE) != RESET)
//	{ 
//		
//    HAL_UART_IRQHandler(&UART1_Handler);				
//		HAL_UART_Receive_IT(&UART1_Handler,&tdata,1);
//		RxBuff[0] = tdata;
//		writeRingBuff(RxBuff[0]);
//			
//		
//	}

//}
     

		
/********************************************************
函数名：  	initRingBuff
作者：
日期：    	2022.10.08
功能：    	初始化环形缓冲区
输入参数：
返回值： 		void
修改记录：
**********************************************************/
void initRingBuff(void)
{
  //初始化相关信息
  ringBuff.Head = 0;
  ringBuff.Tail = 0;
  ringBuff.Lenght = 0;
}



/********************************************************
函数名：  	writeRingBuff
作者：
日期：    	2022.10.08
功能：    	往环形缓冲区写入数据
输入参数：
返回值： 		void
修改记录：
**********************************************************/
void writeRingBuff(uint8_t data)
{
  if(ringBuff.Lenght >= RINGBUFF_LEN) //判断缓冲区是否已满
  {
    return ;
  }
  ringBuff.Ring_data[ringBuff.Tail]=data;
  ringBuff.Tail = (ringBuff.Tail+1)%RINGBUFF_LEN;//防止越界非法访问
  ringBuff.Lenght++;

}




/********************************************************
函数名：  	deleteRingBuff
作者：
日期：    	2022.10.08
功能：    	删除串口缓冲区中相应长度的数据
输入参数：		要删除的长度
返回值： 		void
修改记录：
**********************************************************/
void deleteRingBuff(uint16_t size)
{
	if(size >= ringBuff.Lenght)
	{
	    initRingBuff();
	    return;
	}
	for(int i = 0; i < size; i++)
	{

		if(ringBuff.Lenght == 0)//判断非空
		{
		initRingBuff();
		return;
		}
		ringBuff.Head = (ringBuff.Head+1)%RINGBUFF_LEN;//防止越界非法访问
		ringBuff.Lenght--;

	}

}



/********************************************************
函数名：  	read1BFromRingBuff
作者：
日期：    	2022.10.08
功能：    	从串口缓冲区读取1字节数据
输入参数：		position:读取的位置
返回值： 		所在位置的数据(1字节)
修改记录：
**********************************************************/
uint8_t read1BFromRingBuff(uint16_t position)
{
	uint16_t realPosition = (ringBuff.Head + position) % RINGBUFF_LEN;

	return ringBuff.Ring_data[realPosition];
}




/********************************************************
函数名：  	getRingBuffLenght
作者：
日期：    	2022.10.08
功能：    	获取串口缓冲区的数据数量
输入参数：
返回值： 		串口缓冲区的数据数量
修改记录：
**********************************************************/
uint16_t getRingBuffLenght()
{
	return ringBuff.Lenght;
}


/********************************************************
函数名：  	isRingBuffOverflow
作者：
日期：    	2022.10.08
功能：    	判断环形缓冲区是否已满
输入参数：
返回值： 		1:环形缓冲区已满 , 2:环形缓冲区未满
修改记录：
**********************************************************/
uint8_t isRingBuffOverflow()
{
	return ringBuff.Lenght == RINGBUFF_LEN;
}







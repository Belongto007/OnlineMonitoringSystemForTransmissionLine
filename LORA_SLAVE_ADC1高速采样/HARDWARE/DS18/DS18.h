#ifndef __DS18_H
#define __DS18_H

#include "sys.h"   
#include "gpio.h"
#define DS18B20_HIGH  1
#define DS18B20_LOW   0


/*---------------------------------------*/
#define DS18B20_CLK     RCC_AHB1Periph_GPIOB
#define DS18B20_PIN     GPIO_PIN_10               
#define DS18B20_PORT    GPIOB


#define DS18B20_DATA_OUT(a)	if (a)	\
                                  HAL_GPIO_WritePin(DS18B20_PORT,DS18B20_PIN, GPIO_PIN_SET);\
                                   else		\
                                  HAL_GPIO_WritePin(DS18B20_PORT,DS18B20_PIN, GPIO_PIN_RESET);

#define  DS18B20_DATA_IN()	  GPIO_ReadInputDataBit(DS18B20_PORT,DS18B20_PIN)


uint8_t DS18B20_Init(void);
float DS18B20_Get_Temp(void);
void DS18B20_ReadId ( uint8_t * ds18b20_id );					
uint8_t * DS18B20_Res(void);
void DS18B20_Stm(void);

extern uint8_t RAM[8];																	 
#endif





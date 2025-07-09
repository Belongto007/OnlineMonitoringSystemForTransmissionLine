#include "gpio.h"
	/*
		AIN2									AIN			PA0
	  AIN1									AIN			PA1
		AIN3									AIN			PA2
		icm_pwren							OUT			PA3
		gps_pwren							OUT     PA4
		gps_on/off(NC)				OUT     PA5	
		gps_1pps							OUT     PA6
		NC										OUT			PA7
		NC										OUT   	PA8
		NC										OUT			PA9
		NC										OUT			PA10
		NC						        OUT			PA11
		NC						        OUT			PA12
		NC										OUT			PA15
	
		lora_busy     				IN			PB0
	  lora_dio      				OUT			PB1
		lora_pwren    				OUT			PB2
	  NC										OUT			PB3
	  NC										OUT			PB4
		NC										OUT			PB5
		NC										OUT			PB6
		NC										OUT			PB7
		NC										OUT			PB8
		NC										OUT			PB9
		ds18b20(NC)						OUT			PB10
		NC										OUT			PB11
		lora_cs								PB12
		lora_sck							PB13
		lora_miso							PB14
		lora_mosi							PB15
		
		icm_scl(NC)	       		OUT			PC0
		icm_sda(NC)	       		OUT			PC1
		icm_int(NC)	       		OUT			PC2
		ai_pwren      				OUT			PC3
		USART3_TX(NC)	     		OUT			PC4 
		USART3_RX(NC)	     		OUT			PC5
		lora_rst      				OUT			PC6
		NC       							OUT			PC7
		NC       							OUT			PC8	
		NC       							OUT			PC9	
		NC       							OUT			PC10	
		NC       							OUT			PC11	
		NC       							OUT			PC12
		NC       							OUT			PC13	
	*/
	
	
void GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();

		//GPIOA
	/*
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 ;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	*/
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |  GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |	GPIO_PIN_10 |\
													GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15 ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			//GPIOB
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  GPIO_InitStruct.Pin = GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |  GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |	GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	  GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
			//GPIOC
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1 |GPIO_PIN_2 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |  GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9  | GPIO_PIN_12 | GPIO_PIN_13 ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
		
		
}






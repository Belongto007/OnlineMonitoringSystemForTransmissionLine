#ifndef __SPI2_H
#define __SPI2_H
#include "sys.h"

void SPI2_Init(void);
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI2_ReadWriteByte(u8 TxData);
u8 SPI2_WriteByte(u8 *TxData,u16 size);
uint8_t SpiInOut( uint8_t txBuffer);

#endif



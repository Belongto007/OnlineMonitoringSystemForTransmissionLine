#include "myiic.h"
#include "MLX90614.h"
#include "delay.h" 
u8 value_H,value_L,PEC;
float MLX90614_read_value()
{
	u16 MLX90614_value = 0;
  float MLX90614_t = 0;
	
	IIC_Start();	
  //Slave address 单个MLX90614时地址为0x00  地址+wr	
	IIC_Send_Byte(0x00); 
	IIC_Wait_Ack(); 	//有应答  
	//发送命令
	IIC_Send_Byte(0x07);
  IIC_Wait_Ack();

	IIC_Start();    //Sr	
	IIC_Send_Byte(0x01);
	IIC_Wait_Ack();
		
	//读Tobj1低八位并发送应答
	value_L = IIC_Read_Byte(0);
			
	//读Tobj1高八位并发送应答  
	value_H = IIC_Read_Byte(0);

		
	//读PEC出错数据包并发送应答 
	PEC = IIC_Read_Byte(0);
		
	IIC_Stop();	
	
	/*数据的处理*/	
	MLX90614_value = value_H;
  MLX90614_value	<<= 8;
	MLX90614_value |= value_L;
	MLX90614_t=MLX90614_value*0.02-272.15;
	
	return MLX90614_t;	
}


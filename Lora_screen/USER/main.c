#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "gpio.h"
#include "spi2.h"
#include "adc.h"
#include "radio.h"
#include "sx126x.h"
#include "sx126x-board.h"
#include "crc.h"
#include "tjc_usart_hmi.h"
#include <stdlib.h>
#include <stdio.h>

  

#define ARRAY_SIZE 100 // 字符串数组最大长度
#define BUF_SIZE 20     // 提取数值的缓冲区大小

  // 假设payload字符串数组已定义并初始化
  char payload[ARRAY_SIZE] = "Hello 123.45 world 678";


void Get_AI(void);
void Lora_Iint(void);


u16 AD_CH1;
u16 AD_CH2;
u16 AD_CH3;


/**************************************************************************************************************************************
Demo 程序流程  EnableMaster=true  为主机端，主机端发送一个"PING"数据后切换到接收，等待从机返回的应答"PONG"数据LED闪烁

               EnableMaster=false 为从机端，从机端接收到主机端发过来的"PING"数据后LED闪烁并发送一个"PONG"数据作为应答
***************************************************************************************************************************************/


#define USE_MODEM_LORA
#define REGION_CN779
#define FRAMELENGTH 7
#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz

#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                434000000 // Hz

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 // Hz

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 // Hz

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_US915_HYBRID )

#define RF_FREQUENCY                                915000000 // Hz

#else

    #error "Please define a frequency band in the compiler options."

#endif

#define TX_OUTPUT_POWER                             22        // 22 dBm

extern bool IrqFired;




//bool EnableMaster=false;//主从选择
bool EnableMaster=true;//主从选择

uint16_t  crc_value;
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              2         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             2         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    5e3      // Hz 
#define FSK_DATARATE                                2.4e3      // bps
#define FSK_BANDWIDTH                               20e3     // Hz >> DSB in sx126x
#define FSK_AFC_BANDWIDTH                           100e3     // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t TX_Buffer[BUFFER_SIZE];
uint8_t RX_Buffer[BUFFER_SIZE];


States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;
void OnTxDone( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnTxTimeout( void );
void OnRxTimeout( void );
void OnRxError( void );
void screenstate(u8 state);

uint8_t  	data[2]={5,10};
uint8_t  	test[2];
extern 		SPI_HandleTypeDef SPI2_Handler; 
uint8_t  	ss;
RadioStatus_t status;
u16 tpbuffer[200];
uint8_t tjcstate=0;
float vobuffer[100]; 
char sendbuffer[100];
int main(void)
{ 
		
    float temp;
		ss=0xaa;
    HAL_Init();
    SystemClock_Config();			//初始化系统时钟为80M
    delay_init(80); 					//初始化延时函数80M系统时钟
    GPIO_Init();							//初始化GPIO
		uart_init(9600,9600);					//初始化串口，波特率为9600
		HAL_UART_Receive_IT(&UART4_Handler, &rxData, 1);
	
		AI_PWENH;									//						
		MY_ADC_Init();						//初始化ADC1通道5 6 7
		SPI2_Init();
		SPI2CSH;	
		LORA_PWENH;
		delay_ms(100);
		Lora_Iint();
	 // 提取数字
		
//		TJCPrintf("\x00"); 
	       // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    
//		while(1)
//		{
//				Radio.WriteBuffer(0x06C0,data,2);
//				delay_ms(100);
//				Radio.ReadBuffer(0x06C0,test,2);
//				delay_ms(100);
//		}
	
				
				
#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, false );

#elif defined( USE_MODEM_FSK )
    
    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );
#else
    #error "Please define a frequency band in the compiler options."
#endif

    
    if(EnableMaster)
    {
          TX_Buffer[0] = 'P';
          TX_Buffer[1] = 'I';
          TX_Buffer[2] = 'N';
          TX_Buffer[3] = 'G'; 
		printf("PING");
          
          crc_value=RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
          TX_Buffer[4]=crc_value>>8;
          TX_Buffer[5]=crc_value;
          Radio.Send( TX_Buffer, 6);
    }
    else
    {
       Radio.Rx( RX_TIMEOUT_VALUE ); 
    }
		
		PacketStatus_t pktStatus;	
		for(int itp=0;itp<100;itp++)
		{
			tpbuffer[itp]=itp+1;
		}
		
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);	
    while(1)
    {
//		HAL_UART_Transmit(&UART1_Handler,"bb",2,1000);
//		HAL_UART_Transmit(&UART4_Handler,"cc",2,1000);
//		printf("aa");
			//Get_AI();
			//串口数据格式：
	//串口数据帧长度：7字节
	//帧头      led编号  LED状态    帧尾
	//0x65      1字节    1字节     0xffffff
	//例子1：上位机代码  printh 65 00 01 00 ff ff ff  含义：开启温度访问
	//例子2：上位机代码  printh 65 09 00 01 FF FF FF   含义：开启电压访问
	//例子3：上位机代码  printh 65 00 02 00 FF FF FF   含义：0号led打开
	//例子4：上位机代码  printh 55 04 00 ff ff ff  含义：4号led关闭
//	  while(usize >= FRAMELENGTH)
//	  {
//		  //校验帧头帧尾是否匹配
//		  if(u(0) != 0x65 || u(4) != 0xff || u(5) != 0xff || u(6) != 0xff)
//		  {
//			  //不匹配删除1字节
//			  udelete(1);
//		  }else
//		  {
//			  //匹配，跳出循环
//			  break;
//		  }

//	  }
//		if(usize==1) udelete(1);
//  // printf("%x,%x,%x,%x,%x,%x,%x,%d",u(0),u(1),u(2),u(3),u(4),u(5),u(6),usize);

//	  //进行解析
//	  if(usize >= FRAMELENGTH && u(0) == 0x65&& u(3) == 0x00 && u(4) == 0xff && u(5) == 0xff && u(6) == 0xff)
//	  {
//		 tjcstate=1;
//		  udelete(FRAMELENGTH);
//			  screenstate(tjcstate);
//	  }		
//		  if(usize >= FRAMELENGTH && u(0) == 0x65&& u(3) == 0x01 && u(4) == 0xff && u(5) == 0xff && u(6) == 0xff)
//	  {
//		  tjcstate=0;
//		  udelete(FRAMELENGTH);
//			  screenstate(tjcstate);
//	  }	

		

		delay_ms(100);
			Radio.IrqProcess( ); // 						Process Radio IRQ
		
	}
	
			

			//status=SX126xGetStatus();
			//SX126xGetPacketStatus(&pktStatus );
    
			//Radio.Send( TX_Buffer, 6);

    }

u8 tepe=0;
void screenstate(u8 state)
{
	switch(state)
	{
		case 0:
		{ 
			tepe=0;
			
			//主屏幕备用
		}
		break;
		case 1:			
		{
			if(tepe==0)
			{
				int i=0;
				while(tp_buffer0[i]&&i<60)
				{	
					for(int u=0;u<4;u++)
					{
						printf("add 1,0,%d\xff\xff\xff",tp_buffer0[i]);						
					}
					i++;
				}	
				printf("\r\n");
				i=0;
				while(tp_buffer1[i]&&i<60)
				{	
					for(int u=0;u<4;u++)
					{
						printf("add 1,1,%d\xff\xff\xff",tp_buffer1[i]);						
					}
					i++;
				}
				
				tepe=3;
			}
		}
		break;
		case 2:			
		{
			if(tepe==0)
			{
				int j=0;
				while(vl_buffer0[j]&&j<60)
				{	
					for(int u=0;u<4;u++)
					{
						printf("add 1,0,%d\xff\xff\xff",(int)vl_buffer0[j]);						
					}
					j++;
				}
				j=0;
				while(vl_buffer1[j]&&j<60)
				{	
					for(int u=0;u<4;u++)
					{
						printf("add 1,1,%d\xff\xff\xff",(int)vl_buffer1[j]);						
					}
					j++;
				}
				j=0;
				while(vl_buffer2[j]&&j<60)
				{	
					for(int u=0;u<4;u++)
					{
						printf("add 1,2,%d\xff\xff\xff",(int)vl_buffer2[j]);						
					}
					j++;
				}			
				tepe=3;
			}
		}
		break;
		
	}
}



void Get_AI(void)
{
		AD_CH1 = Get_Adc_Average(ADC_CHANNEL_5, 4); 	//获取通道3的转换值，20次取平均
    AD_CH2 = Get_Adc_Average(ADC_CHANNEL_6, 4); 	//获取通道3的转换值，20次取平均
		AD_CH3 = Get_Adc_Average(ADC_CHANNEL_7, 4); 	//获取通道3的转换值，20次取平均
}


void Lora_Iint(void)
{
	LORA_RSTH;
	delay_ms(10);
	LORA_RSTL;
	delay_ms(100);
	LORA_RSTH;
	delay_ms(10);
}

void OnTxDone( void )
{   
    Radio.Standby();
    Radio.Rx( RX_TIMEOUT_VALUE ); //进入接收

}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    BufferSize = size;
    memcpy( RX_Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    
    Radio.Standby();
    
    if(EnableMaster)
    {
      if(memcmp(RX_Buffer,PongMsg,4)==0)
      {
        //LedToggle();//LED闪烁
		  printf("PONG");
        
      }
        TX_Buffer[0] = 'P';
        TX_Buffer[1] = 'I';
        TX_Buffer[2] = 'N';
        TX_Buffer[3] = 'G'; 
        
        crc_value=RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
        TX_Buffer[4]=crc_value>>8;
        TX_Buffer[5]=crc_value;
        Radio.Send( TX_Buffer, 6);
    }
    else
    {
      if(memcmp(RX_Buffer,PingMsg,4)==0)
      {
        //LedToggle();//LED闪烁
        
        TX_Buffer[0] = 'P';
        TX_Buffer[1] = 'O';
        TX_Buffer[2] = 'N';
        TX_Buffer[3] = 'G'; 
        
        crc_value=RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
        TX_Buffer[4]=crc_value>>8;
        TX_Buffer[5]=crc_value;
        Radio.Send( TX_Buffer, 6);
      }
      else
      {
        Radio.Rx( RX_TIMEOUT_VALUE ); 
      }   
    }
}

void OnTxTimeout( void )
{
   
}

void OnRxTimeout( void )
{
    Radio.Standby();
    if(EnableMaster)
    {
        TX_Buffer[0] = 'P';
        TX_Buffer[1] = 'I';
        TX_Buffer[2] = 'N';
        TX_Buffer[3] = 'G'; 
        
        crc_value=RadioComputeCRC(TX_Buffer,4,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
        TX_Buffer[4]=crc_value>>8;
        TX_Buffer[5]=crc_value;
        Radio.Send( TX_Buffer, 6);
    }
    else
    {
      Radio.Rx( RX_TIMEOUT_VALUE ); 
    }
}

void OnRxError( void )
{

  
}

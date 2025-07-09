#include "stm32l4xx.h"
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
#include "dma.h"
#include "DS18.h"
#include "GPS.h"
#include "MLX90614.h"
#include "tim.h"
void Get_AI(void);
void Get_Temprature(void);
void Get_T0(void);
void Get_T1(void);
void Lora_Iint(void);


u16 AD_CH1;
u16 AD_CH2;
u16 AD_CH3;

#define BUFFER_SIZE2 1000 // 20ms数据量：400μs * 50次 * 2通道
__attribute__((aligned(4))) uint16_t adc_buffer[BUFFER_SIZE2]; // DMA目标缓冲区


/**************************************************************************************************************************************
Demo 程序流程  EnableMaster=true  为主机端，主机端发送一个"PING"数据后切换到接收，等待从机返回的应答"PONG"数据LED闪烁

               EnableMaster=false 为从机端，从机端接收到主机端发过来的"PING"数据后LED闪烁并发送一个"PONG"数据作为应答
***************************************************************************************************************************************/


#define USE_MODEM_LORA
#define REGION_CN779

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
bool EnableMaster=false;//主从选择

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
#define LORA_SPREADING_FACTOR                       9    //9     // [SF7..SF12]
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

#define RX_TIMEOUT_VALUE                            3000
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

uint8_t  	data[2]={5,10};
uint8_t  	test[2];
extern 		SPI_HandleTypeDef SPI2_Handler; 
uint8_t  	ss,DS18B20Id[8];
RadioStatus_t status;
float fad1v,fad2v,fad3v,temperature,T0;
uint8_t command4[] = {0x5A, 0x5A, 0x03, 0x00, 0x00, 0xB7};
char sad1buf[12];
char sad2buf[12];
char sad3buf[12];
char tempbuf[12];
char sadbuf[128];
uint8_t txlen;
int main(void)
{
    
    float temp;
		ss=0xaa;
    HAL_Init();
    SystemClock_Config();			//初始化系统时钟为80M
    delay_init(80); 					//初始化延时函数80M系统时钟
    GPIO_Init();							//初始化GPIO
	
	MX_DMA_Init();
	MX_TIM1_Init();
	
		uart_init(115200);					//初始化串口，波特率为9600
		

		AI_PWENH;									//						
		MY_ADC_Init();						//初始化ADC1通道5 6 7
	

		SPI2_Init();
		SPI2CSH;	
	
	  DS18B20_Init();
	  DS18B20_ReadId ( DS18B20Id  );
		DS18B20_Stm();
	  DS18B20_Res();
//		

		LORA_PWENH;
		delay_ms(100);
		Lora_Iint();
			mlx90640_init();
				delay_ms(100);
			
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
    while(1)
    {
		
			Get_AI();
			delay_ms(1);
		 //Get_Temprature();
		  Get_T0();
			delay_ms(5);
//			Get_T1();
		Get_Temprature();
			delay_ms(1);
			Radio.IrqProcess(); // 						Process Radio IRQ
			//status=SX126xGetStatus();
			//SX126xGetPacketStatus(&pktStatus );
      //printf("sAS\n");
			
			txlen=sizeof(sadbuf);
	/*
			TX_Buffer[0]=(unsigned char )(AD_CH1>>8);
			TX_Buffer[1]=(unsigned char )AD_CH1;
			TX_Buffer[2]=(unsigned char )(AD_CH2>>8);
			TX_Buffer[3]=(unsigned char )AD_CH2;
			TX_Buffer[4]=(unsigned char )(AD_CH3>>8);
			TX_Buffer[5]=(unsigned char )AD_CH3;
			*/
			crc_value=RadioComputeCRC(sadbuf,txlen,CRC_TYPE_IBM);//计算得出要发送数据包CRC值
      sadbuf[txlen+1]=crc_value>>8;
      sadbuf[txlen+2]=crc_value;
      Radio.Send(sadbuf,txlen+2);
			delay_ms(1500);
    }
}

float T1 ;

void extract_float(uint8_t* buffer)
	{
    uint16_t value = (buffer[4] << 8) | buffer[5];
    T1 = (float)value / 100.0f;
}


 uint8_t response[8];
void Get_T1()
{
	 memset(response,0,sizeof(tempbuf));
	send_command(command4, sizeof(command4));
      
        HAL_UART_Receive(&UART3_Handler, response, 8, 1000);
			delay_ms(10);
        if (response[0] == 0x5A&&response[1] == 0x5A) 
					{
            extract_float(response);
						memset(tempbuf,0,sizeof(tempbuf));
			    	sprintf(tempbuf, "%.0f", T1);	
			    	strcat(sadbuf,tempbuf);//温度T1
				    strcat(sadbuf,"\r\n");
        } 
			else
			{
				 HAL_UART_AbortReceive(&UART3_Handler);
			}  
}


float convert_value(uint16_t raw) {
  return (raw / 4095.0f * 3.343f - 1.675f) * 0.5025f * 10.1f + 0.03f;
}
void Process_ADC_Data(void) {
//	for(int i=0; i<10; i++) {
//		printf("%.3f\t",convert_value(adc_buffer[i]));
//	}
//	printf("\r\n");
	
	for(int i=1; i<BUFFER_SIZE2; i+=2) {
    float ch6 = convert_value(adc_buffer[i]);
    printf("%.3f\t",ch6);
    
  }
	printf("\r\n");

	
	
}
void Reset_ADC_State(void) {
  // 只需要停止ADC和DMA，不需要完全重置
    HAL_ADC_Stop(&ADC1_Handler);  // 先停止ADC
    HAL_ADC_Stop_DMA(&ADC1_Handler);  // 再停止DMA
}

void Start_Sampling(void) {
    // 重置标志位
    g_adc_ready = 0;
    Reset_ADC_State();
    // 启动定时器触发
    
	    
//	__HAL_DMA_ENABLE_IT(&ADC1_Handler, DMA_IT_TC);
    
    // 启动ADC带DMA传输
	HAL_StatusTypeDef  status = HAL_ADC_Start_DMA(&ADC1_Handler, 
                     (uint32_t*)adc_buffer, 
                     BUFFER_SIZE2);
	 if(status != HAL_OK) {
    printf("ADC DMA Start Failed: ");
    
    // 详细错误报告
    if(status == HAL_ERROR) {
      printf("HAL_ERROR (ADC ErrorCode: 0x%08lX)\r\n", ADC1_Handler.ErrorCode);
      
      // 解析具体错误
      if(ADC1_Handler.ErrorCode & HAL_ADC_ERROR_INTERNAL) 
        printf(" - Internal error (clock/calibration issue)\r\n");
      if(ADC1_Handler.ErrorCode & HAL_ADC_ERROR_OVR)
        printf(" - Overrun error\r\n");
      if(ADC1_Handler.ErrorCode & HAL_ADC_ERROR_DMA)
        printf(" - DMA transfer error\r\n");
      if(ADC1_Handler.ErrorCode & HAL_ADC_ERROR_JQOVF)
        printf(" - Configuration error\r\n");
    }
    else if(status == HAL_BUSY) {
      printf("HAL_BUSY (ADC state: %d)\r\n", ADC1_Handler.State);
    }
    else {
      printf("Status: %d\r\n", status);
    }
  }
	 else
		 HAL_TIM_Base_Start(&htim1);

}

void Get_AI(void)
{
//	printf("AI\r\n");
	Start_Sampling();
	uint32_t start = HAL_GetTick();
	while(!g_adc_ready && (HAL_GetTick() - start < 50)); // 50ms超时
  
  if(g_adc_ready) {
    Process_ADC_Data();
    g_adc_ready = 0; // 重置标志
  }

	
	 
//		AD_CH1 = Get_Adc_Average(ADC_CHANNEL_5, 10); 	//获取通道3的转换值，20次取平均
//    AD_CH2 = Get_Adc_Average(ADC_CHANNEL_6, 10); 	//获取通道3的转换值，20次取平均
	
//	HAL_ADC_Start(&ADC1_Handler);                               //开启ADC
//    HAL_ADC_PollForConversion(&ADC1_Handler, 10);               //轮询转换
//	AD_CH2 = (u16)HAL_ADC_GetValue(&ADC1_Handler);

//	AD_CH3 = Get_Adc_Average(ADC_CHANNEL_7, 10); 	//获取通道3的转换值，20次取平均
//		fad1v = (float)((AD_CH1)/4095.0*3.343-1.675)*0.5025*10.1+0.25;
//	  fad2v = (float)((AD_CH2)/4095.0*3.343-1.675)*0.5025*10.1+0.25;
//		fad3v = (float)(AD_CH3)/4095.0*3.343*10.1+0.05;
//	printf("ADC = %.3f\r\n",fad2v);
//	  fad3v = fad3v*1.1;
//	  memset(sad1buf,0,sizeof(sad1buf));
//		memset(sad2buf,0,sizeof(sad1buf));
//		memset(sad3buf,0,sizeof(sad1buf));
//		memset(sadbuf,0,sizeof(sadbuf));
//	  
//	  sprintf(sad1buf, "%.3f", fad1v);
//		sprintf(sad2buf, "%.3f", fad2v);
//		sprintf(sad3buf, "%.3f", fad3v);
//		strcat(sadbuf,"1:");
//		strcat(sadbuf,sad1buf);//V1
//		strcat(sadbuf,",");
//		strcat(sadbuf,sad2buf);//V2
//		strcat(sadbuf,",");
//		strcat(sadbuf,sad3buf);//V3
//		strcat(sadbuf,",");
	
	
}
void Get_Temprature(void)
{
	temperature=DS18B20_Get_Temp();

	memset(tempbuf,0,sizeof(tempbuf));
	sprintf(tempbuf, "%.0f", temperature);	
//	strcat(sadbuf,"t1=");
	strcat(sadbuf,tempbuf);
	strcat(sadbuf,"\r\n");
}

void Get_T0(void)
{
	T0=MLX90614_read_value();

	
	memset(tempbuf,0,sizeof(tempbuf));
	sprintf(tempbuf, "%.0f", T0);	
	strcat(sadbuf,tempbuf);//外界温度T0
	strcat(sadbuf,",");
	//strcat(sadbuf,"\r\n");
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

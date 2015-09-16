#ifndef __I2S_H
#define __I2S_H
#include "sys.h"    									
//////////								  
//********************************************************************************
//V1.1 20140606
//����I2S2ext_Init��I2S2ext_RX_DMA_Init��I2S_Rec_Start��I2S_Rec_Stop�Ⱥ���
////////////////////////////////////////////////////////////////////////////////// 	
 

extern void (*i2s_tx_callback)(void);		//IIS TX�ص�����ָ�� 
extern void (*i2s_rx_callback)(void);		//IIS RX�ص�����ָ�� 

void I2S2_Init(u16 I2S_Standard,u16 I2S_Mode,u16 I2S_Clock_Polarity,u16 I2S_DataFormat);
void I2S2ext_Init(u16 I2S_Standard,u16 I2S_Mode,u16 I2S_Clock_Polarity,u16 I2S_DataFormat);
u8 I2S2_SampleRate_Set(u32 samplerate);
void I2S2_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num);
void I2S2ext_RX_DMA_Init(u8* buf0,u8 *buf1,u16 num);
void I2S_Play_Start(void);
void I2S_Rec_Start(void);
void I2S_Play_Stop(void);
void I2S_Rec_Stop(void);
 

#endif






















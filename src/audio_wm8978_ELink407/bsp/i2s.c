#include "i2s.h"  
#include "usart.h"
////////////////////////////////////////////////// 	
 

//I2S2��ʼ��

//����I2S_Standard:  @ref SPI_I2S_Standard  I2S��׼,
//I2S_Standard_Phillips,�����ֱ�׼;
//I2S_Standard_MSB,MSB�����׼(�Ҷ���);
//I2S_Standard_LSB,LSB�����׼(�����);
//I2S_Standard_PCMShort,I2S_Standard_PCMLong:PCM��׼
//����I2S_Mode:  @ref SPI_I2S_Mode  I2S_Mode_SlaveTx:�ӻ�����;I2S_Mode_SlaveRx:�ӻ�����;I2S_Mode_MasterTx:��������;I2S_Mode_MasterRx:��������;
//����I2S_Clock_Polarity   @ref SPI_I2S_Clock_Polarity:  I2S_CPOL_Low,ʱ�ӵ͵�ƽ��Ч;I2S_CPOL_High,ʱ�Ӹߵ�ƽ��Ч
//����I2S_DataFormat�� @ref SPI_I2S_Data_Format :���ݳ���,I2S_DataFormat_16b,16λ��׼;I2S_DataFormat_16bextended,16λ��չ(frame=32bit);I2S_DataFormat_24b,24λ;I2S_DataFormat_32b,32λ.
void I2S2_Init(u16 I2S_Standard,u16 I2S_Mode,u16 I2S_Clock_Polarity,u16 I2S_DataFormat)
{ 
  I2S_InitTypeDef I2S_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//ʹ��SPI2ʱ��
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE); //��λSPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);//������λ
  
	I2S_InitStructure.I2S_Mode=I2S_Mode;//IISģʽ
	I2S_InitStructure.I2S_Standard=I2S_Standard;//IIS��׼
	I2S_InitStructure.I2S_DataFormat=I2S_DataFormat;//IIS���ݳ���
	I2S_InitStructure.I2S_MCLKOutput=I2S_MCLKOutput_Disable;//��ʱ�������ֹ
	I2S_InitStructure.I2S_AudioFreq=I2S_AudioFreq_Default;//IISƵ������
	I2S_InitStructure.I2S_CPOL=I2S_Clock_Polarity;//����״̬ʱ�ӵ�ƽ
	I2S_Init(SPI2,&I2S_InitStructure);//��ʼ��IIS

 
	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);//SPI2 TX DMA����ʹ��.
  I2S_Cmd(SPI2,ENABLE);//SPI2 I2S ENʹ��.	
} 
//�����ʼ��㹫ʽ:Fs=I2SxCLK/[256*(2*I2SDIV+ODD)]
//I2SxCLK=(HSE/pllm)*PLLI2SN/PLLI2SR
//һ��HSE=8Mhz 
//pllm:��Sys_Clock_Set���õ�ʱ��ȷ����һ����8
//PLLI2SN:һ����192~432 
//PLLI2SR:2~7
//I2SDIV:2~255
//ODD:0/1
//I2S��Ƶϵ����@pllm=8,HSE=8Mhz,��vco����Ƶ��Ϊ1Mhz
//���ʽ:������/10,PLLI2SN,PLLI2SR,I2SDIV,ODD
const u16 I2S_PSC_TBL[][5]=
{
	{800 ,256,5,12,1},		//8Khz������
	{1102,429,4,19,0},		//11.025Khz������ 
	{1600,213,2,13,0},		//16Khz������
	{2205,429,4, 9,1},		//22.05Khz������
	{3200,213,2, 6,1},		//32Khz������
	{4410,271,2, 6,0},		//44.1Khz������
	{4800,258,3, 3,1},		//48Khz������
	{8820,316,2, 3,1},		//88.2Khz������
	{9600,344,2, 3,1},  	//96Khz������
	{17640,361,2,2,0},  	//176.4Khz������ 
	{19200,393,2,2,0},  	//192Khz������
};  
//����IIS�Ĳ�����(@MCKEN)
//samplerate:������,��λ:Hz
//����ֵ:0,���óɹ�;1,�޷�����.
u8 I2S2_SampleRate_Set(u32 samplerate)
{ 
	u8 i=0; 
	u32 tempreg=0;
	samplerate/=10;//��С10��   
	
	for(i=0;i<(sizeof(I2S_PSC_TBL)/10);i++)//�����Ĳ������Ƿ����֧��
	{
		if(samplerate==I2S_PSC_TBL[i][0])break;
	}
 
	RCC_PLLI2SCmd(DISABLE);//�ȹر�PLLI2S
	if(i==(sizeof(I2S_PSC_TBL)/10))return 1;//�ѱ���Ҳ�Ҳ���
	RCC_PLLI2SConfig((u32)I2S_PSC_TBL[i][1],(u32)I2S_PSC_TBL[i][2]);//����I2SxCLK��Ƶ��(x=2)  ����PLLI2SN PLLI2SR
 
	RCC->CR|=1<<26;					//����I2Sʱ��
	while((RCC->CR&1<<27)==0);		//�ȴ�I2Sʱ�ӿ����ɹ�. 
	tempreg=I2S_PSC_TBL[i][3]<<0;	//����I2SDIV
	tempreg|=I2S_PSC_TBL[i][3]<<8;	//����ODDλ
	tempreg|=1<<9;					//ʹ��MCKOEλ,���MCK
	SPI2->I2SPR=tempreg;			//����I2SPR�Ĵ��� 
	return 0;
}  
//I2S2 TX DMA����
//����Ϊ˫����ģʽ,������DMA��������ж�
//buf0:M0AR��ַ.
//buf1:M1AR��ַ.
//num:ÿ�δ���������
void I2S2_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num)
{  
	NVIC_InitTypeDef   NVIC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	
 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1ʱ��ʹ�� 
	
	DMA_DeInit(DMA1_Stream4);
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}//�ȴ�DMA1_Stream1������ 
		
  /* ���� DMA Stream */

  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //ͨ��0 SPI2_TXͨ�� 
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;//�����ַΪ:(u32)&SPI2->DR
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)buf0;//DMA �洢��0��ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//�洢��������ģʽ
  DMA_InitStructure.DMA_BufferSize = num;//���ݴ����� 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�洢������ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//�������ݳ���:16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//�洢�����ݳ��ȣ�16λ 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// ʹ��ѭ��ģʽ 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//�����ȼ�
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //��ʹ��FIFOģʽ        
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//����ͻ�����δ���
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//�洢��ͻ�����δ���
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);//��ʼ��DMA Stream
	
	DMA_DoubleBufferModeConfig(DMA1_Stream4,(u32)buf1,DMA_Memory_0);//˫����ģʽ����
 
  DMA_DoubleBufferModeCmd(DMA1_Stream4,ENABLE);//˫����ģʽ����
 
  DMA_ITConfig(DMA1_Stream4,DMA_IT_TC,ENABLE);//������������ж�
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//�����ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
  
}  
//I2S DMA�ص�����ָ��
void (*i2s_tx_callback)(void);	//TX�ص����� 
//DMA1_Stream4�жϷ�����
void DMA1_Stream4_IRQHandler(void)
{      
	if(DMA_GetITStatus(DMA1_Stream4,DMA_IT_TCIF4)==SET)////DMA1_Stream4,������ɱ�־
	{ 
		DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_TCIF4);
      	i2s_tx_callback();	//ִ�лص�����,��ȡ���ݵȲ����������洦��  
	}   											 
}  
//I2S��ʼ����
void I2S_Play_Start(void)
{   	  
	DMA_Cmd(DMA1_Stream4,ENABLE);//����DMA TX����,��ʼ���� 		
}
//�ر�I2S����
void I2S_Play_Stop(void)
{   
		DMA_Cmd(DMA1_Stream4,DISABLE);//�ر�DMA,��������	 
 	 
} 








#include "recorder.h" 
#include "audioplay.h"
#include "ff.h"
#include "malloc.h"

#include "usart.h"
#include "wm8978.h"
#include "i2s.h"

#include "delay.h"
#include "key.h"
#include "exfuns.h"  

#include "string.h"  
////////////////////////////////////////////////////////////// 	
  
u8 *i2srecbuf1;
u8 *i2srecbuf2; 

FIL* f_rec=0;		//¼���ļ�	
u32 wavsize;		//wav���ݴ�С(�ֽ���,�������ļ�ͷ!!)
u8 rec_sta=0;		//¼��״̬
					//[7]:0,û�п���¼��;1,�Ѿ�����¼��;
					//[6:1]:����
					//[0]:0,����¼��;1,��ͣ¼��;
					
//¼�� I2S_DMA�����жϷ�����.���ж�����д������
void rec_i2s_dma_rx_callback(void) 
{    
	u16 bw;
	u8 res;
	if(rec_sta==0X80)//¼��ģʽ
	{  
		if(DMA1_Stream3->CR&(1<<19))
		{
			res=f_write(f_rec,i2srecbuf1,I2S_RX_DMA_BUF_SIZE,(UINT*)&bw);//д���ļ�
			if(res)
			{
				printf("write error:%d\r\n",res);
			}
			 
		}else 
		{
			res=f_write(f_rec,i2srecbuf2,I2S_RX_DMA_BUF_SIZE,(UINT*)&bw);//д���ļ�
			if(res)
			{
				printf("write error:%d\r\n",res);
			}
		}
		wavsize+=I2S_RX_DMA_BUF_SIZE;
	} 
}  
const u16 i2splaybuf[2]={0X0000,0X0000};//2��16λ����,����¼��ʱI2S Master����.ѭ������0.
//����PCM ¼��ģʽ 		  
void recoder_enter_rec_mode(void)
{
	WM8978_ADDA_Cfg(0,1);		//����ADC
	WM8978_Input_Cfg(1,1,0);	//��������ͨ��(MIC&LINE IN)
	WM8978_Output_Cfg(0,1);		//����BYPASS��� 
	WM8978_MIC_Gain(46);		//MIC�������� 
	
	WM8978_I2S_Cfg(2,0);		//�����ֱ�׼,16λ���ݳ���
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//�����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ֡���� 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//�����ֱ�׼,�ӻ�����,ʱ�ӵ͵�ƽ��Ч,16λ֡����	
	I2S2_SampleRate_Set(16000);	//���ò����� 
 	I2S2_TX_DMA_Init((u8*)&i2splaybuf[0],(u8*)&i2splaybuf[1],1); 		//����TX DMA 
	DMA1_Stream4->CR&=~(1<<4);	//�رմ�������ж�(���ﲻ���ж�������) 
	I2S2ext_RX_DMA_Init(i2srecbuf1,i2srecbuf2,I2S_RX_DMA_BUF_SIZE/2); 	//����RX DMA
  	i2s_rx_callback=rec_i2s_dma_rx_callback;//�ص�����ָwav_i2s_dma_callback
 	I2S_Play_Start();	//��ʼI2S���ݷ���(����)
	I2S_Rec_Start(); 	//��ʼI2S���ݽ���(�ӻ�)
	recoder_remindmsg_show(0);
}  
//����PCM ����ģʽ 		  
void recoder_enter_play_mode(void)
{
	WM8978_ADDA_Cfg(1,0);		//����DAC 
	WM8978_Input_Cfg(0,0,0);	//�ر�����ͨ��(MIC&LINE IN)
	WM8978_Output_Cfg(1,0);		//����DAC��� 
	WM8978_MIC_Gain(0);			//MIC��������Ϊ0 
	I2S_Play_Stop();			//ֹͣʱ�ӷ���
	I2S_Rec_Stop(); 			//ֹͣ¼��
	recoder_remindmsg_show(1);
}
//��ʼ��WAVͷ.
void recoder_wav_init(__WaveHeader* wavhead) //��ʼ��WAVͷ			   
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=0;			//��δȷ��,�����Ҫ����
	wavhead->riff.Format=0X45564157; 	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66; 	//"fmt "
	wavhead->fmt.ChunkSize=16; 			//��СΪ16���ֽ�
	wavhead->fmt.AudioFormat=0X01; 		//0X01,��ʾPCM;0X01,��ʾIMA ADPCM
 	wavhead->fmt.NumOfChannels=2;		//˫����
 	wavhead->fmt.SampleRate=16000;		//16Khz������ ��������
 	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*4;//�ֽ�����=������*ͨ����*(ADCλ��/8)
 	wavhead->fmt.BlockAlign=4;			//���С=ͨ����*(ADCλ��/8)
 	wavhead->fmt.BitsPerSample=16;		//16λPCM
   	wavhead->data.ChunkID=0X61746164;	//"data"
 	wavhead->data.ChunkSize=0;			//���ݴ�С,����Ҫ����  
} 						    
//��ʾ¼��ʱ�������
//tsec:������.
void recoder_msg_show(u32 tsec,u32 kbps)
{   
	//��ʾ¼��ʱ��	
    /*	
	LCD_ShowString(30,210,200,16,16,"TIME:");	  	  
	LCD_ShowxNum(30+40,210,tsec/60,2,16,0X80);	//����
	LCD_ShowChar(30+56,210,':',16,0);
	LCD_ShowxNum(30+64,210,tsec%60,2,16,0X80);	//����	
	//��ʾ����		 
	LCD_ShowString(140,210,200,16,16,"KPBS:");	  	  
	LCD_ShowxNum(140+40,210,kbps/1000,4,16,0X80);	//������ʾ 	
	*/
}  	
//��ʾ��Ϣ
//mode:0,¼��ģʽ;1,����ģʽ
void recoder_remindmsg_show(u8 mode)
{
	//LCD_Fill(30,120,lcddev.width,180,WHITE);//���ԭ������ʾ
	//POINT_COLOR=RED;
	if(mode==0)	//¼��ģʽ
	{
//		Show_Str(30,120,200,16,"KEY0:REC/PAUSE",16,0); 
//		Show_Str(30,140,200,16,"KEY2:STOP&SAVE",16,0); 
//		Show_Str(30,160,200,16,"WK_UP:PLAY",16,0); 
		;
	}else		//����ģʽ
	{
		;
//		Show_Str(30,120,200,16,"KEY0:STOP Play",16,0);  
//		Show_Str(30,140,200,16,"WK_UP:PLAY/PAUSE",16,0); 
	}
}
//ͨ��ʱ���ȡ�ļ���
//������SD������,��֧��FLASH DISK����
//��ϳ�:����"0:RECORDER/REC20120321210633.wav"���ļ���
void recoder_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECORDER/REC%05d.wav",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
		if(res==FR_NO_FILE)break;		//���ļ���������=����������Ҫ��.
		index++;
	}
} 
//WAV¼�� 
void wav_recorder(void)
{ 
	u8 res;
	u8 key;
	u8 rval=0;
	__WaveHeader *wavhead=0; 
 	DIR recdir;	 					//Ŀ¼  
 	u8 *pname=0;
	u8 timecnt=0;					//��ʱ��   
	u32 recsec=0;					//¼��ʱ�� 
  	while(f_opendir(&recdir,"0:/RECORDER"))//��¼���ļ���
 	{	 
		//Show_Str(30,230,240,16,"RECORDER�ļ��д���!",16,0);
		delay_ms(200);				  
		//LCD_Fill(30,230,240,246,WHITE);		//�����ʾ	     
		delay_ms(200);				  
		f_mkdir("0:/RECORDER");				//������Ŀ¼   
	}   
	i2srecbuf1=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�1����
	i2srecbuf2=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�2����  
  	f_rec=(FIL *)mymalloc(SRAMIN,sizeof(FIL));		//����FIL�ֽڵ��ڴ�����  
 	wavhead=(__WaveHeader*)mymalloc(SRAMIN,sizeof(__WaveHeader));//����__WaveHeader�ֽڵ��ڴ����� 
	pname=mymalloc(SRAMIN,30);						//����30���ֽ��ڴ�,����"0:RECORDER/REC00001.wav" 
	if(!i2srecbuf1||!i2srecbuf2||!f_rec||!wavhead||!pname)rval=1; 	if(rval==0)		
	{
		recoder_enter_rec_mode();	//����¼��ģʽ,��ʱ��������������ͷ�ɼ�������Ƶ   
		pname[0]=0;					//pnameû���κ��ļ��� 
 	   	while(rval==0)
		{
			key=KEY_Scan(0);
			switch(key)
			{		
				case KEY2_PRES:	//STOP&SAVE
					if(rec_sta&0X80)//��¼��
					{
						rec_sta=0;	//�ر�¼��
						wavhead->riff.ChunkSize=wavsize+36;		//�����ļ��Ĵ�С-8;
				   		wavhead->data.ChunkSize=wavsize;		//���ݴ�С
						f_lseek(f_rec,0);						//ƫ�Ƶ��ļ�ͷ.
				  		f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//д��ͷ����
						f_close(f_rec);
						wavsize=0;
					}
					rec_sta=0;
					recsec=0;
				 	//LED1=1;	 						//�ر�DS1
					//LCD_Fill(30,190,lcddev.width,lcddev.height,WHITE);//�����ʾ,���֮ǰ��ʾ��¼���ļ���		      
					break;	 
				case KEY0_PRES:	//REC/PAUSE
					if(rec_sta&0X01)//ԭ������ͣ,����¼��
					{
						rec_sta&=0XFE;//ȡ����ͣ
					}else if(rec_sta&0X80)//�Ѿ���¼����,��ͣ
					{
						rec_sta|=0X01;	//��ͣ
					}else				//��û��ʼ¼�� 
					{
						recsec=0;	 
						recoder_new_pathname(pname);			//�õ��µ�����
						//Show_Str(30,190,lcddev.width,16,"¼��:",16,0);		   
						//Show_Str(30+40,190,lcddev.width,16,pname+11,16,0);//��ʾ��ǰ¼���ļ�����
				 		recoder_wav_init(wavhead);				//��ʼ��wav����	
	 					res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE); 
						if(res)			//�ļ�����ʧ��
						{
							rec_sta=0;	//�����ļ�ʧ��,����¼��
							rval=0XFE;	//��ʾ�Ƿ����SD��
						}else 
						{
							res=f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//д��ͷ����
							recoder_msg_show(0,0);
 							rec_sta|=0X80;	//��ʼ¼��	 
						} 
 					}
					//if(rec_sta&0X01)LED1=0;	//��ʾ������ͣ
					//else LED1=1;
					break;  
				case WKUP_PRES:	//�������һ��¼��
					if(rec_sta!=0X80)//û����¼��
					{   	 		 				  
						if(pname[0])//�����������������,��pname��Ϊ��
						{				 
							//Show_Str(30,190,lcddev.width,16,"����:",16,0);		   
							//Show_Str(30+40,190,lcddev.width,16,pname+11,16,0);//��ʾ�����ŵ��ļ�����
							recoder_enter_play_mode();	//���벥��ģʽ
							audio_play_song(pname);		//����pname
							//LCD_Fill(30,190,lcddev.width,lcddev.height,WHITE);//�����ʾ,���֮ǰ��ʾ��¼���ļ���	  
							recoder_enter_rec_mode();	//���½���¼��ģʽ 
						}
					}
					break;
			} 
			delay_ms(5);
			timecnt++;
			//if((timecnt%20)==0)LED0=!LED0;//DS0��˸  
 			if(recsec!=(wavsize/wavhead->fmt.ByteRate))	//¼��ʱ����ʾ
			{	   
				//LED0=!LED0;//DS0��˸ 
				recsec=wavsize/wavhead->fmt.ByteRate;	//¼��ʱ��
				recoder_msg_show(recsec,wavhead->fmt.SampleRate*wavhead->fmt.NumOfChannels*wavhead->fmt.BitsPerSample);//��ʾ����
			}
		}		 
	}    
	myfree(SRAMIN,i2srecbuf1);	//�ͷ��ڴ�
	myfree(SRAMIN,i2srecbuf2);	//�ͷ��ڴ�  
	myfree(SRAMIN,f_rec);		//�ͷ��ڴ�
	myfree(SRAMIN,wavhead);		//�ͷ��ڴ�  
	myfree(SRAMIN,pname);		//�ͷ��ڴ�  
}




































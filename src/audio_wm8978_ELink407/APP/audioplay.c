#include "audioplay.h"
#include "ff.h"
#include "malloc.h"
#include "usart.h"
#include "wm8978.h"
#include "i2s.h"
//#include "led.h"
//#include "lcd.h"
#include "delay.h"
//#include "key.h"
#include "exfuns.h"  

#include "string.h"  	
 

//���ֲ��ſ�����
__audiodev audiodev;	  
 

//��ʼ��Ƶ����
void audio_start(void)
{
	audiodev.status=3<<0;//��ʼ����+����ͣ
	I2S_Play_Start();
} 
//�ر���Ƶ����
void audio_stop(void)
{
	audiodev.status=0;
	I2S_Play_Stop();
}  
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 audio_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}
//��ʾ��Ŀ����
//index:��ǰ����
//total:���ļ���
void audio_index_show(u16 index,u16 total)
{
	/*
	//��ʾ��ǰ��Ŀ������,������Ŀ��
	LCD_ShowxNum(60+0,230,index,3,16,0X80);		//����
	LCD_ShowChar(60+24,230,'/',16,0);
	LCD_ShowxNum(60+32,230,total,3,16,0X80); 	//����Ŀ				  	  
*/
}
 
//��ʾ����ʱ��,������ ��Ϣ  
//totsec;��Ƶ�ļ���ʱ�䳤��
//cursec:��ǰ����ʱ��
//bitrate:������(λ��)
void audio_msg_show(u32 totsec,u32 cursec,u32 bitrate)
{	
	static u16 playtime=0XFFFF;//����ʱ����	      
	if(playtime!=cursec)					//��Ҫ������ʾʱ��
	{
		playtime=cursec;
		/*
		//��ʾ����ʱ��			 
		LCD_ShowxNum(60,210,playtime/60,2,16,0X80);		//����
		LCD_ShowChar(60+16,210,':',16,0);
		LCD_ShowxNum(60+24,210,playtime%60,2,16,0X80);	//����		
 		LCD_ShowChar(60+40,210,'/',16,0); 	    	 
		//��ʾ��ʱ��    	   
 		LCD_ShowxNum(60+48,210,totsec/60,2,16,0X80);	//����
		LCD_ShowChar(60+64,210,':',16,0);
		LCD_ShowxNum(60+72,210,totsec%60,2,16,0X80);	//����	  		    
		//��ʾλ��			   
   		LCD_ShowxNum(60+110,210,bitrate/1000,4,16,0X80);//��ʾλ��	 
		LCD_ShowString(60+110+32,210,200,16,16,"Kbps");	 
	*/
	} 		 
}
//��������
void audio_play(void)
{
	
	  
 
	WM8978_ADDA_Cfg(1,0);	//����DAC
	WM8978_Input_Cfg(0,0,0);//�ر�����ͨ��
	WM8978_Output_Cfg(1,0);	//����DAC���   

	audio_play_song("0:/wav/1.wav"); 			 		//���������Ƶ�ļ�
}
//����ĳ����Ƶ�ļ�
u8 audio_play_song(u8* fname)
{
	u8 res;  
	res=f_typetell(fname); 
	switch(res)
	{
		case T_WAV:
			res=wav_play_song(fname);
			break;
		default://�����ļ�,�Զ���ת����һ��
			printf("can't play:%s\r\n",fname);
			//res=KEY0_PRES;
			break;
	}
	return res;
}




























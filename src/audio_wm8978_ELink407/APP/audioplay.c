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
 

//音乐播放控制器
__audiodev audiodev;	  
 

//开始音频播放
void audio_start(void)
{
	audiodev.status=3<<0;//开始播放+非暂停
	I2S_Play_Start();
} 
//关闭音频播放
void audio_stop(void)
{
	audiodev.status=0;
	I2S_Play_Stop();
}  
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 audio_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}
//显示曲目索引
//index:当前索引
//total:总文件数
void audio_index_show(u16 index,u16 total)
{
	/*
	//显示当前曲目的索引,及总曲目数
	LCD_ShowxNum(60+0,230,index,3,16,0X80);		//索引
	LCD_ShowChar(60+24,230,'/',16,0);
	LCD_ShowxNum(60+32,230,total,3,16,0X80); 	//总曲目				  	  
*/
}
 
//显示播放时间,比特率 信息  
//totsec;音频文件总时间长度
//cursec:当前播放时间
//bitrate:比特率(位速)
void audio_msg_show(u32 totsec,u32 cursec,u32 bitrate)
{	
	static u16 playtime=0XFFFF;//播放时间标记	      
	if(playtime!=cursec)					//需要更新显示时间
	{
		playtime=cursec;
		/*
		//显示播放时间			 
		LCD_ShowxNum(60,210,playtime/60,2,16,0X80);		//分钟
		LCD_ShowChar(60+16,210,':',16,0);
		LCD_ShowxNum(60+24,210,playtime%60,2,16,0X80);	//秒钟		
 		LCD_ShowChar(60+40,210,'/',16,0); 	    	 
		//显示总时间    	   
 		LCD_ShowxNum(60+48,210,totsec/60,2,16,0X80);	//分钟
		LCD_ShowChar(60+64,210,':',16,0);
		LCD_ShowxNum(60+72,210,totsec%60,2,16,0X80);	//秒钟	  		    
		//显示位率			   
   		LCD_ShowxNum(60+110,210,bitrate/1000,4,16,0X80);//显示位率	 
		LCD_ShowString(60+110+32,210,200,16,16,"Kbps");	 
	*/
	} 		 
}
//播放音乐
void audio_play(void)
{
	
	  
 
	WM8978_ADDA_Cfg(1,0);	//开启DAC
	WM8978_Input_Cfg(0,0,0);//关闭输入通道
	WM8978_Output_Cfg(1,0);	//开启DAC输出   

	audio_play_song("0:/wav/1.wav"); 			 		//播放这个音频文件
}
//播放某个音频文件
u8 audio_play_song(u8* fname)
{
	u8 res;  
	res=f_typetell(fname); 
	switch(res)
	{
		case T_WAV:
			res=wav_play_song(fname);
			break;
		default://其他文件,自动跳转到下一曲
			printf("can't play:%s\r\n",fname);
			//res=KEY0_PRES;
			break;
	}
	return res;
}




























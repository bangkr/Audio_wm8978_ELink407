#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"

#include "key.h"  

#include "malloc.h" 
  
#include "sdio_sdcard.h"
#include "ff.h"  
#include "exfuns.h"    

 
#include "wm8978.h"	 
#include "audioplay.h"	

 
int main(void)
{        
 
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	//LED_Init();					//初始化LED 
//	usmart_dev.init(84);		//初始化USMART
 
 	//KEY_Init();					//按键初始化  

	WM8978_Init();				//初始化WM8978
	WM8978_HPvol_Set(40,40);	//耳机音量设置
	WM8978_SPKvol_Set(60);		//喇叭音量设置
	

	exfuns_init();				//为fatfs相关变量申请内存  
  	f_mount(fs[0],"0:",1); 		//挂载SD卡  
	  
    delay_ms(1000);	

	while(1)
	{ 
		audio_play();
	} 
}








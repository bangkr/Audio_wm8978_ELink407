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
 
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	//LED_Init();					//��ʼ��LED 
//	usmart_dev.init(84);		//��ʼ��USMART
 
 	//KEY_Init();					//������ʼ��  

	WM8978_Init();				//��ʼ��WM8978
	WM8978_HPvol_Set(40,40);	//������������
	WM8978_SPKvol_Set(60);		//������������
	

	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
  	f_mount(fs[0],"0:",1); 		//����SD��  
	  
    delay_ms(1000);	

	while(1)
	{ 
		audio_play();
	} 
}







